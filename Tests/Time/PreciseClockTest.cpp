#include <Threading/Thread.hpp>
#include <Threading/Time.hpp>
#include <cassert>
#include <time.h>

int main()
{
    // precise_clock is wall-clock time: it matches the C library's time() and
    // is the clock that absolute condition-variable deadlines are measured on.
    const hsd::precise_clock now = hsd::precise_clock::now();
    const long long unix_now = static_cast<long long>(time(nullptr));
    const long long drift = static_cast<long long>(now.to_native().tv_sec) - unix_now;
    assert(drift >= -2 && drift <= 2);

    // A sleeping thread consumes no CPU time, so a CPU-time clock would report
    // almost nothing for this interval.
    const hsd::precise_clock before = hsd::precise_clock::now();
    hsd::this_thread::sleep_for(0.05f).unwrap();
    const hsd::precise_clock elapsed = hsd::precise_clock::now() - before;
    assert(elapsed.to_nanoseconds() >= 50'000'000);

    return 0;
}
