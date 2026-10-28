// Title: Tracing across threads
// Level: Advanced
// Shows: per-thread nesting depth, serialized hooks, and tracing from worker threads.

#define HSD_ENABLE_TRACING
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/Tracing.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

namespace
{
    constexpr hsd::usize worker_count = 4;
    constexpr hsd::usize rounds = 250;

    // Delivery is serialized by the sink, so the hook needs no lock of its own.
    // It compares function names by their first letter, which is enough here.
    struct counters
    {
        hsd::usize total = 0;
        hsd::usize outer_events = 0;
        hsd::usize inner_events = 0;
        hsd::usize wrong_depth = 0;
    };

    counters seen;

    void count_event(const hsd::trace_event& event)
    {
        ++seen.total;

        // 'o' for outer_step, 'i' for inner_step. If threads shared one depth
        // counter, these depths would drift.
        if (event.function[0] == 'o')
        {
            ++seen.outer_events;
            seen.wrong_depth += event.depth != 0 ? 1 : 0;
        }
        else
        {
            ++seen.inner_events;
            seen.wrong_depth += event.depth != 1 ? 1 : 0;
        }
    }

    void inner_step(hsd::u64& sum, hsd::usize value)
    {
        HSD_TRACE_SCOPE();
        sum += value;
    }

    void outer_step(hsd::u64& sum, hsd::usize value)
    {
        HSD_TRACE_SCOPE();
        inner_step(sum, value);
    }
}

int main()
{
    hsd::trace_sink::set_hook(&count_event);

    // Each worker owns one slot, so no worker touches another's result.
    hsd::vector<hsd::u64> sums;
    sums.resize(worker_count);

    hsd::vector<hsd::thread> workers;
    for (hsd::usize id = 0; id < worker_count; ++id)
    {
        workers.emplace_back([&sums, id]
        {
            for (hsd::usize i = 1; i <= rounds; ++i)
            {
                outer_step(sums[id], i * (id + 1));
            }
        });
    }

    // Join in order, then print from the main thread only.
    for (hsd::usize id = 0; id < worker_count; ++id)
    {
        workers[id].join().unwrap();
    }

    for (hsd::usize id = 0; id < worker_count; ++id)
    {
        hsd::println("worker {} sum = {}"_fmt, id, sums[id]);
    }

    hsd::println("total events: {}"_fmt, seen.total);
    hsd::println("outer events: {}, inner events: {}"_fmt, seen.outer_events, seen.inner_events);
    hsd::println("events at the wrong depth: {}"_fmt, seen.wrong_depth);

    return 0;
}
