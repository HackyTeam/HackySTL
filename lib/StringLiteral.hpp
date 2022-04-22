#pragma once

#include "Utility.hpp"

namespace hsd
{
    // Could be replaced by static_basic_string
    template <typename CharT, usize N>
    struct basic_string_literal
    {
        CharT data[N]{};
        using char_type = CharT;

        consteval basic_string_literal() = default;

        consteval basic_string_literal(const CharT (&str)[N])
        {
            copy_n(str, N, data);
        }

        consteval basic_string_literal(const CharT* str, usize len)
        {
            copy_n(str, len, data);
        }

        template <usize N2>
        consteval auto operator+(
            const basic_string_literal<CharT, N2>& rhs) const
        {
            basic_string_literal<CharT, N + N2 - 1> _str;
            copy_n(data, N, _str.data);
            copy_n(rhs.data, N2, (_str.data + N - 1));
            return _str;
        }

        template <usize N2>
        consteval auto operator+(const CharT (&rhs)[N2]) const
        {
            basic_string_literal<CharT, N + N2 - 1> _str;
            copy_n(data, N, _str.data);
            copy_n(rhs, N2, (_str.data + N - 1));
            return _str;
        }

        template <usize N2>
        consteval friend auto operator+(
            const CharT (&lhs)[N2], 
            const basic_string_literal& rhs)
        {
            basic_string_literal<CharT, N + N2 - 1> _str;
            copy_n(lhs, N2, _str.data);
            copy_n(rhs.data, N, (_str.data + N2 - 1));
            return _str;
        }

        consteval usize size() const
        {
            return N;
        }

        consteval const auto* begin() const
        {
            return data;
        }
        
        consteval const auto* end() const
        {
            return data + N;
        }
    };

    template <usize N>
    using string_literal = basic_string_literal<char, N>;
    template <usize N>
    using wstring_literal = basic_string_literal<wchar, N>;
    template <usize N>
    using u8string_literal = basic_string_literal<char8, N>;
    template <usize N>
    using u16string_literal = basic_string_literal<char16, N>;
    template <usize N>
    using u32string_literal = basic_string_literal<char32, N>;

    template <usize N>
    basic_string_literal(const char (&)[N]) -> basic_string_literal<char, N>;

    template <usize N>
    basic_string_literal(const wchar (&)[N]) -> basic_string_literal<wchar, N>;

    template <usize N>
    basic_string_literal(const char8 (&)[N]) -> basic_string_literal<char8, N>;

    template <usize N>
    basic_string_literal(const char16 (&)[N]) -> basic_string_literal<char16, N>;

    template <usize N>
    basic_string_literal(const char32 (&)[N]) -> basic_string_literal<char32, N>;
} // namespace hsd
