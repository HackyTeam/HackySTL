#pragma once

/// @file _Int128.hpp
/// @brief Portable 128-bit integers for compilers without `__int128`.
/// @details Used as `u128` and `i128` on MSVC. Defining `HSD_LIBRARY_INT128` also
/// selects it on GCC and Clang, which is how the tests exercise it on those compilers.
/// A value is stored as two 64-bit halves. Arithmetic wraps modulo 2^128, and every
/// operation is `constexpr`. Division by a 64-bit divisor uses 64-bit arithmetic, and
/// division by a larger divisor runs bit by bit. The only platform header is
/// `<intrin.h>`, which MSVC x64 needs for its hardware divide.

#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif

namespace hsd
{
    namespace int128_detail
    {
        /// @brief Predicate: detects the built-in integer types that convert implicitly.
        template <typename T>
        struct is_integer_type
        {
            /// @brief False for types that are not built-in integers.
            static constexpr bool value = false;
        };

        /// @brief Built-in integer specializations of `is_integer_type`.
        template <> struct is_integer_type<bool> { static constexpr bool value = true; };
        template <> struct is_integer_type<char> { static constexpr bool value = true; };
        template <> struct is_integer_type<signed char> { static constexpr bool value = true; };
        template <> struct is_integer_type<unsigned char> { static constexpr bool value = true; };
        template <> struct is_integer_type<short> { static constexpr bool value = true; };
        template <> struct is_integer_type<unsigned short> { static constexpr bool value = true; };
        template <> struct is_integer_type<int> { static constexpr bool value = true; };
        template <> struct is_integer_type<unsigned int> { static constexpr bool value = true; };
        template <> struct is_integer_type<long> { static constexpr bool value = true; };
        template <> struct is_integer_type<unsigned long> { static constexpr bool value = true; };
        template <> struct is_integer_type<long long> { static constexpr bool value = true; };
        template <> struct is_integer_type<unsigned long long> { static constexpr bool value = true; };
        template <> struct is_integer_type<wchar_t> { static constexpr bool value = true; };
        template <> struct is_integer_type<char8_t> { static constexpr bool value = true; };
        template <> struct is_integer_type<char16_t> { static constexpr bool value = true; };
        template <> struct is_integer_type<char32_t> { static constexpr bool value = true; };

        /// @brief Matches the built-in integer types.
        template <typename T>
        concept IntegerLike = is_integer_type<T>::value;

        /// @brief Predicate: detects the built-in floating-point types.
        template <typename T>
        struct is_float_type
        {
            /// @brief False for types that are not built-in floating-point types.
            static constexpr bool value = false;
        };

        /// @brief Built-in floating-point specializations of `is_float_type`.
        template <> struct is_float_type<float> { static constexpr bool value = true; };
        template <> struct is_float_type<double> { static constexpr bool value = true; };
        template <> struct is_float_type<long double> { static constexpr bool value = true; };

        /// @brief Matches the built-in floating-point types.
        template <typename T>
        concept FloatLike = is_float_type<T>::value;

        /// @brief A 128-bit integer stored as two 64-bit halves.
        /// @tparam Signed True for two's-complement signed values, false for unsigned.
        /// @details Operators are hidden friends, so a built-in integer on either side
        /// converts implicitly. Conversions to other integer types are explicit.
        /// Dividing by zero is undefined, as it is for built-in integers.
        template <bool Signed>
        struct basic_int128
        {
            /// @brief Low 64 bits of the value.
            unsigned long long _lo = 0;
            /// @brief High 64 bits of the value.
            unsigned long long _hi = 0;

            /// @brief Constructs zero.
            constexpr basic_int128() = default;

            /// @brief Constructs from the low and high 64-bit halves.
            /// @param low Low 64 bits.
            /// @param high High 64 bits.
            constexpr basic_int128(unsigned long long low, unsigned long long high)
                : _lo{low}, _hi{high}
            {}

