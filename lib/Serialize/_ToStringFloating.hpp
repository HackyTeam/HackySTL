#pragma once

/// @file _ToStringFloating.hpp
/// @brief Internal floating-point conversion support.
/// @details Contains compile-time and runtime helpers used to render floating
/// values without exposing conversion buffers as public API.

#include "../Base/Limits.hpp"
#include "../Base/Concepts.hpp"
#include "../Base/Utility.hpp"

namespace hsd::to_string_detail
{
    /// @brief Maximum decimal digits needed to round-trip a floating-point type `T`.
    /// @tparam T Floating-point type queried; only the specializations below are defined.
    template <typename>
    struct fp_max_digits;

    /// @brief Digit bound for `f128`.
    template <>
    struct fp_max_digits<f128>
    {
        /// @brief Maximum decimal digit count.
        static constexpr usize val = 4933;
    };

    /// @brief Digit bound for `f64`.
    template <>
    struct fp_max_digits<f64>
    {
        /// @brief Maximum decimal digit count.
        static constexpr usize val = 309;
    };

    /// @brief Digit bound for `f32`.
    template <>
    struct fp_max_digits<f32>
    {
        /// @brief Maximum decimal digit count.
        static constexpr usize val = 39;
    };

    /// @brief Base of each limb in the exact decimal expansion.
    static constexpr u64 decimal_limb_base = 1'000'000'000'000'000'000ull;

    /// @brief Decimal digits stored in one limb.
    static constexpr usize decimal_limb_digits = 18;

    /// @brief Largest precision whose power of five still fits in a `u64`.
    static constexpr usize max_fast_precision = 27;

    /// @brief Characters needed for the decimal digits of any `u128`.
    static constexpr usize max_u128_digits = 40;

    /// @brief Exact binary decomposition of a positive finite value.
    /// @details `val == mantissa * 2^exponent`. Normal values carry the implicit
    /// leading bit, so `mantissa` lies in `[2^mantissa_bits, 2^(mantissa_bits + 1))`
    /// where `mantissa_bits` is `limits<T>::mantissa`. Subnormal values have a
    /// smaller mantissa and the exponent `limits<T>::min_exponent - mantissa_bits`.
    struct binary_parts
    {
        /// @brief Integer significand holding every mantissa bit of the value.
        u128 mantissa;
        /// @brief Power of two applied to `mantissa`.
        i32 exponent;
    };

    /// @brief Scales a positive finite value by two until it is an integer, then
    /// normalizes the integer so its leading bit sits where IEEE values keep it.
    /// @tparam T Floating-point type of `val`.
    /// @param val Positive, finite value.
    /// @return `binary_parts` for `val`, following the contract of `binary_parts`.
    /// @details Every step is exact, and the code is constant-evaluable for any type.
    template <FloatingPointType T>
    static constexpr binary_parts decompose_by_scaling(T val)
    {
        constexpr T _significand_limit = [] {
            T _power_of_two = 1;

            for (usize _bit = 0; _bit <= limits<T>::mantissa; ++_bit)
            {
                _power_of_two *= 2;
            }

            return _power_of_two;
        }();

        i32 _exponent = 0;
        T _scaled = val;

        while (_scaled >= _significand_limit)   // exact: the spacing is at least 2 here
        {
            _scaled /= 2;
            ++_exponent;
        }

        while (_scaled != static_cast<T>(static_cast<u128>(_scaled)))   // exact: doubling
        {
            _scaled *= 2;
            --_exponent;
        }

        // Shift up until the leading bit reaches the implicit-bit position. Shifting stops
        // at the subnormal exponent, so smaller values keep a mantissa below that position.
        constexpr i32 _subnormal_exponent =
            limits<T>::min_exponent - static_cast<i32>(limits<T>::mantissa);
        u128 _mantissa = static_cast<u128>(_scaled);

        while (_mantissa < (u128{1} << limits<T>::mantissa) && _exponent > _subnormal_exponent)
        {
            _mantissa <<= 1;
            --_exponent;
        }

        return binary_parts{_mantissa, _exponent};
    }

