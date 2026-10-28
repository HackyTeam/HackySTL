#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

#include <Base/Unicode.hpp>

// Regression tests for conversions between UTF-8, UTF-16 and UTF-32 units.
// Each case is checked against an encoder written separately from the library.

namespace
{
    using code_point_list = std::vector<std::uint32_t>;

    template <typename Unit>
    std::vector<Unit> encode_all(const code_point_list& code_points)
    {
        std::vector<Unit> out;
        for (const std::uint32_t cp : code_points)
        {
            if constexpr (sizeof(Unit) == 1)
            {
                if (cp < 0x80)
                {
                    out.push_back(static_cast<Unit>(cp));
                }
                else if (cp < 0x800)
                {
                    out.push_back(static_cast<Unit>(0xC0 | (cp >> 6)));
                    out.push_back(static_cast<Unit>(0x80 | (cp & 0x3F)));
                }
                else if (cp < 0x10000)
                {
                    out.push_back(static_cast<Unit>(0xE0 | (cp >> 12)));
                    out.push_back(static_cast<Unit>(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(static_cast<Unit>(0x80 | (cp & 0x3F)));
                }
                else
                {
                    out.push_back(static_cast<Unit>(0xF0 | (cp >> 18)));
                    out.push_back(static_cast<Unit>(0x80 | ((cp >> 12) & 0x3F)));
                    out.push_back(static_cast<Unit>(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(static_cast<Unit>(0x80 | (cp & 0x3F)));
                }
            }
            else if constexpr (sizeof(Unit) == 2)
            {
                if (cp < 0x10000)
                {
                    out.push_back(static_cast<Unit>(cp));
                }
                else
                {
                    const std::uint32_t offset = cp - 0x10000;
                    out.push_back(static_cast<Unit>(0xD800 | (offset >> 10)));
                    out.push_back(static_cast<Unit>(0xDC00 | (offset & 0x3FF)));
                }
            }
            else
            {
                out.push_back(static_cast<Unit>(cp));
            }
        }
        return out;
    }

    code_point_list sample_code_points()
    {
        code_point_list values = {
            0x01, 0x41, 0x7F, 0x80, 0x7FF, 0x800, 0xD7FF,
            0xE000, 0xFFFD, 0xFFFF, 0x10000, 0x1F600, 0x10FFFF
        };

        std::mt19937 engine{20260501};
        std::uniform_int_distribution<std::uint32_t> any{0, 0x10FFFF};
        while (values.size() < 600)
        {
            const std::uint32_t cp = any(engine);
            if (cp < 0xD800 || cp > 0xDFFF)
            {
                values.push_back(cp);
            }
        }
        return values;
    }

    // Converts `code_points` from Src units to Dest units, first with a buffer
    // that fits exactly, then with one unit too few. Nothing may be written at
    // or past the size passed in, which the guard region after the buffer checks.
    template <typename Dest, typename Src>
    void check_pair(const code_point_list& code_points)
    {
        const auto source = encode_all<Src>(code_points);
        const auto expected = encode_all<Dest>(code_points);

        std::vector<Src> terminated = source;
        terminated.push_back(static_cast<Src>(0));

        const auto measured = hsd::unicode::length<Dest, Src>(terminated.data());
        const auto bounded_measure = hsd::unicode::length<Dest, Src>(source.data(), source.size());
        assert(measured == expected.size());
        assert(bounded_measure == expected.size());

        constexpr std::size_t guard = 8;
        const Dest sentinel = static_cast<Dest>(~std::uint32_t{0});

        std::vector<Dest> exact(expected.size() + 1 + guard, sentinel);
        auto result = hsd::unicode::convert_to(
            exact.data(), expected.size() + 1, terminated.data());
        assert(result.is_ok());
        assert(result.unwrap() == 0);
        for (std::size_t index = 0; index < expected.size(); ++index)
        {
            assert(exact[index] == expected[index]);
        }
        assert(exact[expected.size()] == static_cast<Dest>(0));
        for (std::size_t index = expected.size() + 1; index < exact.size(); ++index)
        {
            assert(exact[index] == sentinel);
        }

        std::vector<Dest> bounded(expected.size() + 1 + guard, sentinel);
        auto bounded_result = hsd::unicode::convert_to(
            bounded.data(), expected.size() + 1, source.data(), source.size());
        assert(bounded_result.is_ok());
        assert(bounded_result.unwrap() == 0);
        for (std::size_t index = 0; index < expected.size(); ++index)
        {
            assert(bounded[index] == expected[index]);
        }

        if (!expected.empty())
        {
            std::vector<Dest> short_buffer(expected.size() + 1 + guard, sentinel);
            auto too_small = hsd::unicode::convert_to(
                short_buffer.data(), expected.size(), terminated.data());
            assert(!too_small.is_ok());
            for (std::size_t index = expected.size(); index < short_buffer.size(); ++index)
            {
                assert(short_buffer[index] == sentinel);
            }
        }
    }

    template <typename Dest>
    void check_all_sources(const code_point_list& code_points)
    {
        check_pair<Dest, hsd::char8>(code_points);
        check_pair<Dest, hsd::char16>(code_points);
        check_pair<Dest, hsd::char32>(code_points);
    }

    // Each sample is encoded alone, so the buffer boundaries land on every
    // length from one to four units.
    void check_every_sample_alone()
    {
        for (const std::uint32_t cp : sample_code_points())
        {
            const code_point_list single{cp};
            check_all_sources<hsd::char8>(single);
            check_all_sources<hsd::char16>(single);
            check_all_sources<hsd::char32>(single);
        }
    }

    void check_known_encodings()
    {
        // U+20AC EURO SIGN is E2 82 AC in UTF-8 and 20AC in UTF-16.
        const hsd::char8 euro_utf8[] = {0xE2, 0x82, 0xAC, 0};
        hsd::char16 euro_utf16[4]{};
        assert(hsd::unicode::convert_to(euro_utf16, 4, euro_utf8).unwrap() == 2);
        assert(euro_utf16[0] == 0x20AC && euro_utf16[1] == 0);

        // U+1F600 is F0 9F 98 80 in UTF-8 and D83D DE00 in UTF-16.
        const hsd::char8 smile_utf8[] = {0xF0, 0x9F, 0x98, 0x80, 0};
        hsd::char32 smile_utf32[2]{};
        assert(hsd::unicode::convert_to(smile_utf32, 2, smile_utf8).unwrap() == 0);
        assert(smile_utf32[0] == 0x1F600);

        const hsd::char16 smile_utf16[] = {0xD83D, 0xDE00, 0};
        hsd::char8 smile_back[5]{};
        assert(hsd::unicode::convert_to(smile_back, 5, smile_utf16).unwrap() == 0);
        assert(smile_back[0] == 0xF0 && smile_back[3] == 0x80 && smile_back[4] == 0);
    }

    void check_empty_input()
    {
        hsd::char8 out[2]{0x55, 0x55};
        auto empty = hsd::unicode::convert_to(out, 2, static_cast<const hsd::char32*>(nullptr), 0);
        assert(empty.is_ok());
        assert(empty.unwrap() == 1);
        assert(out[0] == 0);

        const char* text = "";
        hsd::char16 wide[1]{0x55};
        auto blank = hsd::unicode::convert_to(wide, 1, text, 0);
        assert(blank.is_ok() && blank.unwrap() == 0);
        assert(wide[0] == 0);

        auto nothing = hsd::unicode::convert_to(wide, 0, text, 0);
        assert(!nothing.is_ok());
    }

    void check_malformed_input_is_rejected()
    {
        hsd::char32 wide[4]{};

        const hsd::char8 lone_continuation[] = {0x80, 0};
        assert(!hsd::unicode::convert_to(wide, 4, lone_continuation).is_ok());

        const hsd::char8 overlong[] = {0xC0, 0x80, 0};
        assert(!hsd::unicode::convert_to(wide, 4, overlong).is_ok());

        const hsd::char8 encoded_surrogate[] = {0xED, 0xA0, 0x80, 0};
        assert(!hsd::unicode::convert_to(wide, 4, encoded_surrogate).is_ok());

        const hsd::char8 above_max[] = {0xF4, 0x90, 0x80, 0x80, 0};
        assert(!hsd::unicode::convert_to(wide, 4, above_max).is_ok());

        const hsd::char8 truncated[] = {0xE2, 0x82, 0};
        assert(!hsd::unicode::convert_to(wide, 4, truncated).is_ok());

        // A bounded source must not be read past its limit, even when the
        // bytes after it would complete the sequence.
        const hsd::char8 cut_short[] = {0xE2, 0x82, 0xAC};
        assert(!hsd::unicode::convert_to(wide, 4, cut_short, 2).is_ok());
        assert(hsd::unicode::convert_to(wide, 4, cut_short, 3).is_ok());

        const hsd::char16 lone_high[] = {0xD800, 'A', 0};
        assert(!hsd::unicode::convert_to(wide, 4, lone_high).is_ok());

        const hsd::char16 lone_low[] = {0xDC00, 0};
        assert(!hsd::unicode::convert_to(wide, 4, lone_low).is_ok());

        const hsd::char16 high_at_end[] = {0xD83D};
        assert(!hsd::unicode::convert_to(wide, 4, high_at_end, 1).is_ok());

        const hsd::char32 beyond_unicode[] = {0x110000, 0};
        hsd::char8 narrow[8]{};
        assert(!hsd::unicode::convert_to(narrow, 8, beyond_unicode).is_ok());

        const hsd::char32 surrogate_value[] = {0xD800, 0};
        assert(!hsd::unicode::convert_to(narrow, 8, surrogate_value).is_ok());
    }

    void check_stops_at_null_before_limit()
    {
        // A null unit ends the source even when `src_len` allows more.
        const hsd::char8 text[] = {'a', 0, 'b', 'c'};
        hsd::char16 out[8]{0x55};
        auto result = hsd::unicode::convert_to(out, 8, text, 4);
        assert(result.is_ok());
        assert(result.unwrap() == 6);
        assert(out[0] == 'a' && out[1] == 0);
    }
}

int main()
{
    check_known_encodings();
    check_empty_input();
    check_malformed_input_is_rejected();
    check_stops_at_null_before_limit();

    check_every_sample_alone();
    check_all_sources<hsd::char8>(sample_code_points());
    check_all_sources<hsd::char16>(sample_code_points());
    check_all_sources<hsd::char32>(sample_code_points());
}