            /// @brief Converts a built-in integer, sign-extending signed values.
            /// @tparam T Built-in integer type.
            /// @param value Value to convert.
            template <IntegerLike T>
            constexpr basic_int128(T value)
                : _lo{static_cast<unsigned long long>(value)}
            {
                if constexpr (static_cast<T>(-1) < static_cast<T>(0))
                {
                    if (value < static_cast<T>(0))
                    {
                        _hi = ~0ull;
                    }
                }
            }

            /// @brief Converts a floating-point value, truncating toward zero.
            /// @tparam T Built-in floating-point type.
            /// @param value Value to convert; must be finite and below 2^128 in magnitude.
            template <FloatLike T>
            constexpr explicit basic_int128(T value)
            {
                constexpr T _two_64 = static_cast<T>(18446744073709551616.0);
                const bool _negative = value < static_cast<T>(0);
                const T _magnitude = _negative ? -value : value;

                _hi = static_cast<unsigned long long>(_magnitude / _two_64);
                _lo = static_cast<unsigned long long>(
                    _magnitude - static_cast<T>(_hi) * _two_64);

                if (_negative)
                {
                    *this = basic_int128{} - *this;
                }
            }

            /// @brief Converts between signed and unsigned 128-bit values by bit pattern.
            /// @tparam Other Signedness of the source value.
            /// @param other Value to convert.
            template <bool Other>
            constexpr explicit basic_int128(basic_int128<Other> other)
                : _lo{other._lo}, _hi{other._hi}
            {}

            /// @brief Converts to a built-in integer by truncation, or to a built-in
            /// floating-point type.
            /// @tparam T Built-in integer or floating-point type.
            template <typename T>
            requires (IntegerLike<T> || FloatLike<T>)
            constexpr explicit operator T() const
            {
                if constexpr (IntegerLike<T>)
                {
                    return static_cast<T>(_lo);
                }
                else
                {
                    constexpr T _two_64 = static_cast<T>(18446744073709551616.0);

                    if constexpr (Signed)
                    {
                        if ((_hi >> 63) != 0)
                        {
                            return -static_cast<T>(basic_int128{} - *this);
                        }
                    }

                    return static_cast<T>(_hi) * _two_64 + static_cast<T>(_lo);
                }
            }

            /// @brief Builds `high * 2^64 + low` from a full 128-bit product of two 64-bit values.
            /// @param a First factor.
            /// @param b Second factor.
            /// @return The exact 128-bit product.
            static constexpr basic_int128 mul_full(unsigned long long a, unsigned long long b)
            {
                constexpr unsigned long long _mask32 = 0xFFFFFFFFull;
                const unsigned long long _a0 = a & _mask32;
                const unsigned long long _a1 = a >> 32;
                const unsigned long long _b0 = b & _mask32;
                const unsigned long long _b1 = b >> 32;
                const unsigned long long _p00 = _a0 * _b0;
                const unsigned long long _p01 = _a0 * _b1;
                const unsigned long long _p10 = _a1 * _b0;
                const unsigned long long _p11 = _a1 * _b1;
                const unsigned long long _mid = (_p00 >> 32) + (_p01 & _mask32) + (_p10 & _mask32);

                return basic_int128{
                    (_mid << 32) | (_p00 & _mask32),
                    _p11 + (_p01 >> 32) + (_p10 >> 32) + (_mid >> 32)
                };
            }

            /// @brief Shifts left, giving zero when the count is 128 or more.
            /// @param value Value to shift.
            /// @param count Number of bit positions.
            /// @return `value` shifted left by `count`.
            static constexpr basic_int128 shift_left(basic_int128 value, unsigned long long count)
            {
                if (count == 0)
                {
                    return value;
                }

                if (count >= 128)
                {
                    return basic_int128{};
                }

                if (count >= 64)
                {
                    return basic_int128{0, value._lo << (count - 64)};
                }

                return basic_int128{
                    value._lo << count,
                    (value._hi << count) | (value._lo >> (64 - count))
                };
            }