    /// @brief Reads the significand and exponent of a positive finite x87 `long double`
    /// from its 80-bit layout.
    /// @param val Positive, finite x87 `long double`.
    /// @return `binary_parts` for `val`, following the contract of `binary_parts`.
    /// @details Run-time only. The layout is a 64-bit significand with an explicit
    /// integer bit, then a 15-bit exponent and a sign, so the value needs no scaling.
    template <typename T>
    static inline binary_parts decompose_x87_layout(T val)
    {
        struct x87_layout
        {
            u64 significand;
            u16 sign_and_exponent;
            u8 unused[6];
        };

        const x87_layout _layout = bit_cast<x87_layout>(val);
        constexpr i32 _fraction_bits = static_cast<i32>(limits<long double>::mantissa);
        constexpr i32 _subnormal_exponent = limits<long double>::min_exponent - _fraction_bits;
        const i32 _raw_exponent = static_cast<i32>(_layout.sign_and_exponent & 0x7FFF);

        if (_raw_exponent == 0)   // subnormal: the integer bit is clear
        {
            return binary_parts{_layout.significand, _subnormal_exponent};
        }

        return binary_parts{
            _layout.significand,
            _raw_exponent + limits<long double>::min_exponent - 1 - _fraction_bits
        };
    }

    /// @brief Splits a positive finite value into its exact significand and power of two.
    /// @tparam T Floating-point type of `val`.
    /// @param val Positive, finite value.
    /// @return `binary_parts` such that `val == mantissa * 2^exponent`.
    /// @details IEEE binary32 and binary64 values are read from their bit pattern,
    /// which `bit_cast` evaluates in constant expressions. x87 `long double` values are
    /// read from their layout at run time and scaled in constant evaluation. Other
    /// types are scaled.
    template <FloatingPointType T>
    static constexpr binary_parts decompose_binary(T val)
    {
        if constexpr (sizeof(T) <= 8)
        {
            using bits_type = conditional_t<sizeof(T) == 8, u64, u32>;
            constexpr i32 _fraction_bits = limits<T>::mantissa;
            constexpr i32 _min_normal_exponent = limits<T>::min_exponent;
            constexpr bits_type _exponent_mask =
                (bits_type{1} << (sizeof(T) * 8 - _fraction_bits - 1)) - 1;

            const bits_type _bits = bit_cast<bits_type>(val);
            const i32 _raw_exponent = static_cast<i32>((_bits >> _fraction_bits) & _exponent_mask);
            const u128 _fraction = _bits & ((bits_type{1} << _fraction_bits) - 1);

            if (_raw_exponent == 0)   // subnormal: no implicit leading bit
            {
                return binary_parts{_fraction, _min_normal_exponent - _fraction_bits};
            }

            return binary_parts{
                _fraction | (u128{1} << _fraction_bits),
                _raw_exponent + _min_normal_exponent - 1 - _fraction_bits
            };
        }
        else if constexpr (limits<T>::mantissa == 63)   // x87 extended precision
        {
            if (is_constant_evaluated())
            {
                return decompose_by_scaling(val);
            }

            return decompose_x87_layout(static_cast<long double>(val));
        }
        else
        {
            return decompose_by_scaling(val);
        }
    }

    /// @brief Upper bound on the decimal digits of any value of `T` after exact expansion.
    /// @tparam T Floating-point type.
    /// @return Digit capacity that covers both the integer and the fractional part.
    /// @details A fraction `mantissa * 2^-k` has at most `bits * log10(2) +
    /// k * log10(5) + 1` decimal digits. The logarithms are rounded up in integer
    /// arithmetic, so the bound is safe without floating-point evaluation.
    template <FloatingPointType T>
    static constexpr usize max_decimal_digits()
    {
        constexpr usize _significand_bits = limits<T>::mantissa + 1;
        constexpr usize _max_fraction_shift =
            limits<T>::mantissa + static_cast<usize>(-limits<T>::min_exponent);
        constexpr usize _integer_bits = static_cast<usize>(2 - limits<T>::min_exponent);

        constexpr usize _fraction_digits =
            (_significand_bits * 30103 + _max_fraction_shift * 69898) / 100000 + 2;
        constexpr usize _integer_digits = (_integer_bits * 30103) / 100000 + 2;

        return _fraction_digits > _integer_digits ? _fraction_digits : _integer_digits;
    }

