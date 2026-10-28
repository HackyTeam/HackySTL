#include <cassert>

#include <Threading/Time.hpp>

// Regression tests for unit conversions: whole milliseconds and microseconds
// convert exactly, with no truncation from floating-point error.
int main()
{
    for (long value = 1; value <= 2000; ++value)
    {
        const hsd::precise_clock milliseconds{timespec{0, value * 1000000L}};
        assert(milliseconds.to_miliseconds() == value);
        assert(milliseconds.to_microseconds() == value * 1000);
        assert(milliseconds.to_nanoseconds() == static_cast<hsd::u64>(value) * 1000000ull);

        const hsd::precise_clock microseconds{timespec{0, value * 1000L}};
        assert(microseconds.to_microseconds() == value);
        assert(microseconds.to_miliseconds() == 0 || value >= 1000);
    }

    // Values with whole seconds keep their nanoseconds.
    const hsd::precise_clock mixed{timespec{3, 250000000L}};
    assert(mixed.to_nanoseconds() == 3250000000ull);
    assert(mixed.to_miliseconds() == 3250);
    assert(mixed.to_microseconds() == 3250000);
}