            /// @brief Shifts right, filling with zeros.
            /// @param value Value to shift.
            /// @param count Number of bit positions.
            /// @return `value` shifted right by `count`, treating it as unsigned.
            static constexpr basic_int128 shift_right_logical(basic_int128 value, unsigned long long count)
            {
                if (count == 0)
                {
                    return value;
                }

                if (count >= 128)
                {
                    return basic_int128{};
                }

                if (count >= 64)
                {
                    return basic_int128{value._hi >> (count - 64), 0};
                }

                return basic_int128{
                    (value._lo >> count) | (value._hi << (64 - count)),
                    value._hi >> count
                };
            }

            /// @brief Shifts right, filling with copies of the sign bit.
            /// @param value Value to shift.
            /// @param count Number of bit positions.
            /// @return `value` shifted right by `count`, keeping the sign.
            static constexpr basic_int128 shift_right_arithmetic(basic_int128 value, unsigned long long count)
            {
                const basic_int128 _logical = shift_right_logical(value, count);

                if (count == 0 || (value._hi >> 63) == 0)
                {
                    return _logical;
                }

                if (count >= 128)
                {
                    return basic_int128{~0ull, ~0ull};
                }

                return _logical | shift_left(basic_int128{~0ull, ~0ull}, 128 - count);
            }

            /// @brief Compares two values as unsigned bit patterns.
            /// @return True when `a` is less than `b`.
            static constexpr bool less_unsigned(basic_int128 a, basic_int128 b)
            {
                if (a._hi != b._hi)
                {
                    return a._hi < b._hi;
                }

                return a._lo < b._lo;
            }

            /// @brief Compares two values using this type's signedness.
            /// @return True when `a` is less than `b`.
            static constexpr bool less(basic_int128 a, basic_int128 b)
            {
                if constexpr (Signed)
                {
                    if (a._hi != b._hi)
                    {
                        return static_cast<long long>(a._hi) < static_cast<long long>(b._hi);
                    }

                    return a._lo < b._lo;
                }
                else
                {
                    return less_unsigned(a, b);
                }
            }

            /// @brief Counts the leading zero bits of a 64-bit value.
            /// @param value Value to inspect; zero gives 64.
            /// @return Number of zero bits above the highest set bit.
            static constexpr unsigned int leading_zeros(unsigned long long value)
            {
                if (value == 0)
                {
                    return 64;
                }

                unsigned int _count = 0;

                if ((value >> 32) == 0) { _count += 32; value <<= 32; }
                if ((value >> 48) == 0) { _count += 16; value <<= 16; }
                if ((value >> 56) == 0) { _count += 8; value <<= 8; }
                if ((value >> 60) == 0) { _count += 4; value <<= 4; }
                if ((value >> 62) == 0) { _count += 2; value <<= 2; }
                if ((value >> 63) == 0) { _count += 1; }

                return _count;
            }

            /// @brief Divides the 128-bit value `u1:u0` by `v`, where `u1 < v`, using 32-bit digits.
            /// @details Hacker's Delight `divlu`: the quotient fits in 64 bits, and every
            /// intermediate product fits in 64 bits.
            /// @param u1 High 64 bits of the dividend.
            /// @param u0 Low 64 bits of the dividend.
            /// @param v Non-zero divisor.
            /// @param remainder Receives the remainder.
            /// @return The 64-bit quotient.
            static constexpr unsigned long long divide_2_by_1(
                unsigned long long u1, unsigned long long u0,
                unsigned long long v, unsigned long long& remainder)
            {
    #if defined(_MSC_VER) && defined(_M_X64)
                // The hardware divide is much faster than the portable code below. It
                // cannot run in constant evaluation, so that case uses the portable code.
                if (!__builtin_is_constant_evaluated())
                {
                    return _udiv128(u1, u0, v, &remainder);
                }
    #endif

                constexpr unsigned long long _base = 1ull << 32;
                constexpr unsigned long long _mask = 0xFFFFFFFFull;
                const unsigned int _s = leading_zeros(v);
                const unsigned long long _vn = v << _s;
                const unsigned long long _vn1 = _vn >> 32;
                const unsigned long long _vn0 = _vn & _mask;
                const unsigned long long _un32 = _s == 0 ? u1 : ((u1 << _s) | (u0 >> (64 - _s)));
                const unsigned long long _un10 = u0 << _s;
                const unsigned long long _un1 = _un10 >> 32;
                const unsigned long long _un0 = _un10 & _mask;

                unsigned long long _q1 = _un32 / _vn1;
                unsigned long long _rhat = _un32 - _q1 * _vn1;

                while (_q1 >= _base || _q1 * _vn0 > _base * _rhat + _un1)
                {
                    --_q1;
                    _rhat += _vn1;

                    if (_rhat >= _base)
                    {
                        break;
                    }
                }

                const unsigned long long _un21 = _un32 * _base + _un1 - _q1 * _vn;

                unsigned long long _q0 = _un21 / _vn1;
                _rhat = _un21 - _q0 * _vn1;

                while (_q0 >= _base || _q0 * _vn0 > _base * _rhat + _un0)
                {
                    --_q0;
                    _rhat += _vn1;

                    if (_rhat >= _base)
                    {
                        break;
                    }
                }

                remainder = (_un21 * _base + _un0 - _q0 * _vn) >> _s;
                return _q1 * _base + _q0;
            }

