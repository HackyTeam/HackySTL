#pragma once

/// @file Limits.hpp
/// @brief Numeric boundary constants for HackySTL scalar types.
/// @details Exposes project-compatible minimum, maximum, infinity, and denormal
/// values used by numeric containers, math, formatting, and random generation.

#include "Utility.hpp"

// MSVC is the dumbest thing in existence
#ifdef HSD_COMPILER_MSVC
#undef min
#undef max
#endif

namespace hsd
{
    /// @brief Numeric boundary and floating-point metadata for a scalar type.
    /// @tparam T Scalar type described by the specialization.
    template <typename>
    struct limits;

    namespace detail
    {
        template <typename T>
        /// @brief Tests whether `T` represents signed values.
        /// @return True when converting `-1` to `T` remains negative.
        constexpr bool is_signed_byte()
        {
            return static_cast<T>(-1) < 0;
        }

        template <typename T, usize B>
        /// @brief Returns value bits excluding a sign bit when present.
        /// @return Number of value bits in a `B`-bit representation.
        constexpr usize digits_byte()
        {
            return B - is_signed_byte<T>();
        }

        template <typename T, usize B>
        /// @brief Computes the largest representable value for a bit width.
        /// @return Largest value representable by `T` in `B` bits.
        constexpr T max_byte()
        {
            constexpr auto _lshf = static_cast<T>(1) << (digits_byte<T, B>() - 1);

            if constexpr (is_signed_byte<T>())
            {
                return ((_lshf - 1) << 1) + 1;
            }

            return ~static_cast<T>(0);
        }

        template <typename T, usize B>
        /// @brief Computes the smallest representable value for a bit width.
        /// @return Smallest value representable by `T` in `B` bits.
        constexpr T min_byte()
        {
            if constexpr (is_signed_byte<T>())
            {
                return -max_byte<T, B>() - 1;
            }

            return 0;
        }

        template <typename T, usize B>
        /// @brief Estimates decimal digits from binary value bits.
        /// @return Estimated number of decimal digits.
        constexpr usize digits10_byte()
        {
            return digits_byte<T, B>() * 643L / 2136;
        }

        template <typename T>
        /// @brief Returns whether `T` is signed.
        /// @return True when `T` can represent negative values.
        constexpr bool is_signed()
        {
            return is_signed_byte<T>();
        }

