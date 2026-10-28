// Title: Unicode conversion
// Level: Basic
// Shows: convert_to from UTF-8 into UTF-32 and UTF-16, printing the code points.

#include <Base/Unicode.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

int main()
{
    // The source file is UTF-8, so a u8 literal holds the encoded bytes.
    const hsd::char8* text = u8"aé€😀";

    // One call transcodes a null-terminated string. The size is the capacity
    // of the destination, including room for the terminator.
    hsd::char32 code_points[16]{};
    auto utf32 = hsd::unicode::convert_to(code_points, 16, text);

    if (!utf32.is_ok())
    {
        hsd::println("conversion failed"_fmt);
        return 0;
    }

    hsd::print("code points:"_fmt);
    for (hsd::usize index = 0; code_points[index] != 0; index++)
    {
        hsd::print(" {hex}"_fmt, static_cast<hsd::u32>(code_points[index]));
    }
    hsd::println(""_fmt);

    // Characters above U+FFFF need two UTF-16 units, called a surrogate pair.
    hsd::char16 units[16]{};
    hsd::unicode::convert_to(units, 16, text).unwrap();

    hsd::print("utf-16 units:"_fmt);
    for (hsd::usize index = 0; units[index] != 0; index++)
    {
        hsd::print(" {hex}"_fmt, static_cast<hsd::u32>(units[index]));
    }
    hsd::println(""_fmt);

    // 'a' takes one UTF-8 byte and the accented letter takes two, so the text
    // needs four units with its terminator: exactly the buffer size here.
    const hsd::char32 edge_text[] = {U'a', U'\u00e9', 0};
    hsd::char8 edge_out[4]{};
    auto edge_fit = hsd::unicode::convert_to(edge_out, 4, edge_text);
    hsd::println("exact fit: {}"_fmt, edge_fit.is_ok() ? "true" : "false");

    hsd::char8 edge_small[3]{};
    auto edge_short = hsd::unicode::convert_to(edge_small, 3, edge_text);
    hsd::println("one unit short: {}"_fmt, edge_short.is_ok() ? "true" : "false");
    return 0;
}
