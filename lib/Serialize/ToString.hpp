#pragma once

/// @file ToString.hpp
/// @brief Numeric and character conversion to project string values.
/// @details Converts integral, floating-point, hexadecimal, and character data
/// into bounded null-terminated representations used by formatting and logs.

#include "../Base/Math.hpp"
#include "_ToStringBase.hpp"
#include "_ToStringFloating.hpp"

namespace hsd
{
    /// @brief Implementation details backing the public `to_pseudo_string*` functions.
    namespace to_string_detail
    {
        /// @brief Writes the decimal digits of an integer, least significant first.
        /// @tparam CharT Character type pushed into `buf`.
        /// @param buf Destination buffer supporting `push(CharT)`; filled back-to-front.
        /// @param val Integral value to convert; a leading `-` is appended for negatives.
        template <typename CharT = char>
        static constexpr void to_pseudo_string_int_impl(auto& buf, IsIntegral auto val)
        {
            const bool _negative = val < 0;

            if (val == 0)
            {
               buf.push('0'); 
            }

            for (; val != 0; val /= 10)
            {
                const i32 _digit = static_cast<i32>((_negative) ? -(val % 10) : (val % 10));
                buf.push('0' + _digit);
            }

            if (_negative) buf.push('-');
        }

        /// @brief Writes the lowercase hexadecimal digits of an integer, least significant first.
        /// @tparam CharT Character type pushed into `buf`.
        /// @param buf Destination buffer supporting `push(CharT)`; filled back-to-front.
        /// @param val Integral value to convert; treated as unsigned (no sign digit emitted).
        template <typename CharT = char>
        static constexpr void to_pseudo_string_hex_int_impl(auto& buf, IsIntegral auto val)
        {
            add_unsigned_t<decltype(val)> _copy = val;
            constexpr const CharT _hex_table[16] = {
                '0', '1', '2', '3', '4', '5', '6', '7', 
                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
            };

            if (_copy == 0)
            {
               buf.push('0'); 
            }

            for (; _copy != 0; _copy >>= 4)
            {
                buf.push(_hex_table[static_cast<usize>(_copy & 15)]);
            }
        }
    } // namespace to_string_detail

    /// @brief Converts an integral value to its decimal string representation.
    /// @tparam CharT Character type of the returned buffer.
    /// @param val Integral value to convert.
    /// @return A fixed-capacity buffer holding the decimal digits (and leading
    /// `-` for negative values), sized for the widest supported integer type.
    template <typename CharT = char>
    static constexpr auto to_pseudo_string(IsIntegral auto val)
    {
        to_string_detail::to_string_value_rev<CharT, 41> _string = {};
        to_string_detail::to_pseudo_string_int_impl(_string, val);

        return _string;
    }

    /// @brief Converts an integral value to its `0x`-prefixed hexadecimal string representation.
    /// @tparam CharT Character type of the returned buffer.
    /// @param val Integral value to convert; treated as unsigned.
    /// @return A fixed-capacity buffer holding `"0x"` followed by the hex digits.
    template <typename CharT = char>
    static constexpr auto to_pseudo_string_hex(IsIntegral auto val)
    {
        to_string_detail::to_string_value_rev<CharT, 35> _string = {};
        to_string_detail::to_pseudo_string_hex_int_impl(_string, val);

        _string.push('x');
        _string.push('0');

        return _string;
    }

    /// @brief Converts a floating-point value to its fixed-point decimal string representation.
    /// @tparam CharT Character type of the returned buffer.
    /// @tparam DecPrecision Number of digits written after the decimal point.
    /// @param val Floating-point value to convert.
    /// @return A fixed-capacity buffer holding the sign (if negative), integral
    /// part, `.`, and `DecPrecision` fractional digits; or the literal `"nan"`
    /// / `"inf"` (with sign) for non-finite input.
    /// @details The value is rounded to nearest with ties to even, matching `%.*f`.
    /// The point is omitted when `DecPrecision` is zero.
    template <typename CharT = char, usize DecPrecision = 6>
    static constexpr auto to_pseudo_string(IsFloat auto val)
    {
        using FpType = decltype(val);
        const bool _neg = val < 0.;
        val = _neg ? -val : val;

        using max_digits = to_string_detail::fp_max_digits<FpType>;
        to_string_detail::to_string_value_for<CharT, max_digits::val + DecPrecision + 2> _str{};

        if (_neg == true)
        {
            _str.push(static_cast<CharT>('-'));
        }

        if (math::is_nan(val) == true)
        {
            _str.push('n');
            _str.push('a');
            _str.push('n');

            return _str;
        }
        else if (val == limits<FpType>::infinity)
        {
            _str.push('i');
            _str.push('n');
            _str.push('f');

            return _str;
        }

        to_string_detail::write_fixed_point(_str, val, DecPrecision);

        return _str;
    }

