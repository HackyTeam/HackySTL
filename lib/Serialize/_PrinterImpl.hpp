#pragma once

/// @file _PrinterImpl.hpp
/// @brief Internal type-dispatch implementation for formatted output.
/// @details Selects printers for scalar, string, Unicode, tuple-like, and
/// project-specific pretty-argument values.

#include "FormatGenerator.hpp"
#include "ToString.hpp"
#include "../Base/Unicode.hpp"

namespace hsd
{
    /// @brief Character type used by a resolved `format_output` instantiation.
    template <typename FmtType>
    using fmt_char_type = typename FmtType::template lit_type<1>::char_type;

    /// @brief Constrains a type to an integral value excluding character types.
    template <typename T>
    concept FormatIntegral = IsIntegral<T> && !IsChar<T>;

    ///
    /// @brief Tells whether the presentation tag of `fmt` can format an integral value.
    ///
    /// @tparam fmt - Resolved `format_output` whose tag is checked.
    /// @return True for no tag, or for hexadecimal without a precision.
    ///
    template <format_output fmt>
    constexpr bool integral_tag_ok()
    {
        using FmtType = decltype(fmt);

        return fmt.tag == FmtType::none
            || ((fmt.tag & FmtType::hex) == FmtType::hex && (fmt.tag & FmtType::prec) != FmtType::prec);
    }

    ///
    /// @brief Tells whether the presentation tag of `fmt` can format a floating-point value.
    ///
    /// @tparam fmt - Resolved `format_output` whose tag is checked.
    /// @return True for no tag, hexadecimal, or exponent. A precision is allowed with each.
    ///
    template <format_output fmt>
    constexpr bool floating_tag_ok()
    {
        using FmtType = decltype(fmt);
        const usize _tag = fmt.tag & ~FmtType::prec;

        return _tag == FmtType::none
            || (_tag & FmtType::hex) == FmtType::hex
            || (_tag & FmtType::exp) == FmtType::exp;
    }

    ///
    /// @brief Copies an iterable character sequence into a raw buffer.
    ///
    /// @tparam CharT - Character type written to `dest`.
    /// @param dest - Destination buffer; must have room for every element of `str`.
    /// @param str - Iterable character sequence to copy.
    /// @return Pointer to the last character written (`dest + size(str) - 1`),
    /// not one past the end.
    ///
    template <typename CharT>
    static constexpr CharT* _print_to_impl(CharT* dest, const auto& str)
    {
        for (const CharT _chr : str)
        {
            *dest = _chr;
            ++dest;
        }

        return dest - 1;
    }