            /// @brief Divides by a 64-bit divisor, which is the common case for formatting.
            /// @param numerator Dividend, read as unsigned.
            /// @param divisor Non-zero 64-bit divisor.
            /// @param quotient Receives the quotient.
            /// @param remainder Receives the remainder.
            static constexpr void divide_by_word(
                basic_int128 numerator, unsigned long long divisor,
                basic_int128& quotient, basic_int128& remainder)
            {
                unsigned long long _rem = 0;

                if (numerator._hi < divisor)
                {
                    const unsigned long long _q = divide_2_by_1(numerator._hi, numerator._lo, divisor, _rem);
                    quotient = basic_int128{_q, 0};
                }
                else
                {
                    const unsigned long long _q_hi = numerator._hi / divisor;
                    const unsigned long long _mid = numerator._hi % divisor;
                    const unsigned long long _q_lo = divide_2_by_1(_mid, numerator._lo, divisor, _rem);
                    quotient = basic_int128{_q_lo, _q_hi};
                }

                remainder = basic_int128{_rem, 0};
            }

            /// @brief Divides two non-negative bit patterns, bit by bit.
            /// @param numerator Dividend, read as unsigned.
            /// @param denominator Divisor, read as unsigned.
            /// @param quotient Receives the quotient.
            /// @param remainder Receives the remainder.
            static constexpr void divide_unsigned(
                basic_int128 numerator, basic_int128 denominator,
                basic_int128& quotient, basic_int128& remainder)
            {
                if (denominator._hi == 0)
                {
                    divide_by_word(numerator, denominator._lo, quotient, remainder);
                    return;
                }

                quotient = basic_int128{};
                remainder = basic_int128{};

                for (int _bit = 127; _bit >= 0; --_bit)
                {
                    const bool _overflow = (remainder._hi >> 63) != 0;
                    const unsigned long long _incoming = _bit >= 64
                        ? (numerator._hi >> (_bit - 64)) & 1ull
                        : (numerator._lo >> _bit) & 1ull;

                    remainder = shift_left(remainder, 1);
                    remainder._lo |= _incoming;

                    if (_overflow || !less_unsigned(remainder, denominator))
                    {
                        remainder = remainder - denominator;

                        if (_bit >= 64)
                        {
                            quotient._hi |= 1ull << (_bit - 64);
                        }
                        else
                        {
                            quotient._lo |= 1ull << _bit;
                        }
                    }
                }
            }

