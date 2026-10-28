// Title: Stopwatch
// Level: Advanced
// Shows: measuring with monotonic_clock and precise_clock, restart, and sleep_for.

#include <Serialize/Io.hpp>
#include <Threading/MonotonicClock.hpp>
#include <Threading/Thread.hpp>
#include <Threading/Time.hpp>

using namespace hsd::format_literals;
using namespace hsd::precise_clock_literals;

// A small stopwatch over the monotonic clock, which never jumps backwards.
class stopwatch
{
private:
    hsd::u64 _start = hsd::monotonic_clock::now_ns();

public:
    hsd::u64 elapsed_ns() const
    {
        return hsd::monotonic_clock::now_ns() - _start;
    }

    hsd::u64 restart()
    {
        hsd::u64 now = hsd::monotonic_clock::now_ns();
        hsd::u64 lap = now - _start;
        _start = now;
        return lap;
    }
};

int main()
{
    // Exact timings change from run to run, so each check prints true or false.
    stopwatch watch;
    hsd::this_thread::sleep_for(0.05f).unwrap();
    hsd::u64 first_lap = watch.restart();
    hsd::println("first lap at least 45 ms: {}"_fmt, first_lap >= 45'000'000 ? "true" : "false");

    // restart() begins a new lap, so the second one is short.
    hsd::u64 second_lap = watch.restart();
    hsd::println("second lap under the first: {}"_fmt, second_lap < first_lap ? "true" : "false");

    // Work takes some time, even a little. Compare against zero, not a value.
    hsd::u64 sink = 0;
    watch.restart();

    for (hsd::u64 i = 0; i < 200000; ++i)
    {
        sink += i % 7;
    }

    hsd::println("work took time: {}"_fmt, watch.elapsed_ns() > 0 ? "true" : "false");
    hsd::println("work result: {}"_fmt, sink);

    // precise_clock is the wall clock. restart() returns the time since the last restart.
    hsd::precise_clock wall;
    hsd::this_thread::sleep_for(0.03f).unwrap();
    hsd::precise_clock slept = wall.restart();
    hsd::println("wall clock saw the sleep: {}"_fmt, slept >= 25_ms ? "true" : "false");

    // elapsed_time() measures from the last restart without resetting it.
    hsd::this_thread::sleep_for(0.02f).unwrap();
    hsd::precise_clock since = wall.elapsed_time();
    hsd::println("elapsed_time at least 15 ms: {}"_fmt, since >= 15_ms ? "true" : "false");
    hsd::println("elapsed_time under one second: {}"_fmt, since < 1_s ? "true" : "false");
    return 0;
}