    /// @brief Exact decimal expansion of a positive finite value, with no rounding.
    /// @tparam T Floating-point type of the expanded value.
    /// @details The value equals `digits[0..digit_count)` scaled by
    /// `10^-fraction_digits`. Leading zeros are removed, so `digits[0]` is the most
    /// significant nonzero digit.
    template <FloatingPointType T>
    struct exact_decimal
    {
        /// @brief Limbs that hold the largest expansion of `T`, with two spare.
        static constexpr usize limb_count = max_decimal_digits<T>() / decimal_limb_digits + 2;

        /// @brief Significant digits, most significant first.
        char digits[limb_count * decimal_limb_digits] = {};
        /// @brief Number of significant digits stored in `digits`.
        usize digit_count = 0;
        /// @brief Number of digits to the right of the decimal point.
        usize fraction_digits = 0;
    };

    /// @brief Multiplies a little-endian number of base-`decimal_limb_base` limbs by a word.
    /// @param limbs Limbs of the number, least significant first.
    /// @param limb_count Limbs in use; grows when the product needs another limb.
    /// @param factor Multiplier, at most `2^29` or `5^max_fast_precision`.
    static constexpr void multiply_decimal_limbs(u64* limbs, usize& limb_count, u64 factor)
    {
        u128 _carry = 0;

        for (usize _limb_idx = 0; _limb_idx < limb_count; ++_limb_idx)
        {
            const u128 _product = static_cast<u128>(limbs[_limb_idx]) * factor + _carry;
            limbs[_limb_idx] = static_cast<u64>(_product % decimal_limb_base);
            _carry = _product / decimal_limb_base;
        }

        while (_carry != 0)
        {
            limbs[limb_count++] = static_cast<u64>(_carry % decimal_limb_base);
            _carry /= decimal_limb_base;
        }
    }

    /// @brief Computes `5^exponent`.
    /// @param exponent Power to compute; must be at most `max_fast_precision`.
    /// @return The power, which fits in a `u64`.
    static constexpr u64 power_of_five(usize exponent)
    {
        u64 _result = 1;

        for (usize _factor_idx = 0; _factor_idx < exponent; ++_factor_idx)
        {
            _result *= 5;
        }

        return _result;
    }

    /// @brief Counts the significant bits of a `u128`.
    /// @param value Value to measure.
    /// @return Position of the highest set bit plus one; `0` for zero.
    static constexpr i32 bit_width_u128(u128 value)
    {
        const u64 _high_word = static_cast<u64>(value >> 64);

        if (_high_word != 0)
        {
            return 128 - countl_zero(_high_word);
        }

        return 64 - countl_zero(static_cast<u64>(value));
    }

    /// @brief Decides whether keeping the first `kept_count` digits rounds the value up.
    /// @param digits Significant digits, most significant first.
    /// @param digit_count Number of significant digits; digits past it are zeros.
    /// @param kept_count Number of leading digits kept after rounding.
    /// @return `true` when the discarded digits exceed one half, or equal one half
    /// with an odd last kept digit. Ties therefore go to the even neighbour.
    static constexpr bool rounds_up(const char* digits, usize digit_count, usize kept_count)
    {
        if (kept_count >= digit_count)
        {
            return false;
        }

        const char _first_discarded = digits[kept_count];

        if (_first_discarded != '5')
        {
            return _first_discarded > '5';
        }

        for (usize _digit_idx = kept_count + 1; _digit_idx < digit_count; ++_digit_idx)
        {
            if (digits[_digit_idx] != '0')
            {
                return true;
            }
        }

        // Exactly one half. A missing last kept digit counts as zero, which is even.
        return kept_count != 0 && (digits[kept_count - 1] - '0') % 2 == 1;
    }

    /// @brief Adds one unit in the last place to the first `kept_count` digits.
    /// @param digits Significant digits, most significant first; updated in place.
    /// @param kept_count Number of leading digits to increment.
    /// @return `true` when every kept digit was 9, so the sum carries out of them.
    static constexpr bool increment_digits(char* digits, usize kept_count)
    {
        for (usize _digit_idx = kept_count; _digit_idx-- > 0;)
        {
            if (digits[_digit_idx] != '9')
            {
                ++digits[_digit_idx];
                return false;
            }

            digits[_digit_idx] = '0';
        }

        return true;
    }

