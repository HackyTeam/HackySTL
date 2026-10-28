// Enables HSD_TRACE_SCOPE for this translation unit. The macro is fixed when
// the header is included, so the define must come first.
#define HSD_ENABLE_TRACING
#include <Serialize/Tracing.hpp>
#include <Threading/Thread.hpp>
#include <cassert>

namespace
{
    /// @brief Counts events for the test hook.
    /// @details The sink calls hooks one at a time under its lock, so plain
    /// members are safe to update from the hook.
    struct recorder
    {
        hsd::usize total = 0;
        hsd::usize at_depth[2] = {};
        hsd::usize other_depths = 0;
        hsd::u32 last_depth = 0;
        hsd::u64 last_duration_ns = 0;
    };

    recorder rec;

    void record(const hsd::trace_event& ev)
    {
        ++rec.total;
        if (ev.depth < 2)
        {
            ++rec.at_depth[ev.depth];
        }
        else
        {
            ++rec.other_depths;
        }

        rec.last_depth = ev.depth;
        rec.last_duration_ns = ev.duration_ns;
    }

    void reset_recorder()
    {
        rec = recorder{};
        hsd::trace_sink::set_hook(&record);
    }

    void traced_worker()
    {
        for (int i = 0; i < 500; ++i)
        {
            hsd::trace_scope outer;
            hsd::trace_scope inner;
        }
    }
}

// A scope emits one event on destruction, so copying or moving it would
// report the same block twice. The concept form keeps the check in the
// immediate context, so a deleted copy is detected rather than diagnosed.
template <typename T>
concept copyable_scope = requires (const T& s) { T{s}; };
template <typename T>
concept movable_scope = requires (T&& s) { T{hsd::move(s)}; };

static_assert(!copyable_scope<hsd::trace_scope>);
static_assert(!movable_scope<hsd::trace_scope>);

static void test_nested_scopes_report_depth()
{
    reset_recorder();
    {
        hsd::trace_scope outer;
        {
            hsd::trace_scope inner;
        }
        // The inner scope ends first, one level deep.
        assert(rec.total == 1);
        assert(rec.last_depth == 1);
    }

    // Both scopes report, and the outer one is back at depth zero.
    assert(rec.total == 2);
    assert(rec.last_depth == 0);
    assert(rec.at_depth[0] == 1);
    assert(rec.at_depth[1] == 1);
}

static void test_duration_covers_sleep()
{
    reset_recorder();
    {
        hsd::trace_scope timed;
        hsd::this_thread::sleep_for(0.02f).unwrap();
    }

    assert(rec.total == 1);
    assert(rec.last_duration_ns >= 20'000'000);
}

static void test_macro_declares_scope()
{
    reset_recorder();
    {
        HSD_TRACE_SCOPE();
        HSD_TRACE_SCOPE();
    }

    assert(rec.total == 2);
    assert(rec.at_depth[0] == 1);
    assert(rec.at_depth[1] == 1);
}

static void test_output_waits_for_flush()
{
    hsd::trace_sink::set_hook(&hsd::print_text);
    hsd::trace_sink::flush();

    {
        hsd::trace_scope buffered;
    }

    // The event sits in the output buffer until an explicit flush.
    assert(hsd::io::cout().get_stream_data().length() > 0);
    hsd::trace_sink::flush();
    assert(hsd::io::cout().get_stream_data().length() == 0);
}

static void test_each_event_ends_its_line()
{
    hsd::trace_sink::set_hook(&hsd::print_text);
    hsd::trace_sink::flush();

    {
        hsd::trace_scope first;
    }
    {
        hsd::trace_scope second;
    }

    // Each event is one line, so two events leave exactly two newlines.
    const auto data = hsd::io::cout().get_stream_data();
    hsd::usize newlines = 0;
    for (hsd::usize index = 0; index < data.length(); ++index)
    {
        if (data[index] == '\n')
        {
            ++newlines;
        }
    }

    assert(newlines == 2);
    assert(data[data.length() - 1] == '\n');
    hsd::trace_sink::flush();
}

static void test_threads_keep_their_own_depth()
{
    reset_recorder();

    hsd::thread workers[4];
    for (auto& worker : workers)
    {
        worker = hsd::thread{traced_worker};
    }

    for (auto& worker : workers)
    {
        worker.join().unwrap();
    }

    // Every thread nests two scopes 500 times. A depth counter shared between
    // threads would push some events past depth one.
    assert(rec.total == hsd::usize{4 * 500 * 2});
    assert(rec.at_depth[0] == hsd::usize{4 * 500});
    assert(rec.at_depth[1] == hsd::usize{4 * 500});
    assert(rec.other_depths == 0);
}

int main()
{
    test_nested_scopes_report_depth();
    test_duration_covers_sleep();
    test_macro_declares_scope();
    test_output_waits_for_flush();
    test_each_event_ends_its_line();
    test_threads_keep_their_own_depth();

    return 0;
}