        template <typename T>
        /// @brief Returns the minimum value for `T`.
        /// @return Lowest representable value.
        constexpr auto min()
        {
            return min_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        /// @brief Returns the maximum value for `T`.
        /// @return Highest representable value.
        constexpr auto max()
        {
            return max_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        /// @brief Returns the number of value bits for `T`.
        /// @return Number of value bits in `T`.
        constexpr auto digits()
        {
            return digits_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        /// @brief Returns the estimated base-ten digit count for `T`.
        /// @return Estimated decimal digit count.
        constexpr auto digits10()
        {
            return digits10_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        /// @brief Estimates decimal digits needed to represent `value`.
        /// @param value Value whose magnitude determines the estimate.
        /// @return Estimated decimal digit count for the value.
        constexpr T max_digits10(T value)
        {
            return 2 + value * 643L / 2136;
        }

        template <typename T>
        /// @brief Computes the exponent bit count used by a floating type.
        /// @return Number of exponent bits inferred from `limits<T>`.
        static constexpr auto _exp_bits()
        {
            hsd::usize _sz = 0;
            auto _max_exp = (limits<T>::max_exponent + 1) * 2;

            while ((_max_exp & 1) == 0)
            {
                _max_exp >>= 1;
                ++_sz;
            }

            return _sz;
        }

        /// @brief Layout of an x87 extended-precision value: 64-bit significand and
        /// a 16-bit sign and exponent, padded to 16 bytes.
        struct _x87_bits
        {
            u64 significand;
            u16 sign_and_exponent;
            u8 unused[6];
        };

        template <typename T>
        /// @brief Builds positive infinity from its bit pattern.
        /// @return Infinity for `T`, which is `float`, `double`, or x87 `long double`.
        static constexpr T _infinity()
        {
            if constexpr (sizeof(T) == 4)
            {
                return hsd::bit_cast<T>(u32{0x7F800000u});
            }
            else if constexpr (sizeof(T) == 8)
            {
                return hsd::bit_cast<T>(u64{0x7FF0000000000000ull});
            }
            else
            {
                return hsd::bit_cast<T>(_x87_bits{0x8000000000000000ull, 0x7FFF, {}});
            }
        }

        template <typename T>
        /// @brief Builds a quiet NaN from its bit pattern.
        /// @return Quiet NaN for `T`, which is `float`, `double`, or x87 `long double`.
        static constexpr T _quiet_nan()
        {
            if constexpr (sizeof(T) == 4)
            {
                return hsd::bit_cast<T>(u32{0x7FC00000u});
            }
            else if constexpr (sizeof(T) == 8)
            {
                return hsd::bit_cast<T>(u64{0x7FF8000000000000ull});
            }
            else
            {
                return hsd::bit_cast<T>(_x87_bits{0xC000000000000000ull, 0x7FFF, {}});
            }
        }
        /// @brief Builds an extended-precision value from its x87 layout, or from
        /// `as_double` when `T` is plain `double` (MSVC's `long double`).
        /// @details Literals such as `0x1p-16382L` overflow a `double` literal on MSVC,
        /// so the x87 bit pattern is used instead.
        template <typename T>
        static constexpr T _extended(f64 as_double, [[maybe_unused]] _x87_bits as_x87)
        {
            if constexpr (sizeof(T) == 8)
            {
                return static_cast<T>(as_double);
            }
            else
            {
                return hsd::bit_cast<T>(as_x87);
            }
        }
    } // namespace detail


    /// @brief Default numeric limits for integral-like scalar types.
    /// @details Floating-point types use the explicit specializations below;
    /// other types receive integer boundaries and zero floating metadata.
    template <typename T>
    struct limits
    {
        /// @brief False because this is the generic fallback.
        static constexpr bool  is_specialized        = false;
        /// @brief No infinity representation in the fallback.
        static constexpr T     infinity              = 0;
        /// @brief No floating epsilon in the fallback.
        static constexpr T     epsilon               = 0;
        /// @brief No NaN representation in the fallback.
        static constexpr T     nan                   = 0;
        /// @brief Lowest representable value.
        static constexpr T     min                   = detail::min<T>();
        /// @brief Highest representable value.
        static constexpr T     max                   = detail::max<T>();
        /// @brief No denormal representation in the fallback.
        static constexpr T     denorm_min            = 0;
        /// @brief Whether the type can represent negative values.
        static constexpr bool  is_signed             = detail::is_signed<T>();
        /// @brief Binary exponent minimum, zero for the fallback.
        static constexpr i32   min_exponent          = 0;
        /// @brief Decimal exponent minimum, zero for the fallback.
        static constexpr i32   min_exponent10        = 0;
        /// @brief Denormal decimal exponent minimum, zero for the fallback.
        static constexpr i32   min_exponent10_denorm = 0;
        /// @brief Binary exponent maximum, zero for the fallback.
        static constexpr i32   max_exponent          = 0;
        /// @brief Decimal exponent maximum, zero for the fallback.
        static constexpr i32   max_exponent10        = 0;
        /// @brief Number of exponent bits, zero for the fallback.
        static constexpr usize exponent              = 0;
        /// @brief Number of value bits.
        static constexpr usize digits                = detail::digits<T>();
        /// @brief Fractional mantissa bits, zero for the fallback.
        static constexpr usize mantissa              = 0;
        /// @brief Base-ten value digits.
        static constexpr usize digits10              = detail::digits10<T>();
        /// @brief Maximum base-ten round-trip digits, zero for the fallback.
        static constexpr usize max_digits10          = 0;
    };

    /// @brief IEEE single-precision numeric limits.
    template <>
    struct limits<f32>
    {
        /// @brief True because this specialization supplies floating metadata.
        static constexpr bool  is_specialized        = true;
        /// @brief Positive infinity.
        static constexpr f32   infinity              = detail::_infinity<f32>();
        /// @brief Not-a-number value.
        static constexpr f32   nan                   = detail::_quiet_nan<f32>();
        /// @brief Difference between one and the next representable value.
        static constexpr f32   epsilon               = 0x1p-23f;
        /// @brief Smallest positive normalized `f32`.
        static constexpr f32   min                   = 0x1p-126f;
        /// @brief Largest finite `f32`.
        static constexpr f32   max                   = 0x1.fffffep+127f;
        /// @brief Smallest positive subnormal `f32`.
        static constexpr f32   denorm_min            = 0x1p-149f;
        /// @brief Whether `f32` is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = -126;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = -38;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = -149;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = -45;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = 127;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = 38;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f32>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = 24;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = 23;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = 6;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = 9;
    };

    /// @brief IEEE double-precision numeric limits.
    template <>
    struct limits<f64>
    {
        /// @brief True because this specialization supplies floating metadata.
        static constexpr bool  is_specialized        = true;
        /// @brief Positive infinity.
        static constexpr f64   infinity              = detail::_infinity<f64>();
        /// @brief Not-a-number value.
        static constexpr f64   nan                   = detail::_quiet_nan<f64>();
        /// @brief Difference between one and the next representable value.
        static constexpr f64   epsilon               = 0x1p-52;
        /// @brief Smallest positive normalized `f64`.
        static constexpr f64   min                   = 0x1p-1022;
        /// @brief Largest finite `f64`.
        static constexpr f64   max                   = 0x1.fffffffffffffp+1023;
        /// @brief Smallest positive subnormal `f64`.
        static constexpr f64   denorm_min            = 0x1p-1074;
        /// @brief Whether `f64` is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = -1022;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = -308;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = -1074;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = -324;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = 1023;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = 308;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f64>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = 53;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = 52;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = 15;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = 17;
    };

    /// @brief Extended-precision floating-point numeric limits.
    /// @details `f128` is `long double`: 80-bit x87 extended precision on GCC and
    /// Clang for x86, and plain `double` on MSVC. Each value is picked by the size
    /// of `f128`, so both cases give the right numbers.
    template <>
    struct limits<f128>
    {
        /// @brief True because this specialization supplies floating metadata.
        static constexpr bool  is_specialized        = true;
        /// @brief Positive infinity.
        static constexpr f128  infinity              = detail::_infinity<f128>();
        /// @brief Not-a-number value.
        static constexpr f128  nan                   = detail::_quiet_nan<f128>();
        /// @brief Difference between one and the next representable value.
        static constexpr f128  epsilon               = detail::_extended<f128>(0x1p-52, detail::_x87_bits{0x8000000000000000ull, 0x3FC0, {}});
        /// @brief Smallest positive normalized extended value.
        static constexpr f128  min                   = detail::_extended<f128>(0x1p-1022, detail::_x87_bits{0x8000000000000000ull, 0x0001, {}});
        /// @brief Largest finite extended value.
        static constexpr f128  max                   = detail::_extended<f128>(0x1.fffffffffffffp+1023, detail::_x87_bits{0xFFFFFFFFFFFFFFFFull, 0x7FFE, {}});
        /// @brief Smallest positive subnormal extended value.
        static constexpr f128  denorm_min            = detail::_extended<f128>(0x1p-1074, detail::_x87_bits{0x0000000000000001ull, 0x0000, {}});
        /// @brief Whether the extended type is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = sizeof(f128) == 8 ? -1022 : -16382;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = sizeof(f128) == 8 ? -308 : -4932;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = sizeof(f128) == 8 ? -1074 : -16445;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = sizeof(f128) == 8 ? -324 : -4951;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = sizeof(f128) == 8 ? 1023 : 16383;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = sizeof(f128) == 8 ? 308 : 4932;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f128>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = sizeof(f128) == 8 ? 53 : 64;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = sizeof(f128) == 8 ? 52 : 63;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = sizeof(f128) == 8 ? 15 : 18;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = sizeof(f128) == 8 ? 17 : 21;
    };
} // namespace hsd
