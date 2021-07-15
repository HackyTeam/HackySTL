#pragma once

#include "Types.hpp"

namespace hsd
{
    namespace detail
    {
        template <typename T>
        constexpr bool is_signed_byte()
        {
            return static_cast<T>(-1) < 0;
        }

        template <typename T, usize B>
        constexpr usize digits_byte()
        {
            return B - is_signed_byte<T>();
        }

        template <typename T, usize B>
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
        constexpr T min_byte()
        {
            if constexpr (is_signed_byte<T>())
            {
                return -max_byte<T, B>() - 1;
            }

            return 0;
        }

        template <typename T, usize B>
        constexpr usize digits10_byte()
        {
            return digits_byte<T, B>() * 643L / 2136;
        }

        template <typename T>
        constexpr bool is_signed()
        {
            return is_signed_byte<T>();
        }

        template <typename T>
        constexpr auto min()
        {
            return min_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        constexpr auto max()
        {
            return max_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        constexpr auto digits() 
        {
            return digits_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        constexpr auto digits10()
        {
            return digits10_byte<T, sizeof(T) * 8>();
        }

        template <typename T>
        constexpr T max_digits10(T value)
        {
            return 2 + value * 643L / 2136;
        }
    } // namespace detail
    

    template <typename T>
    struct limits
    {
        static constexpr bool is_specialized = false;
        static constexpr T infinity = 0;
        static constexpr T epsilon = 0;
        static constexpr T nan = 0;
        static constexpr T min = detail::min<T>();
        static constexpr T max = detail::max<T>();
        static constexpr bool is_signed = detail::is_signed<T>();
        static constexpr usize digits = detail::digits<T>();
        static constexpr usize digits10 = detail::digits10<T>();
        static constexpr usize max_digits10 = 0;
    }; 

    template <>
    struct limits<f32>
    {
        static constexpr bool is_specialized = true;
        #if defined(HSD_COMPILER_MSVC)
        static constexpr f32 infinity = __builtin_huge_valf();
        static constexpr f32 nan = __builtin_nanf("");
        #else
        static constexpr f32 infinity = __builtin_inff();
        static constexpr f32 nan = __builtin_nanf("");
        #endif
        static constexpr f32 epsilon = 1.19209290e-7f;
        static constexpr f32 min = 1.17549435e-38f;
        static constexpr f32 max = 3.40282347e+38f;
        static constexpr bool is_signed = true;
        static constexpr usize digits = 24;
        static constexpr usize digits10 = 6;
        static constexpr usize max_digits10 = 9;
    };

    template <>
    struct limits<f64>
    {
        static constexpr bool is_specialized = true;
        #if defined(HSD_COMPILER_MSVC)
        static constexpr f64 infinity = __builtin_huge_val();
        static constexpr f64 nan = __builtin_nans("");
        #else
        static constexpr f64 infinity = __builtin_inff();
        static constexpr f64 nan = __builtin_nanf("");
        #endif
        static constexpr f64 epsilon = 2.2204460492503131e-16;
        static constexpr f64 min = 2.2250738585072014e-308;
        static constexpr f64 max = 1.7976931348623157e+308;
        static constexpr bool is_signed = true;
        static constexpr usize digits = 53;
        static constexpr usize digits10 = 6;
        static constexpr usize max_digits10 = 15;
    };

    template <>
    struct limits<f128>
    {
        static constexpr bool is_specialized = true;
        #if defined(HSD_COMPILER_MSVC)
        static constexpr f128 infinity = __builtin_huge_val();
        static constexpr f128 nan = __builtin_nans("");
        #else
        static constexpr f128 infinity = __builtin_inff();
        static constexpr f128 nan = __builtin_nanf("");
        #endif
        static constexpr f128 epsilon = 1.08420217248550443401e-19l;
        #if defined(HSD_PLATFORM_WINDOWS)
        static constexpr f128 min = 4.9406564584124654E-324l;
        static constexpr f128 max = 1.7976931348623157e+308l;
        #else
        static constexpr f128 min = 3.36210314311209350626e-4932l;
        static constexpr f128 max = 1.18973149535723176502e+4932l;
        #endif
        static constexpr bool is_signed = true;
        static constexpr usize digits = 64;
        static constexpr usize digits10 = 18;
        static constexpr usize max_digits10 = 21;
    };
} // namespace hsd