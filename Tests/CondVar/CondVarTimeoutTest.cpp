#include <Threading/ConditionVariable.hpp>
#include <cassert>

int main()
{
    using hsd::cv_detail::max_timeout_ms;
    using hsd::cv_detail::timeout_ms;

    // Native Windows waits take a relative timeout in milliseconds. The
    // conversion is platform-neutral, so it is checked here on every platform.
    const hsd::precise_clock now{timespec{1'000'000'000, 0}};

    auto ms_until = [&now](long sec, long nsec)
    {
        return timeout_ms(hsd::precise_clock{timespec{sec, nsec}}, now);
    };

    // Whole seconds and fractions convert exactly.
    assert(ms_until(1'000'000'005, 500'000'000) == 5500);
    assert(ms_until(1'000'000'000, 2'000'000) == 2);

    // A fraction of a millisecond rounds up, so a wait never ends early.
    assert(ms_until(1'000'000'000, 100'000) == 1);
    assert(ms_until(1'000'000'000, 1'000'001) == 2);

    // Deadlines already past, or exactly now, need no waiting.
    assert(ms_until(999'999'999, 0) == 0);
    assert(ms_until(1'000'000'000, 0) == 0);

    // Deadlines beyond what a native wait accepts are capped.
    assert(ms_until(2'000'000'000, 0) == max_timeout_ms);
    assert(ms_until(1'000'000'000 + 4'300'000, 0) == max_timeout_ms);
    // Exactly at the cap is still representable, so it is not clamped further.
    assert(ms_until(1'000'000'000 + 4'294'967, 294'000'000) == max_timeout_ms);

    return 0;
}