    /// @brief Expands a positive finite value into its exact decimal digits.
    /// @tparam T Floating-point type of `val`.
    /// @param val Positive, finite value.
    /// @return The exact expansion of `val`.
    /// @details `mantissa * 2^exponent` is an integer when `exponent >= 0`, so it is
    /// built by multiplying by powers of two. When `exponent < 0`, the value equals
    /// `(mantissa * 5^k) * 10^-k`. The digits of `mantissa * 5^k` are therefore the
    /// digits of the value, with the decimal point `k` places from the right.
    template <FloatingPointType T>
    static constexpr exact_decimal<T> make_exact_decimal(T val)
    {
        exact_decimal<T> _decimal{};
        u64 _limbs[exact_decimal<T>::limb_count] = {};
        usize _limb_count = 0;

        binary_parts _parts = decompose_binary(val);

        // Trailing zero bits only add limbs, so drop them before multiplying.
        while ((_parts.mantissa & 1) == 0)
        {
            _parts.mantissa >>= 1;
            ++_parts.exponent;
        }

        for (u128 _remaining = _parts.mantissa; _remaining != 0; _remaining /= decimal_limb_base)
        {
            _limbs[_limb_count++] = static_cast<u64>(_remaining % decimal_limb_base);
        }

        if (_parts.exponent >= 0)
        {
            for (i32 _twos_left = _parts.exponent; _twos_left > 0;)
            {
                const i32 _twos_chunk = _twos_left < 29 ? _twos_left : 29;
                multiply_decimal_limbs(_limbs, _limb_count, u64{1} << _twos_chunk);
                _twos_left -= _twos_chunk;
            }
        }
        else
        {
            _decimal.fraction_digits = static_cast<usize>(-_parts.exponent);
            usize _fives_left = _decimal.fraction_digits;

            while (_fives_left > 0)
            {
                const usize _fives_chunk =
                    _fives_left < max_fast_precision ? _fives_left : max_fast_precision;
                multiply_decimal_limbs(_limbs, _limb_count, power_of_five(_fives_chunk));
                _fives_left -= _fives_chunk;
            }
        }

        usize _total_digits = 0;

        for (usize _limb_idx = _limb_count; _limb_idx-- > 0;)
        {
            u64 _remaining_limb = _limbs[_limb_idx];

            for (usize _digit_offset = decimal_limb_digits; _digit_offset-- > 0;)
            {
                _decimal.digits[_total_digits + _digit_offset] =
                    static_cast<char>('0' + _remaining_limb % 10);
                _remaining_limb /= 10;
            }

            _total_digits += decimal_limb_digits;
        }

        usize _leading_zeros = 0;

        // The value is positive, so a nonzero digit exists and the loop stops.
        while (_decimal.digits[_leading_zeros] == '0')
        {
            ++_leading_zeros;
        }

        _decimal.digit_count = _total_digits - _leading_zeros;

        for (usize _digit_idx = 0; _digit_idx < _decimal.digit_count; ++_digit_idx)
        {
            _decimal.digits[_digit_idx] = _decimal.digits[_leading_zeros + _digit_idx];
        }

        return _decimal;
    }

    /// @brief Rounds `val * 10^precision` to an integer using exact 128-bit arithmetic.
    /// @tparam T Floating-point type of `val`.
    /// @param val Positive, finite value.
    /// @param precision Number of fractional digits to keep.
    /// @param rounded Receives the rounded integer when the function returns `true`.
    /// @return `false` when `precision` exceeds `max_fast_precision` or the scaled
    /// value needs more than 127 bits; the caller then uses `make_exact_decimal`.
    /// @details `val * 10^precision == mantissa * 5^precision * 2^(exponent + precision)`,
    /// so the rounded result is one shift of an exact product, with ties to even.
    template <FloatingPointType T>
    static constexpr bool try_round_scaled(T val, usize precision, u128& rounded)
    {
        if (precision > max_fast_precision)
        {
            return false;
        }

        const binary_parts _parts = decompose_binary(val);
        const u128 _power_of_five = power_of_five(precision);

        if (bit_width_u128(_parts.mantissa) + bit_width_u128(_power_of_five) > 127)
        {
            return false;
        }

        const u128 _scaled_mantissa = _parts.mantissa * _power_of_five;
        const i32 _scaled_bits = bit_width_u128(_scaled_mantissa);
        const i64 _shift = static_cast<i64>(_parts.exponent) + static_cast<i64>(precision);

        if (_shift >= 0)
        {
            if (_shift + _scaled_bits > 127)
            {
                return false;
            }

            rounded = _scaled_mantissa << _shift;
            return true;
        }

        const i64 _discarded_bits = -_shift;

        if (_discarded_bits > _scaled_bits)
        {
            rounded = 0;   // the scaled value is below one half
            return true;
        }

        rounded = _scaled_mantissa >> _discarded_bits;
        const u128 _discarded = _scaled_mantissa & ((u128{1} << _discarded_bits) - 1);
        const u128 _half = u128{1} << (_discarded_bits - 1);

        if (_discarded > _half || (_discarded == _half && (rounded & 1) == 1))
        {
            ++rounded;
        }

        return true;
    }

