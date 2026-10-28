// Title: Transcoding pipeline
// Level: Advanced
// Shows: UTF-8 to UTF-16 to UTF-32 and back, error results for small buffers, and constexpr conversion.

#include <Base/Unicode.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;

template <typename CharT>
constexpr bool same_text(const CharT* lhs, const CharT* rhs)
{
    hsd::usize index = 0;

    while (lhs[index] != 0 && lhs[index] == rhs[index])
    {
        index++;
    }

    return lhs[index] == rhs[index];
}

// Converting inside a constant expression works too, since convert_to is constexpr.
constexpr bool euro_sign_is_one_code_point()
{
    hsd::char32 out[4]{};
    auto res = hsd::unicode::convert_to(out, 4, u8"€");
    return res.is_ok() && out[0] == 0x20ac && out[1] == 0;
}
static_assert(euro_sign_is_one_code_point());

int main()
{
    const hsd::char8* original = u8"naïve € \U0001F680";

    // Each stage reads the output of the previous one.
    hsd::char16 utf16[32]{};
    hsd::char32 utf32[32]{};
    hsd::char8 utf8_again[32]{};
    hsd::char16 utf16_again[32]{};
    hsd::char8 utf8_final[32]{};

    bool ok = hsd::unicode::convert_to(utf16, 32, original).is_ok();
    ok = ok && hsd::unicode::convert_to(utf32, 32, utf16).is_ok();
    ok = ok && hsd::unicode::convert_to(utf8_again, 32, utf32).is_ok();
    ok = ok && hsd::unicode::convert_to(utf16_again, 32, utf8_again).is_ok();
    ok = ok && hsd::unicode::convert_to(utf8_final, 32, utf16_again).is_ok();

    if (!ok)
    {
        hsd::println("pipeline failed"_fmt);
        return 0;
    }

    hsd::print("utf-32:"_fmt);
    for (hsd::usize index = 0; utf32[index] != 0; index++)
    {
        hsd::print(" {hex}"_fmt, static_cast<hsd::u32>(utf32[index]));
    }
    hsd::println(""_fmt);

    // A lossless pipeline returns the same bytes it started with.
    hsd::println("utf-8 round trip: {}"_fmt,
        same_text(original, utf8_again) ? "true" : "false");
    hsd::println("utf-16 round trip: {}"_fmt,
        same_text(utf16, utf16_again) ? "true" : "false");
    hsd::println("long round trip: {}"_fmt,
        same_text(original, utf8_final) ? "true" : "false");

    // length<> measures the output size before converting, so a buffer can be
    // checked against it. The terminator needs one more element.
    hsd::usize needed = hsd::unicode::length<hsd::char8>(utf32);
    hsd::println("utf-8 bytes needed: {}"_fmt, needed);

    // A destination without enough room is reported as an error result.
    const hsd::char32 rocket[] = U"\U0001F680";
    hsd::char8 tiny[4]{};
    auto small = hsd::unicode::convert_to(tiny, 4, rocket);
    hsd::println("tiny buffer ok: {}"_fmt, small.is_ok() ? "true" : "false");
    if (!small.is_ok())
    {
        hsd::println("reason: {}"_fmt, small.unwrap_err().pretty_error());
    }

    // The same text fits once the buffer is large enough.
    hsd::char8 roomy[8]{};
    auto fits = hsd::unicode::convert_to(roomy, 8, rocket);
    hsd::println("roomy buffer ok: {}"_fmt, fits.is_ok() ? "true" : "false");

    return 0;
}
