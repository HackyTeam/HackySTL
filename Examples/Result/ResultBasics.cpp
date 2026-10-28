// Title: Result basics
// Level: Basic
// Shows: result, option and option_err, plus the unwrap_or family of recovery calls.

#include <Base/Result.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// result<Ok, Err> carries either a value or an error, and Ok must differ from Err.
static hsd::result<hsd::i32, hsd::runtime_error> divide(hsd::i32 top, hsd::i32 bottom)
{
    if (bottom == 0)
    {
        return hsd::runtime_error{"division by zero"};
    }
    return top / bottom;
}

// option<T> is a value that may be missing, with no error payload.
static hsd::option<hsd::i32> first_negative(const hsd::i32* values, hsd::usize count)
{
    for (hsd::usize index = 0; index < count; ++index)
    {
        if (values[index] < 0)
        {
            return values[index];
        }
    }
    return hsd::err_value{};
}

// option_err<E> is the reverse: nothing to return, but it may fail.
static hsd::option_err<hsd::runtime_error> check_even(hsd::i32 value)
{
    if (value % 2 != 0)
    {
        return hsd::runtime_error{"value is odd"};
    }
    return {};
}

int main()
{
    auto good = divide(84, 2);
    if (good.is_ok())
    {
        hsd::println("84 / 2 = {}"_fmt, good.unwrap());
    }

    auto bad = divide(1, 0);
    if (!bad.is_ok())
    {
        hsd::println("1 / 0 failed: {}"_fmt, bad.unwrap_err().pretty_error());
    }

    // Recover from a failure with a value, or with a function run only when needed.
    hsd::println("unwrap_or: {}"_fmt, divide(1, 0).unwrap_or(-1));
    hsd::println("unwrap_or_else: {}"_fmt, divide(1, 0).unwrap_or_else([] { return 99; }));
    hsd::println("unwrap_or_default: {}"_fmt, divide(1, 0).unwrap_or_default());

    const hsd::i32 mixed[]{4, 7, -3, 8};
    const hsd::i32 positive[]{4, 7, 8};
    hsd::println("negative in mixed: {}"_fmt, first_negative(mixed, 4).unwrap_or(0));
    hsd::println("negative in positive: {}"_fmt,
        first_negative(positive, 3).is_ok() ? "found" : "none");

    const hsd::i32 samples[]{6, 7};
    for (hsd::i32 value : samples)
    {
        auto verdict = check_even(value);
        hsd::println("{} -> {}"_fmt, value,
            verdict.is_ok() ? "even" : verdict.unwrap_err().pretty_error());
    }

    return 0;
}