            /// @brief Divides with truncation toward zero, as built-in integers do.
            /// @param lhs Dividend.
            /// @param rhs Divisor.
            /// @param quotient Receives the quotient.
            /// @param remainder Receives the remainder, with the sign of `lhs`.
            static constexpr void divide(
                basic_int128 lhs, basic_int128 rhs,
                basic_int128& quotient, basic_int128& remainder)
            {
                bool _lhs_negative = false;
                bool _rhs_negative = false;

                if constexpr (Signed)
                {
                    _lhs_negative = (lhs._hi >> 63) != 0;
                    _rhs_negative = (rhs._hi >> 63) != 0;

                    if (_lhs_negative)
                    {
                        lhs = basic_int128{} - lhs;
                    }

                    if (_rhs_negative)
                    {
                        rhs = basic_int128{} - rhs;
                    }
                }

                divide_unsigned(lhs, rhs, quotient, remainder);

                if constexpr (Signed)
                {
                    if (_lhs_negative != _rhs_negative)
                    {
                        quotient = basic_int128{} - quotient;
                    }

                    if (_lhs_negative)
                    {
                        remainder = basic_int128{} - remainder;
                    }
                }
            }

            /// @brief Adds with wrap-around modulo 2^128.
            friend constexpr basic_int128 operator+(basic_int128 lhs, basic_int128 rhs)
            {
                const unsigned long long _lo = lhs._lo + rhs._lo;
                const unsigned long long _carry = _lo < lhs._lo ? 1ull : 0ull;
                return basic_int128{_lo, lhs._hi + rhs._hi + _carry};
            }

            /// @brief Subtracts with wrap-around modulo 2^128.
            friend constexpr basic_int128 operator-(basic_int128 lhs, basic_int128 rhs)
            {
                const unsigned long long _lo = lhs._lo - rhs._lo;
                const unsigned long long _borrow = lhs._lo < rhs._lo ? 1ull : 0ull;
                return basic_int128{_lo, lhs._hi - rhs._hi - _borrow};
            }

            /// @brief Two's-complement negation.
            friend constexpr basic_int128 operator-(basic_int128 value)
            {
                return basic_int128{} - value;
            }

            /// @brief Unary plus, which returns the value unchanged.
            friend constexpr basic_int128 operator+(basic_int128 value)
            {
                return value;
            }

            /// @brief Multiplies with wrap-around modulo 2^128.
            friend constexpr basic_int128 operator*(basic_int128 lhs, basic_int128 rhs)
            {
                basic_int128 _product = mul_full(lhs._lo, rhs._lo);
                _product._hi += lhs._lo * rhs._hi + lhs._hi * rhs._lo;
                return _product;
            }

            /// @brief Divides with truncation toward zero.
            friend constexpr basic_int128 operator/(basic_int128 lhs, basic_int128 rhs)
            {
                basic_int128 _quotient;
                basic_int128 _remainder;
                divide(lhs, rhs, _quotient, _remainder);
                return _quotient;
            }

            /// @brief Remainder of truncating division, with the sign of the dividend.
            friend constexpr basic_int128 operator%(basic_int128 lhs, basic_int128 rhs)
            {
                basic_int128 _quotient;
                basic_int128 _remainder;
                divide(lhs, rhs, _quotient, _remainder);
                return _remainder;
            }

            /// @brief Bitwise AND.
            friend constexpr basic_int128 operator&(basic_int128 lhs, basic_int128 rhs)
            {
                return basic_int128{lhs._lo & rhs._lo, lhs._hi & rhs._hi};
            }

            /// @brief Bitwise OR.
            friend constexpr basic_int128 operator|(basic_int128 lhs, basic_int128 rhs)
            {
                return basic_int128{lhs._lo | rhs._lo, lhs._hi | rhs._hi};
            }

            /// @brief Bitwise XOR.
            friend constexpr basic_int128 operator^(basic_int128 lhs, basic_int128 rhs)
            {
                return basic_int128{lhs._lo ^ rhs._lo, lhs._hi ^ rhs._hi};
            }

            /// @brief Bitwise complement.
            friend constexpr basic_int128 operator~(basic_int128 value)
            {
                return basic_int128{~value._lo, ~value._hi};
            }

            /// @brief Left shift; a count of 128 or more gives zero.
            friend constexpr basic_int128 operator<<(basic_int128 value, basic_int128 count)
            {
                const unsigned long long _n = count._hi != 0 ? 128ull : count._lo;
                return shift_left(value, _n);
            }

