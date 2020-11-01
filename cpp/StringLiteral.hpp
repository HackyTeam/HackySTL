#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename CharT, usize N>
    struct basic_string_literal
    {
        CharT data[N]{};
        using char_type = CharT;

        consteval basic_string_literal() {}

        consteval basic_string_literal(const CharT (&str)[N])
        {
            copy_n(str, N, data);
        }

        template <usize L1, usize L2>
        consteval basic_string_literal(const basic_string_literal<CharT, L1>& strr, const CharT (&strl)[L2])
        {
            copy_n(strr.data, L1, data);
            copy_n(strl, L2, (data + L1 - 1));
        }

        consteval basic_string_literal(const CharT* str, usize len)
        {
            copy_n(str, len, data);
        }

        consteval usize size() const
        {
            return N;
        }
    };

    template <typename CharT, usize L1, usize L2>
    basic_string_literal(const basic_string_literal<CharT, L1>, const CharT (&str)[L2])
        -> basic_string_literal<CharT, L1 + L2>;

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
} // namespace hsd