    ///
    /// @brief Writes a format's literal text, style, and reset escapes with no value.
    ///
    /// @tparam fmt - Resolved `format_output` describing the literal to print.
    /// @tparam CharT - Character type of `str`; must match `fmt`'s character type.
    /// @param str - Destination buffer to write into.
    /// @param sz - Number of characters available at `str`.
    /// @return Number of characters written, or a `runtime_error` when `sz` is too small.
    /// @details Only accepts a format with no presentation tag, since there is no value to format.
    ///
    template <format_output fmt, typename CharT>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz)
    {
        usize _write_sz = (fmt.format.size() + fmt.style.size() + fmt.reset.size() - 3);

        // The terminator written after the text takes one position more than its characters.
        if (_write_sz + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);
        str = _print_to_impl(str, fmt.reset);

        return _write_sz;
    }

    ///
    /// @brief Writes a format's literal text, style, and reset escapes around
    /// an integral value.
    ///
    /// @tparam fmt - Resolved `format_output` describing the literal to print.
    /// @tparam CharT - Character type of `str`; must match `fmt`'s character type.
    /// @tparam T - Integral value type (excluding character types).
    /// @param str - Destination buffer to write into.
    /// @param sz - Number of characters available at `str`.
    /// @param val - Value converted to text via `to_pseudo_string`/
    /// `to_pseudo_string_hex` according to `fmt`'s tag.
    /// @return Number of characters written, or a `runtime_error` when `sz` is too small.
    /// @details Only accepts a tag that `integral_tag_ok` allows: none, or hexadecimal.
    ///
    template <format_output fmt, typename CharT, FormatIntegral T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && integral_tag_ok<fmt>())
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, T&& val)
    {
        using FmtType = decltype(fmt);

        auto _arr = [&val]
        {
            if constexpr (fmt.tag == FmtType::none)
            {
                return to_pseudo_string<CharT>(val);
            }
            else
            {
                return to_pseudo_string_hex<CharT>(val);
            }
        }();

        usize _write_sz = (_arr.size() + fmt.format.size() + fmt.style.size() + fmt.reset.size() - 3);

        // The terminator written after the text takes one position more than its characters.
        if (_write_sz + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);
        str = _print_to_impl(str, _arr) + 1;
        str = _print_to_impl(str, fmt.reset);

        return _write_sz;
    }

    ///
    /// @brief Writes a format's literal text, style, and reset escapes around
    /// a floating-point value.
    ///
    /// @tparam fmt - Resolved `format_output` describing the literal to print.
    /// @tparam CharT - Character type of `str`; must match `fmt`'s character type.
    /// @tparam T - Floating-point value type.
    /// @param str - Destination buffer to write into.
    /// @param sz - Number of characters available at `str`.
    /// @param val - Value converted to text via `to_pseudo_string`,
    /// `to_pseudo_string_hex`, or `to_pseudo_string_exp` according to `fmt`'s
    /// tag; the precision tag selects the fractional digit count (default 6).
    /// @return Number of characters written, or a `runtime_error` when `sz` is too small.
    /// @details Only accepts a tag that `floating_tag_ok` allows: none, hexadecimal, or exponent.
    ///
    template <format_output fmt, typename CharT, FloatingPointType T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && floating_tag_ok<fmt>())
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, T&& val)
    {
        using FmtType = decltype(fmt);

        auto _arr = [&val]
        {
            constexpr usize _prec_val = ((fmt.tag & FmtType::prec) == FmtType::prec) ? fmt.prec_val : 6;
            constexpr auto _tag = fmt.tag & ~fmt.prec;

            if constexpr (_tag == FmtType::none)
            {
                return to_pseudo_string<CharT, _prec_val>(val);
            }
            else if constexpr ((_tag & FmtType::hex) == FmtType::hex)
            {
                return to_pseudo_string_hex<CharT>(val);
            }
            else
            {
                return to_pseudo_string_exp<CharT, _prec_val>(val);
            }
        }();

        usize _write_sz = (_arr.size() + fmt.format.size() + fmt.style.size() + fmt.reset.size() - 3);

        // The terminator written after the text takes one position more than its characters.
        if (_write_sz + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);
        str = _print_to_impl(str, _arr) + 1;
        str = _print_to_impl(str, fmt.reset);

        return _write_sz;
    }

    ///
    /// @brief Writes a format's literal text and style around a null-terminated
    /// character string, transcoding it to `CharT`.
    ///
    /// @tparam fmt - Resolved `format_output` describing the literal to print.
    /// @tparam CharT - Character type of `str`; must match `fmt`'s character type.
    /// @tparam T - Source character type of `val`.
    /// @param str - Destination buffer to write into.
    /// @param sz - Number of characters available at `str`.
    /// @param val - Null-terminated source string transcoded into `str`.
    /// @return Total characters written including the reset escape, or a
    /// `runtime_error` when `sz` is too small or transcoding fails.
    /// @details Only accepts a format with no presentation tag, since text has no numeric form.
    ///
    template <format_output fmt, typename CharT, CharacterType T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const T* val)
    {
        // The style escape before the value and the reset escape after it need room,
        // and the terminator after the value needs one position more than its characters.
        const usize _prefix = fmt.format.size() + fmt.style.size() - 2;
        const usize _suffix = fmt.reset.size() - 1;

        if (_prefix + _suffix + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);

        // Convert into the room left after the prefix, keeping room for the suffix.
        const usize _room = sz - _prefix - _suffix;
        auto _res = unicode::convert_to<CharT, T>(str, _room, val);

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        usize _pos = _room - _res.unwrap() - 1;
        str = _print_to_impl(str + _pos, fmt.reset);

        return _prefix + _pos + _suffix;
    }

    ///
    /// @brief Overload of `print_to` for a single character, forwarded as a
    /// null-terminated one-character string.
    ///
    /// @tparam fmt - Resolved `format_output` describing the literal to print.
    /// @tparam CharT - Character type of `str`; must match `fmt`'s character type.
    /// @tparam T - Source character type of `val`.
    /// @param str - Destination buffer to write into.
    /// @param sz - Number of characters available at `str`.
    /// @param val - Single character transcoded into `str`.
    /// @return Result forwarded from the character-string overload of `print_to`.
    ///
    template <format_output fmt, typename CharT, CharacterType T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, T&& val)
    {
        const T _arr[] = {val, '\0'}; 
        return print_to<fmt, CharT>(str, sz, _arr);
    }
}