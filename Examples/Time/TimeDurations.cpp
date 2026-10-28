// Title: Durations
// Level: Intermediate
// Shows: precise_clock literals, arithmetic with carry, comparison, and unit conversion.

#include <Serialize/Io.hpp>
#include <Threading/Time.hpp>

using namespace hsd::format_literals;
using namespace hsd::precise_clock_literals;

// Prints a clock in each unit it can convert to.
void describe(const char* label, const hsd::precise_clock& duration)
{
    hsd::println("{}: {} ns, {} us, {} ms, {prec=3} s"_fmt, label,
        duration.to_nanoseconds(), duration.to_microseconds(),
        duration.to_miliseconds(), duration.to_seconds());
}

int main()
{
    constexpr auto quarter = 250_ms;
    constexpr auto three_quarters = 750_ms;

    describe("quarter", quarter);
    describe("three quarters", three_quarters);

    // Adding two parts of a second carries into the seconds field.
    constexpr auto one_second = quarter + three_quarters;
    describe("together", one_second);

    // Subtracting borrows from the seconds field when needed.
    constexpr auto difference = 2_s - 250_ms;
    describe("difference", difference);

    describe("microseconds literal", 2000_us);
    describe("nanoseconds literal", 42_ns);

    hsd::println("quarter < three quarters: {}"_fmt, quarter < three_quarters ? "true" : "false");
    hsd::println("parts add up to one second: {}"_fmt, one_second == 1_s ? "true" : "false");
    hsd::println("difference >= 1750 ms: {}"_fmt, difference >= 1750_ms ? "true" : "false");

    // Whole milliseconds and microseconds convert exactly, with no rounding loss.
    hsd::println("1001 ms literal: {} ms, {} us"_fmt,
        (1001_ms).to_miliseconds(), (1001_ms).to_microseconds());
    hsd::println("249 us literal: {} us"_fmt, (249_us).to_microseconds());

    // The compile-time checks use the underlying timespec.
    static_assert(one_second.to_native().tv_sec == 1);
    static_assert(one_second.to_native().tv_nsec == 0);
    static_assert(difference.to_native().tv_sec == 1);
    static_assert(difference.to_native().tv_nsec == 750'000'000);

    // The coarse hsd::clock has its own literals, measured in clock ticks.
    constexpr hsd::clock one_clock_second{CLOCKS_PER_SEC};
    hsd::println("clock seconds: {prec=1}"_fmt, one_clock_second.to_seconds());
    return 0;
}
