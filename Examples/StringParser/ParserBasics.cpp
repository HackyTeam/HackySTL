// Title: Parsing text with a pattern
// Level: Basic
// Shows: parse_string with {} placeholders, typed field conversion, and mismatches.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Serialize/Io.hpp>
#include <Serialize/StringParser.hpp>

using namespace hsd::format_literals;

int main()
{
    // The pattern is a template argument, and the second argument is the number
    // of {} placeholders. The text between placeholders must match exactly.
    auto point = hsd::parse_string<"({}, {})", 2>("(12, -7)");

    if (point.is_ok())
    {
        auto fields = point.unwrap();
        hsd::i32 x = fields[0].to_i32();
        hsd::i32 y = fields[1].to_i32();
        hsd::println("x={} y={} sum={}"_fmt, x, y, x + y);
    }

    // Each field is a view into the input, converted when you ask for a type.
    auto date = hsd::parse_string<"{}-{}-{}", 3>("2024-03-15").unwrap();
    hsd::println("year: {}, month: {}, day: {}"_fmt,
        date[0].to_u32(), date[1].to_u32(), date[2].to_u32());

    auto pair = hsd::parse_string<"{} = {}", 2>("ratio = 0.75").unwrap();
    hsd::println("{} is {prec=2}"_fmt, pair[0].to_string(), pair[1].to_f32());
    hsd::println("name length: {}"_fmt, pair[0].to_string_view().length());

    // A line that does not follow the pattern gives a failed result.
    auto broken = hsd::parse_string<"({}, {})", 2>("[12; -7]");
    hsd::println("broken line parsed: {}"_fmt, broken.is_ok() ? "true" : "false");

    return 0;
}