    /// @brief Converts a floating-point value to its `p`-exponent hexadecimal-float
    /// string representation (C99 `%a`-style, without the `+` on the exponent).
    /// @tparam CharT Character type of the returned buffer.
    /// @param val Floating-point value to convert.
    /// @return A fixed-capacity buffer holding the sign (if negative), `0x`, the
    /// leading digit (`1` for normal values, `0` otherwise), `.` and the fraction
    /// digits (omitted when the fraction is zero), `p`, and the binary exponent; or
    /// the literal `"nan"` / `"inf"` (with sign for infinity) for non-finite input.
    /// @details Zero has exponent `0`, and subnormal values use the smallest normal
    /// exponent, as `%a` does. Both cases come from `decompose_binary`, so the output
    /// is identical in constant evaluation and at run time.
    template <typename CharT = char>
    static constexpr auto to_pseudo_string_hex(IsFloat auto val)
    {
        using FpType = decltype(val);

        // Sign, "0x", leading digit, ".", fraction digits, "p", and an exponent of at
        // most six characters ("-16382" for x87 long double).
        constexpr usize _max_fraction_digits = (limits<FpType>::mantissa + 3) / 4;
        constexpr usize _max_length = 1 + 2 + 1 + 1 + _max_fraction_digits + 1 + 6;
        to_string_detail::to_string_value_rev<CharT, _max_length> _str = {};

        const bool _neg = val < 0.;
        val = _neg ? -val : val;

        if (math::is_nan(val) == true)
        {
            _str.push('n');
            _str.push('a');
            _str.push('n');

            return _str;
        }
        else if (val == limits<FpType>::infinity)
        {
            _str.push('f');
            _str.push('n');
            _str.push('i');

            if (_neg == true)
            {
                _str.push('-');
            }

            return _str;
        }

        constexpr i32 _fraction_bits = static_cast<i32>(limits<FpType>::mantissa);
        i32 _exponent = 0;
        bool _is_normal = false;
        u64 _fraction = 0;

        if (val != 0)
        {
            const to_string_detail::binary_parts _parts = to_string_detail::decompose_binary(val);

            // The leading bit is the implicit bit of a normal value and is absent otherwise.
            _is_normal = (_parts.mantissa >> _fraction_bits) != 0;
            _exponent = _is_normal
                ? _parts.exponent + _fraction_bits
                : limits<FpType>::min_exponent;
            _fraction = static_cast<u64>(_parts.mantissa & ((u128{1} << _fraction_bits) - 1));
        }

        // Pad the fraction to whole hex digits, then drop trailing zero digits.
        constexpr i32 _pad_bits = (4 - _fraction_bits % 4) % 4;
        usize _fraction_digit_count = static_cast<usize>(_fraction_bits + _pad_bits) / 4;
        _fraction <<= _pad_bits;

        while (_fraction != 0 && (_fraction & 15) == 0)
        {
            _fraction >>= 4;
            --_fraction_digit_count;
        }

        to_string_detail::to_pseudo_string_int_impl(_str, _exponent);
        _str.push('p');

        if (_fraction != 0)
        {
            const auto _before_size = _str.size();
            to_string_detail::to_pseudo_string_hex_int_impl(_str, _fraction);

            for (auto _diff = _str.size() - _before_size; _diff < _fraction_digit_count; ++_diff)
            {
                _str.push('0');
            }

            _str.push('.');
        }

        _str.push(_is_normal ? '1' : '0');
        _str.push('x');
        _str.push('0');

        if (_neg == true)
        {
            _str.push('-');
        }

        return _str;
    }

    /// @brief Converts a floating-point value to its scientific-notation string representation.
    /// @tparam CharT Character type of the returned buffer.
    /// @tparam DecPrecision Number of mantissa digits written after the decimal point.
    /// @param val Floating-point value to convert.
    /// @return A fixed-capacity buffer holding the sign (if negative), one
    /// leading mantissa digit, `.`, `DecPrecision` mantissa digits, and an
    /// `e`-prefixed signed exponent; or the literal `"nan"` / `"inf"` (with
    /// sign) for non-finite input.
    /// @details The mantissa is rounded to nearest with ties to even, matching `%.*e`.
    template <typename CharT = char, usize DecPrecision = 6>
    static constexpr auto to_pseudo_string_exp(IsFloat auto val)
    {
        const bool _neg_num = val < 0.;
        val = _neg_num ? -val : val;

        using FpType = decltype(val);
        to_string_detail::to_string_value_for<CharT, DecPrecision + 9> _str{};

        if (math::is_nan(val) == true)
        {
            _str.push('n');
            _str.push('a');
            _str.push('n');

            return _str;
        }
        else if (val == limits<FpType>::infinity)
        {
            _str.push('i');
            _str.push('n');
            _str.push('f');

            return _str;
        }

        if (_neg_num == true)
        {
            _str.push(static_cast<CharT>('-'));
        }

        to_string_detail::write_scientific(_str, val, DecPrecision);

        return _str;
    }
}