// Title: Parsing with results
// Level: Intermediate
// Shows: chaining failures by hand across functions and recovering with unwrap_or_else.

#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// There is no map or and_then: each step checks is_ok and passes the error up.
static hsd::result<hsd::i32, hsd::runtime_error> parse_number(
    const hsd::string& text, hsd::usize from, hsd::usize to)
{
    if (from >= to)
    {
        return hsd::runtime_error{"missing number"};
    }

    bool negative = text[from] == '-';
    hsd::i32 value = 0;

    for (hsd::usize index = negative ? from + 1 : from; index < to; ++index)
    {
        if (text[index] < '0' || text[index] > '9')
        {
            return hsd::runtime_error{"not a number"};
        }
        value = value * 10 + (text[index] - '0');
    }

    return negative ? -value : value;
}

// A line holds two numbers separated by one space.
static hsd::result<hsd::i32, hsd::runtime_error> sum_line(const hsd::string& line)
{
    hsd::usize split = line.find(' ');
    if (split == hsd::string::npos)
    {
        return hsd::runtime_error{"expected two numbers"};
    }

    auto left = parse_number(line, 0, split);
    if (!left.is_ok())
    {
        return left.unwrap_err();
    }

    auto right = parse_number(line, split + 1, line.length());
    if (!right.is_ok())
    {
        return right.unwrap_err();
    }

    return left.unwrap() + right.unwrap();
}

int main()
{
    hsd::i32 total = 0;
    hsd::i32 failures = 0;

    const char* lines[]{"1 2", "-5 10", "abc 4", "12", "3 ", "40 -2", "7 99999x"};

    for (const char* raw : lines)
    {
        hsd::string text{raw};
        auto sum = sum_line(text);

        if (sum.is_ok())
        {
            hsd::i32 value = sum.unwrap();
            total += value;
            hsd::println("'{}' = {}"_fmt, text.c_str(), value);
        }
        else
        {
            ++failures;
            hsd::println("'{}' -> {}"_fmt, text.c_str(), sum.unwrap_err().pretty_error());
        }
    }

    hsd::println("total: {} failures: {}"_fmt, total, failures);

    // unwrap_or_else only runs its function on failure, which suits a costly default.
    hsd::string failing{"7 x"};
    hsd::i32 fallback = sum_line(failing).unwrap_or_else([&] { return -1; });
    hsd::println("fallback: {}"_fmt, fallback);

    return 0;
}
