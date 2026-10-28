// Title: Big factorials
// Level: Intermediate
// Shows: integer<512> products read from stdin, digit sums by division, and 64-bit wrap-around.

#include <Containers/Integer.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

using big = hsd::integer<512>;

constexpr hsd::u32 max_n = 80;

// 80! has 119 digits, which fits in 512 bits (about 154 digits).
static big factorial(hsd::u32 n)
{
    big result = 1;
    for (hsd::u32 factor = 2; factor <= n; ++factor)
    {
        result *= factor;
    }
    return result;
}

struct digit_stats
{
    hsd::usize digits = 0;
    hsd::usize sum = 0;
    hsd::usize trailing_zeros = 0;
};

// Peels digits off the end by dividing by ten until nothing is left.
static digit_stats inspect(big value)
{
    digit_stats stats;
    bool in_zero_run = true;
    const big ten = 10;

    while (value.any())
    {
        auto division = value.divide_with_reminder(ten).unwrap();
        const hsd::u32 digit = division.reminder.get_last_byte();

        ++stats.digits;
        stats.sum += digit;

        if (in_zero_run && digit == 0)
        {
            ++stats.trailing_zeros;
        }
        else
        {
            in_zero_run = false;
        }

        value = division.quotient;
    }

    return stats;
}

// The same product in 64 bits silently wraps once it passes 2^64.
static hsd::u64 factorial_wrapped(hsd::u32 n)
{
    hsd::u64 result = 1;
    for (hsd::u32 factor = 2; factor <= n; ++factor)
    {
        result *= factor;
    }
    return result;
}

int main()
{
    // One n per line. The stream hands over a value once more input follows
    // it, so the input finishes with a 0 that also stops the loop.
    for (auto value = hsd::io::cin().read_value<hsd::u32>(); value.is_ok();
        value = hsd::io::cin().read_value<hsd::u32>())
    {
        const hsd::u32 n = value.unwrap();

        if (n == 0)
        {
            break;
        }

        if (n > max_n)
        {
            hsd::println("{} is too large (limit {})"_fmt, n, max_n);
            continue;
        }

        const big result = factorial(n);
        const digit_stats stats = inspect(result);

        hsd::println("{}! = {}"_fmt, n, result);
        hsd::println("  digits {} digit sum {} trailing zeros {}"_fmt,
            stats.digits, stats.sum, stats.trailing_zeros);

        const big wrapped = factorial_wrapped(n);
        hsd::println("  64-bit product is correct: {}"_fmt,
            wrapped == result ? "true" : "false");
    }

    return 0;
}
