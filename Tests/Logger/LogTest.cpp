#include <Serialize/Logging.hpp>
#include <Serialize/Tracing.hpp>
#include <Serialize/ToString.hpp>
#include <Threading/Atomic.hpp>
#include <Threading/Thread.hpp>
#include <cassert>
#include <format>

template <hsd::usize N>
auto test_trace([[maybe_unused]] const char* msg, [[maybe_unused]] hsd::stack_trace st = hsd::stack_trace::add())
    -> hsd::option_err<hsd::stack_trace_error>
{
    auto res = test_trace<N - 1>(msg);
    res.unwrap();

    return {};
}

template <>
auto test_trace<0>([[maybe_unused]] const char* msg, [[maybe_unused]] hsd::stack_trace st)
    -> hsd::option_err<hsd::stack_trace_error>
{
    return hsd::stack_trace_error{};
}

// A handle owns one stack entry, so copying or moving it would pop twice. The
// concept form keeps the check in the immediate context, so a deleted copy is
// detected rather than diagnosed.
template <typename T>
concept copyable_handle = requires (const T& h) { T{h}; };
template <typename T>
concept movable_handle = requires (T&& h) { T{hsd::move(h)}; };

static_assert(!copyable_handle<hsd::stack_trace>);
static_assert(!movable_handle<hsd::stack_trace>);

static void test_default_handle_owns_nothing()
{
    const hsd::i32 expected_line = __LINE__ + 1;
    auto entry = hsd::stack_trace::add();
    {
        // Destroying a handle that never pushed must not pop `entry`.
        hsd::stack_trace idle;
    }
    assert(entry.get().line() == expected_line);
}

static void test_stack_is_per_thread()
{
    const hsd::i32 main_line = __LINE__ + 1;
    auto main_entry = hsd::stack_trace::add();

    hsd::atomic_bool worker_pushed{false};
    hsd::atomic_bool main_checked{false};
    hsd::atomic_bool worker_saw_own_entry{false};

    hsd::thread worker{[&] {
        const hsd::i32 worker_line = __LINE__ + 1;
        auto worker_entry = hsd::stack_trace::add();
        worker_saw_own_entry.store(worker_entry.get().line() == worker_line);
        worker_pushed.store(true);

        while (main_checked.load() == false)
        {
            hsd::this_thread::yield();
        }
    }};

    while (worker_pushed.load() == false)
    {
        hsd::this_thread::yield();
    }

    // The worker holds an entry on its own stack; ours must still be on top.
    assert(main_entry.get().line() == main_line);
    main_checked.store(true);

    worker.join().unwrap();
    assert(worker_saw_own_entry.load());
}

// Makes the compiler keep `value` in memory, so the conversion is not optimized away.
// A volatile pointer works on every compiler, unlike inline asm on MSVC.
template <typename T>
static void keep_in_memory(T& value)
{
    T* volatile _escaped = &value;
    (void)_escaped;
}

static void test_timed_to_str()
{
    hsd::trace_scope _scope;
    using FpType = hsd::f128;

    for (hsd::usize it = 0; it < 100; ++it)
    {
        for (auto idx = hsd::limits<FpType>::denorm_min; idx != hsd::limits<FpType>::infinity; idx *= 10)
        {
            auto v = hsd::to_pseudo_string_hex(-idx);
            keep_in_memory(idx);
            keep_in_memory(v);
        }
    }
}

static void test_timed_to_snpr()
{
    hsd::trace_scope _scope;
    using FpType = hsd::f128;

    for (hsd::usize it = 0; it < 100; ++it)
    {
        for (auto idx = hsd::limits<FpType>::denorm_min; idx != hsd::limits<FpType>::infinity; idx *= 10)
        {
            auto buf = std::format("{:a}", -idx);
            keep_in_memory(buf);
            keep_in_memory(idx);
        }
    }
}

int main()
{
    auto positive = hsd::to_pseudo_string_hex(1.0L);
    auto negative = hsd::to_pseudo_string_hex(-1.0L);
    assert(positive.size() > 0);
    assert(negative.size() > 0);

    test_default_handle_owns_nothing();
    test_stack_is_per_thread();

    test_timed_to_str();
    test_timed_to_snpr();
    return 0;
}