            /// @brief Right shift; arithmetic for signed values, logical for unsigned.
            friend constexpr basic_int128 operator>>(basic_int128 value, basic_int128 count)
            {
                const unsigned long long _n = count._hi != 0 ? 128ull : count._lo;

                if constexpr (Signed)
                {
                    return shift_right_arithmetic(value, _n);
                }
                else
                {
                    return shift_right_logical(value, _n);
                }
            }

            /// @brief Equality of bit patterns.
            friend constexpr bool operator==(basic_int128 lhs, basic_int128 rhs)
            {
                return lhs._lo == rhs._lo && lhs._hi == rhs._hi;
            }

            /// @brief Inequality of bit patterns.
            friend constexpr bool operator!=(basic_int128 lhs, basic_int128 rhs)
            {
                return !(lhs == rhs);
            }

            /// @brief Less-than, using this type's signedness.
            friend constexpr bool operator<(basic_int128 lhs, basic_int128 rhs)
            {
                return less(lhs, rhs);
            }

            /// @brief Greater-than, using this type's signedness.
            friend constexpr bool operator>(basic_int128 lhs, basic_int128 rhs)
            {
                return less(rhs, lhs);
            }

            /// @brief Less-or-equal, using this type's signedness.
            friend constexpr bool operator<=(basic_int128 lhs, basic_int128 rhs)
            {
                return !less(rhs, lhs);
            }

            /// @brief Greater-or-equal, using this type's signedness.
            friend constexpr bool operator>=(basic_int128 lhs, basic_int128 rhs)
            {
                return !less(lhs, rhs);
            }

            /// @brief Adds `rhs` in place.
            constexpr basic_int128& operator+=(basic_int128 rhs)
            {
                *this = *this + rhs;
                return *this;
            }

            /// @brief Subtracts `rhs` in place.
            constexpr basic_int128& operator-=(basic_int128 rhs)
            {
                *this = *this - rhs;
                return *this;
            }

            /// @brief Multiplies by `rhs` in place.
            constexpr basic_int128& operator*=(basic_int128 rhs)
            {
                *this = *this * rhs;
                return *this;
            }

            /// @brief Divides by `rhs` in place.
            constexpr basic_int128& operator/=(basic_int128 rhs)
            {
                *this = *this / rhs;
                return *this;
            }

            /// @brief Takes the remainder by `rhs` in place.
            constexpr basic_int128& operator%=(basic_int128 rhs)
            {
                *this = *this % rhs;
                return *this;
            }

            /// @brief Bitwise AND in place.
            constexpr basic_int128& operator&=(basic_int128 rhs)
            {
                *this = *this & rhs;
                return *this;
            }

            /// @brief Bitwise OR in place.
            constexpr basic_int128& operator|=(basic_int128 rhs)
            {
                *this = *this | rhs;
                return *this;
            }

            /// @brief Bitwise XOR in place.
            constexpr basic_int128& operator^=(basic_int128 rhs)
            {
                *this = *this ^ rhs;
                return *this;
            }

            /// @brief Left shift in place.
            constexpr basic_int128& operator<<=(basic_int128 count)
            {
                *this = *this << count;
                return *this;
            }

            /// @brief Right shift in place.
            constexpr basic_int128& operator>>=(basic_int128 count)
            {
                *this = *this >> count;
                return *this;
            }

            /// @brief Increments by one, returning the new value.
            constexpr basic_int128& operator++()
            {
                *this = *this + basic_int128{1u};
                return *this;
            }

            /// @brief Increments by one, returning the old value.
            constexpr basic_int128 operator++(int)
            {
                basic_int128 _old = *this;
                ++*this;
                return _old;
            }

            /// @brief Decrements by one, returning the new value.
            constexpr basic_int128& operator--()
            {
                *this = *this - basic_int128{1u};
                return *this;
            }

            /// @brief Decrements by one, returning the old value.
            constexpr basic_int128 operator--(int)
            {
                basic_int128 _old = *this;
                --*this;
                return _old;
            }
        };
    } // namespace int128_detail
} // namespace hsd