    /// @brief Writes the decimal digits of `value` at the end of `buffer`.
    /// @param value Integer to write.
    /// @param buffer Storage for `max_u128_digits` characters.
    /// @return Index of the most significant digit; the digits run to the end of `buffer`.
    static constexpr usize write_decimal_digits(u128 value, char (&buffer)[max_u128_digits])
    {
        usize _first_digit = max_u128_digits;

        if (value <= limits<u64>::max)
        {
            u64 _remaining = static_cast<u64>(value);

            do
            {
                buffer[--_first_digit] = static_cast<char>('0' + _remaining % 10);
                _remaining /= 10;
            } while (_remaining != 0);
        }
        else
        {
            do
            {
                buffer[--_first_digit] = static_cast<char>('0' + static_cast<u32>(value % 10));
                value /= 10;
            } while (value != 0);
        }

        return _first_digit;
    }

    /// @brief Writes a rounded integer as fixed-point text.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param digits Leading digits of the rounded integer, most significant first.
    /// @param digit_count Number of entries of `digits` in use.
    /// @param trailing_zeros Zeros that follow `digits` in the rounded integer.
    /// @param precision Fractional digits to write; `0` writes no decimal point.
    template <typename Buf>
    static constexpr void write_fixed_digits(
        Buf& buf, const char* digits, usize digit_count, usize trailing_zeros, usize precision)
    {
        const usize _length = digit_count + trailing_zeros;
        const usize _min_length = precision + 1;
        const usize _total_length = _length > _min_length ? _length : _min_length;
        const usize _leading_zeros = _total_length - _length;

        for (usize _position = 0; _position < _total_length; ++_position)
        {
            if (precision != 0 && _position == _total_length - precision)
            {
                buf.push('.');
            }

            if (_position < _leading_zeros)
            {
                buf.push('0');
            }
            else if (_position - _leading_zeros < digit_count)
            {
                buf.push(digits[_position - _leading_zeros]);
            }
            else
            {
                buf.push('0');
            }
        }
    }

    /// @brief Writes `e`, the sign of the exponent, and at least two exponent digits.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param exponent Decimal exponent to write.
    template <typename Buf>
    static constexpr void write_exponent(Buf& buf, i64 exponent)
    {
        buf.push('e');
        buf.push(exponent < 0 ? '-' : '+');

        u64 _magnitude = static_cast<u64>(exponent < 0 ? -exponent : exponent);
        char _reversed_digits[20] = {};
        usize _reversed_count = 0;

        do
        {
            _reversed_digits[_reversed_count++] = static_cast<char>('0' + _magnitude % 10);
            _magnitude /= 10;
        } while (_magnitude != 0);

        if (_reversed_count < 2)
        {
            _reversed_digits[_reversed_count++] = '0';
        }

        while (_reversed_count != 0)
        {
            buf.push(_reversed_digits[--_reversed_count]);
        }
    }

