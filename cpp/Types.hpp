#pragma once

#include "_Define.hpp"

namespace hsd
{
    namespace type_detail
    {
        template <typename T>
        struct remove_unsigned
        {
            using type = T;
        };

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

        template <typename T>
        struct add_unsigned
        {
            using type = T;
        };

        template <>
        struct add_unsigned<char>
        {
            using type = unsigned char;
        };

        template <>
        struct add_unsigned<short>
        {
            using type = unsigned short;
        };

        template <>
        struct add_unsigned<int>
        {
            using type = unsigned int;
        };

        template <>
        struct add_unsigned<long>
        {
            using type = unsigned long;
        };

        template <>
        struct add_unsigned<long long>
        {
            using type = unsigned long long;
        };
    } // namespace type_detail

    using usize = decltype(sizeof(int));
    using isize = type_detail::remove_unsigned<usize>::type;

    #ifdef HSD_COMPILER_GCC
    using u128 = __uint128_t;
    using i128 = __int128_t;
    #endif

    using ushort = unsigned short;
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
    using schar = signed char;
    using uchar = unsigned char;
    using char8 = char8_t;
    using char16 = char16_t;
    using char32 = char32_t;

    using f128 = long double;
    using f64 = double;
    using f32 = float;
    
    using NullType = decltype(nullptr);

    template <typename T>
    using add_unsigned_t = typename type_detail::add_unsigned<T>::type;

    template <typename T>
    using remove_unsigned_t = typename type_detail::remove_unsigned<T>::type;

    namespace trivial_literals
    {
        usize operator""_sz(u64 value)
        {
            return static_cast<usize>(value);
        }
    } // namespace trivial_type_literals
} // namespace hsd
