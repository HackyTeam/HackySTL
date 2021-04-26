#pragma once

#include "_Define.hpp"

namespace hsd
{
    namespace detail
    {
        template <typename>
        struct remove_unsigned;

        template <>
        struct remove_unsigned<unsigned char>
        {
            using type = char;
        };

        template <>
        struct remove_unsigned<unsigned short>
        {
            using type = short;
        };

        template <>
        struct remove_unsigned<unsigned int>
        {
            using type = int;
        };

        template <>
        struct remove_unsigned<unsigned long>
        {
            using type = long;
        };

        template <>
        struct remove_unsigned<unsigned long long>
        {
            using type = long long;
        };
    } // namespace detail

    using usize = decltype(sizeof(int));
    using isize = detail::remove_unsigned<usize>::type;

    #ifdef HSD_COMPILER_GCC
    using u128 = __uint128_t;
    using i128 = __int128_t;
    #endif

    using ulong = unsigned long;
    using u64 = unsigned long long;
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

    using i64 = long long;
    using i32 = int;
    using i16 = short;
    using i8 = char;

    using wchar = wchar_t;
    using uchar = unsigned char;
    using char8 = char8_t;
    using char16 = char16_t;
    using char32 = char32_t;

    using f128 = long double;
    using f64 = double;
    using f32 = float;
    
    using NullType = decltype(nullptr);

    namespace trivial_literals
    {
        usize operator""_sz(u64 value)
        {
            return static_cast<usize>(value);
        }
    } // namespace trivial_type_literals
} // namespace hsd