    /// @brief Writes a positive finite value in fixed-point form from its exact digits.
    /// @tparam T Floating-point type of `val`.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param val Positive, finite value.
    /// @param precision Fractional digits to write.
    template <FloatingPointType T, typename Buf>
    static constexpr void write_fixed_exact(Buf& buf, T val, usize precision)
    {
        exact_decimal<T> _decimal = make_exact_decimal(val);
        const i64 _kept_count_signed = static_cast<i64>(_decimal.digit_count)
            - static_cast<i64>(_decimal.fraction_digits) + static_cast<i64>(precision);

        if (_kept_count_signed < 0)
        {
            // The scaled value is below 0.1, so the rounded integer is zero.
            write_fixed_digits(buf, _decimal.digits, 0, 0, precision);
            return;
        }

        const usize _kept_count = static_cast<usize>(_kept_count_signed);

        if (rounds_up(_decimal.digits, _decimal.digit_count, _kept_count)
            && increment_digits(_decimal.digits, _kept_count))
        {
            // The carry left the leading digit, so the rounded integer is 10^_kept_count.
            _decimal.digits[0] = '1';
            write_fixed_digits(buf, _decimal.digits, 1, _kept_count, precision);
            return;
        }

        const usize _stored_count =
            _kept_count < _decimal.digit_count ? _kept_count : _decimal.digit_count;
        write_fixed_digits(
            buf, _decimal.digits, _stored_count, _kept_count - _stored_count, precision);
    }

    /// @brief Writes a positive finite value in scientific form from its exact digits.
    /// @tparam T Floating-point type of `val`.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param val Positive, finite value.
    /// @param precision Digits written after the decimal point of the mantissa.
    template <FloatingPointType T, typename Buf>
    static constexpr void write_scientific_exact(Buf& buf, T val, usize precision)
    {
        exact_decimal<T> _decimal = make_exact_decimal(val);
        i64 _exponent = static_cast<i64>(_decimal.digit_count)
            - static_cast<i64>(_decimal.fraction_digits) - 1;

        const usize _kept_count = precision + 1;
        usize _significant_count =
            _kept_count < _decimal.digit_count ? _kept_count : _decimal.digit_count;

        if (rounds_up(_decimal.digits, _decimal.digit_count, _kept_count)
            && increment_digits(_decimal.digits, _kept_count))
        {
            // Rounding carried out, so 9.99... becomes 1.00... one power of ten higher.
            _decimal.digits[0] = '1';
            _significant_count = 1;
            ++_exponent;
        }

        buf.push(_decimal.digits[0]);

        if (precision != 0)
        {
            buf.push('.');
        }

        for (usize _position = 1; _position < _kept_count; ++_position)
        {
            buf.push(_position < _significant_count ? _decimal.digits[_position] : '0');
        }

        write_exponent(buf, _exponent);
    }

    /// @brief Writes a non-negative finite value in fixed-point form.
    /// @tparam T Floating-point type of `val`.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param val Non-negative, finite value; the caller writes any sign.
    /// @param precision Fractional digits to write; `0` writes no decimal point.
    /// @details The result is `val` rounded to `precision` fractional digits, with
    /// ties to even. This matches the C library's `%.*f`.
    template <FloatingPointType T, typename Buf>
    static constexpr void write_fixed_point(Buf& buf, T val, usize precision)
    {
        if (val == 0)
        {
            write_fixed_digits(buf, "0", 1, 0, precision);
            return;
        }

        u128 _rounded = 0;

        if (try_round_scaled(val, precision, _rounded))
        {
            char _digits[max_u128_digits] = {};
            const usize _first_digit = write_decimal_digits(_rounded, _digits);
            write_fixed_digits(
                buf, _digits + _first_digit, max_u128_digits - _first_digit, 0, precision);
            return;
        }

        write_fixed_exact(buf, val, precision);
    }

    /// @brief Writes a non-negative finite value in scientific form.
    /// @tparam T Floating-point type of `val`.
    /// @tparam Buf Output buffer supporting `push(char)`.
    /// @param buf Destination buffer.
    /// @param val Non-negative, finite value; the caller writes any sign.
    /// @param precision Digits written after the decimal point of the mantissa.
    /// @details The mantissa is `val` rounded to `precision + 1` significant digits,
    /// with ties to even. This matches the C library's `%.*e`.
    template <FloatingPointType T, typename Buf>
    static constexpr void write_scientific(Buf& buf, T val, usize precision)
    {
        if (val == 0)
        {
            buf.push('0');

            if (precision != 0)
            {
                buf.push('.');
            }

            for (usize _position = 0; _position < precision; ++_position)
            {
                buf.push('0');
            }

            write_exponent(buf, 0);
            return;
        }

        write_scientific_exact(buf, val, precision);
    }
}  // namespace hsd::to_string_detail