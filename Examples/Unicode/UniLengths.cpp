// Title: Lengths in three encodings
// Level: Intermediate
// Shows: counting bytes, code points and UTF-16 units of UTF-8 strings.

#include <Base/Unicode.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

// Counts the elements before the terminator.
template <typename CharT>
hsd::usize count_until_null(const CharT* text)
{
    hsd::usize count = 0;

    while (text[count] != 0)
    {
        count++;
    }

    return count;
}

void report(const char* label, const hsd::char8* text)
{
    hsd::char32 code_points[64]{};
    hsd::char16 units[64]{};

    // Each call transcodes the same UTF-8 source. A destination that is too
    // small gives an error result, so the result is checked before the buffer is used.
    auto wide = hsd::unicode::convert_to(code_points, 64, text);
    auto utf16 = hsd::unicode::convert_to(units, 64, text);

    if (!wide.is_ok() || !utf16.is_ok())
    {
        hsd::println("{}: cannot convert"_fmt, label);
        return;
    }

    hsd::println(
        "{}: bytes={} code_points={} utf16_units={}"_fmt,
        label, count_until_null(text),
        count_until_null(code_points), count_until_null(units)
    );
}

int main()
{
    // The byte count grows with the code point value, and code points above
    // U+FFFF also need two UTF-16 units.
    report("ascii", u8"hello");
    report("accent", u8"añb");
    report("euro", u8"€5");
    report("emoji", u8"\U0001F600!");
    report("empty", u8"");

    return 0;
}
