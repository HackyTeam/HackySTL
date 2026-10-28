#pragma once

/// @file StringLiteral.hpp
/// @brief Structural compile-time string literal values.
/// @details Stores character arrays as class-type non-type template parameters
/// so format and parser descriptions can be evaluated at compile time.

#include "../Base/Utility.hpp"

namespace hsd
{
    // Could be replaced by static_basic_string
    /// @brief Stores a fixed character array as a structural compile-time value.
    /// @tparam CharT Character type in the literal.
    /// @tparam N Number of stored elements, including a source terminator when
    ///           the value was initialized from an array literal.
    template <typename CharT, usize N>
    struct basic_string_literal
    {
        /// @brief Characters carried by this structural value.
        CharT data[N]{};
        /// @brief Character type stored in the value.
        using char_type = CharT;

        /// @brief Constructs a value with value-initialized characters.
        consteval basic_string_literal() = default;

        /// @brief Copies an array literal into the structural value.
        /// @param str Source character array.
        consteval basic_string_literal(const CharT (&str)[N])
        {
            copy_n(str, N, data);
        }

        /// @brief Copies a bounded character sequence into the value.
        /// @param str Source character sequence.
        /// @param len Number of characters copied from `str`.
        consteval basic_string_literal(const CharT* str, usize len)
        {
            copy_n(str, len, data);
        }

        /// @brief Concatenates two literals while sharing their join terminator.
        /// @param rhs Literal appended to this value.
        /// @return A new literal containing both character sequences.
        template <usize N2>
        consteval auto operator+(
            const basic_string_literal<CharT, N2>& rhs) const
        {
            basic_string_literal<CharT, N + N2 - 1> _str;
            copy_n(data, N, _str.data);
            copy_n(rhs.data, N2, (_str.data + N - 1));
            return _str;
        }

        /// @brief Appends a character array while sharing the join terminator.
        /// @param rhs Array literal appended to this value.
        /// @return A new literal containing both character sequences.
        template <usize N2>
        consteval auto operator+(const CharT (&rhs)[N2]) const
        {
            basic_string_literal<CharT, N + N2 - 1> _str;
            copy_n(data, N, _str.data);
            copy_n(rhs, N2, (_str.data + N - 1));
            return _str;
        }

        /// @brief Prepends a character array while sharing the join terminator.
        /// @param lhs Array literal placed before this value.
        /// @param rhs Literal receiving the prepended array.
        /// @return A new literal containing both character sequences.
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

        /// @brief Returns the number of stored elements.
        /// @return The compile-time extent `N`.
        constexpr usize size() const
        {
            return N;
        }

        /// @brief Returns a pointer to the first stored character.
        /// @return Pointer to `data`.
        constexpr const auto* begin() const
        {
            return data;
        }
        
        /// @brief Returns one past the last stored character.
        /// @return Pointer to `data + N`.
        constexpr const auto* end() const
        {
            return data + N;
        }
    };

    namespace format_literals
    {
        /// @brief User-defined literal building a lazily-evaluated format literal.
        /// @tparam fmt - Structural `basic_string_literal` parsed from the literal text.
        /// @return A closure returning `fmt`, evaluated where the format is consumed.
        template <hsd::basic_string_literal fmt>
        static constexpr auto operator""_fmt()
        {
            return [] { return fmt; };
        }
    } // namespace format_literals

    /// @brief Character literal alias for `basic_string_literal`.
    template <usize N>
    using string_literal = basic_string_literal<char, N>;
    /// @brief Wide-character literal alias for `basic_string_literal`.
    template <usize N>
    using wstring_literal = basic_string_literal<wchar, N>;
    /// @brief UTF-8 literal alias for `basic_string_literal`.
    template <usize N>
    using u8string_literal = basic_string_literal<char8, N>;
    /// @brief UTF-16 literal alias for `basic_string_literal`.
    template <usize N>
    using u16string_literal = basic_string_literal<char16, N>;
    /// @brief UTF-32 literal alias for `basic_string_literal`.
    template <usize N>
    using u32string_literal = basic_string_literal<char32, N>;

    /// @brief Deduction guide for narrow character arrays.
    /// @param str - Array literal whose type is deduced.
    template <usize N>
    basic_string_literal(const char (&str)[N]) -> basic_string_literal<char, N>;

    /// @brief Deduction guide for wide character arrays.
    /// @param str - Array literal whose type is deduced.
    template <usize N>
    basic_string_literal(const wchar (&str)[N]) -> basic_string_literal<wchar, N>;

    /// @brief Deduction guide for UTF-8 character arrays.
    /// @param str - Array literal whose type is deduced.
    template <usize N>
    basic_string_literal(const char8 (&str)[N]) -> basic_string_literal<char8, N>;

    /// @brief Deduction guide for UTF-16 character arrays.
    /// @param str - Array literal whose type is deduced.
    template <usize N>
    basic_string_literal(const char16 (&str)[N]) -> basic_string_literal<char16, N>;

    /// @brief Deduction guide for UTF-32 character arrays.
    /// @param str - Array literal whose type is deduced.
    template <usize N>
    basic_string_literal(const char32 (&str)[N]) -> basic_string_literal<char32, N>;
} // namespace hsd
