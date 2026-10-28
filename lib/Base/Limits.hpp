#pragma once

/// @file Limits.hpp
/// @brief Numeric boundary constants for HackySTL scalar types.
/// @details Exposes project-compatible minimum, maximum, infinity, and denormal
/// values used by numeric containers, math, formatting, and random generation.

#include "Types.hpp"

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
        static constexpr f32   infinity              = __builtin_huge_valf();
        /// @brief Not-a-number value.
        static constexpr f32   nan                   = __builtin_nanf("");
        /// @brief Difference between one and the next representable value.
        static constexpr f32   epsilon               = __FLT_EPSILON__;
        /// @brief Smallest positive normalized `f32`.
        static constexpr f32   min                   = __FLT_MIN__;
        /// @brief Largest finite `f32`.
        static constexpr f32   max                   = __FLT_MAX__;
        /// @brief Smallest positive subnormal `f32`.
        static constexpr f32   denorm_min            = __FLT_DENORM_MIN__;
        /// @brief Whether `f32` is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = __FLT_MIN_EXP__ - 1;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = __FLT_MIN_10_EXP__ - 1;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = -149;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = -45;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = __FLT_MAX_EXP__ - 1;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = __FLT_MAX_10_EXP__;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f32>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = __FLT_MANT_DIG__;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = __FLT_MANT_DIG__ - 1;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = __FLT_DIG__;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = __FLT_DECIMAL_DIG__;
    };

    /// @brief IEEE double-precision numeric limits.
    template <>
    struct limits<f64>
    {
        /// @brief True because this specialization supplies floating metadata.
        static constexpr bool  is_specialized        = true;
        /// @brief Positive infinity.
        static constexpr f64   infinity              = __builtin_huge_val();
        /// @brief Not-a-number value.
        static constexpr f64   nan                   = __builtin_nan("");
        /// @brief Difference between one and the next representable value.
        static constexpr f64   epsilon               = __DBL_EPSILON__;
        /// @brief Smallest positive normalized `f64`.
        static constexpr f64   min                   = __DBL_MIN__;
        /// @brief Largest finite `f64`.
        static constexpr f64   max                   = __DBL_MAX__;
        /// @brief Smallest positive subnormal `f64`.
        static constexpr f64   denorm_min            = __DBL_DENORM_MIN__;
        /// @brief Whether `f64` is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = __DBL_MIN_EXP__ - 1;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = __DBL_MIN_10_EXP__ - 1;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = -1074;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = -324;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = __DBL_MAX_EXP__ - 1;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = __DBL_MAX_10_EXP__;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f64>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = __DBL_MANT_DIG__;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = __DBL_MANT_DIG__ - 1;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = __DBL_DIG__;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = __DBL_DECIMAL_DIG__;
    };

    /// @brief Extended-precision floating-point numeric limits.
    template <>
    struct limits<f128>
    {
        /// @brief True because this specialization supplies floating metadata.
        static constexpr bool  is_specialized        = true;
        /// @brief Positive infinity.
        static constexpr f128  infinity              = __builtin_huge_vall();
        /// @brief Not-a-number value.
        static constexpr f128  nan                   = __builtin_nanl("");
        /// @brief Difference between one and the next representable value.
        static constexpr f128  epsilon               = __LDBL_EPSILON__;
        /// @brief Smallest positive normalized extended value.
        static constexpr f128  min                   = __LDBL_MIN__;
        /// @brief Largest finite extended value.
        static constexpr f128  max                   = __LDBL_MAX__;
        /// @brief Smallest positive subnormal extended value.
        static constexpr f128  denorm_min            = __LDBL_DENORM_MIN__;
        /// @brief Whether the extended type is signed.
        static constexpr bool  is_signed             = true;
        /// @brief Minimum binary exponent for normalized values.
        static constexpr i32   min_exponent          = __LDBL_MIN_EXP__ - 1;
        /// @brief Minimum decimal exponent for normalized values.
        static constexpr i32   min_exponent10        = __LDBL_MIN_10_EXP__ - 1;
        /// @brief Minimum binary exponent for subnormal values.
        static constexpr i32   min_exponent_denorm   = -16445;
        /// @brief Minimum decimal exponent for subnormal values.
        static constexpr i32   min_exponent10_denorm = -4951;
        /// @brief Maximum binary exponent for finite values.
        static constexpr i32   max_exponent          = __LDBL_MAX_EXP__ - 1;
        /// @brief Maximum decimal exponent for finite values.
        static constexpr i32   max_exponent10        = __LDBL_MAX_10_EXP__;
        /// @brief Number of exponent bits in the representation.
        static constexpr usize exponent              = detail::_exp_bits<f128>();
        /// @brief Significant binary digits including the hidden bit.
        static constexpr usize digits                = __LDBL_MANT_DIG__;
        /// @brief Explicit mantissa bits excluding the hidden bit.
        static constexpr usize mantissa              = __LDBL_MANT_DIG__ - 1;
        /// @brief Decimal digits preserved without change.
        static constexpr usize digits10              = __LDBL_DIG__;
        /// @brief Decimal digits needed for round-trip conversion.
        static constexpr usize max_digits10          = __LDBL_DECIMAL_DIG__;
    };
} // namespace hsd