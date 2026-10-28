#pragma once

/// @file Types.hpp
/// @brief Canonical scalar, character, size, and pointer aliases.
/// @details Centralizes the type vocabulary used by every HackySTL subsystem,
/// including platform-sized integers and Unicode character types.

#include "_Define.hpp"

#if !defined(HSD_COMPILER_GCC) || defined(HSD_LIBRARY_INT128)
#include "_Int128.hpp"
#endif

#include <stdint.h>
#include <stddef.h>

namespace hsd
{
    namespace type_detail
    {
        /// @brief Maps an unsigned integer type to its signed counterpart.
        template <typename T>
        struct remove_unsigned
        {
            /// @brief Mapped signed type, or `T` for unsupported types.
            using type = T;
        };

        /// @brief Maps unsigned char to signed char.
        template <>
        struct remove_unsigned<unsigned char>
        {
            /// @brief Signed counterpart of unsigned char.
            using type = signed char;
        };

        /// @brief Maps unsigned short to signed short.
        template <>
        struct remove_unsigned<unsigned short>
        {
            /// @brief Signed counterpart of unsigned short.
            using type = short;
        };

        /// @brief Maps unsigned int to signed int.
        template <>
        struct remove_unsigned<unsigned int>
        {
            /// @brief Signed counterpart of unsigned int.
            using type = int;
        };

        /// @brief Maps unsigned long to signed long.
        template <>
        struct remove_unsigned<unsigned long>
        {
            /// @brief Signed counterpart of unsigned long.
            using type = long;
        };

        /// @brief Maps unsigned long long to signed long long.
        template <>
        struct remove_unsigned<unsigned long long>
        {
            /// @brief Signed counterpart of unsigned long long.
            using type = long long;
        };

        /// @brief Maps a signed integer type to its unsigned counterpart.
        template <typename T>
        struct add_unsigned
        {
            /// @brief Mapped unsigned type, or `T` for unsupported types.
            using type = T;
        };

        /// @brief Maps char to unsigned char.
        template <>
        struct add_unsigned<char>
        {
            /// @brief Unsigned counterpart of char.
            using type = unsigned char;
        };

        /// @brief Maps short to unsigned short.
        template <>
        struct add_unsigned<short>
        {
            /// @brief Unsigned counterpart of short.
            using type = unsigned short;
        };

        /// @brief Maps int to unsigned int.
        template <>
        struct add_unsigned<int>
        {
            /// @brief Unsigned counterpart of int.
            using type = unsigned int;
        };

        /// @brief Maps long to unsigned long.
        template <>
        struct add_unsigned<long>
        {
            /// @brief Unsigned counterpart of long.
            using type = unsigned long;
        };

        /// @brief Maps long long to unsigned long long.
        template <>
        struct add_unsigned<long long>
        {
            /// @brief Unsigned counterpart of long long.
            using type = unsigned long long;
        };

        /// @brief Maps signed char to unsigned char; `i8` is this type.
        template <>
        struct add_unsigned<signed char>
        {
            /// @brief Unsigned counterpart of signed char.
            using type = unsigned char;
        };

    #if defined(HSD_COMPILER_GCC) && !defined(HSD_LIBRARY_INT128)
        /// @brief Maps __int128 to unsigned __int128; `i128` is this type.
        template <>
        struct add_unsigned<__int128>
        {
            /// @brief Unsigned counterpart of __int128.
            using type = unsigned __int128;
        };
    #endif

    #if !defined(HSD_COMPILER_GCC) || defined(HSD_LIBRARY_INT128)
        /// @brief Maps the library's signed 128-bit type to its unsigned counterpart.
        template <>
        struct add_unsigned<int128_detail::basic_int128<true>>
        {
            /// @brief Unsigned counterpart of `i128`.
            using type = int128_detail::basic_int128<false>;
        };
    #endif
    } // namespace type_detail

    /// @brief Unsigned platform size type.
    using usize = decltype(sizeof(int));
    /// @brief Signed platform size type.
    using isize = type_detail::remove_unsigned<usize>::type;

    #if defined(HSD_COMPILER_GCC) && !defined(HSD_LIBRARY_INT128)
    /// @brief Unsigned 128-bit integer.
    using u128 = __uint128_t;
    /// @brief Signed 128-bit integer.
    using i128 = __int128_t;
    #else
    /// @brief Unsigned 128-bit integer, implemented by the library (see `_Int128.hpp`).
    using u128 = int128_detail::basic_int128<false>;
    /// @brief Signed 128-bit integer, implemented by the library (see `_Int128.hpp`).
    using i128 = int128_detail::basic_int128<true>;
    #endif

    /// @brief Unsigned long integer.
    using ulong = unsigned long;
    /// @brief Unsigned 64-bit integer.
    using u64 = unsigned long long;
    /// @brief Unsigned 32-bit integer.
    using u32 = unsigned int;
    /// @brief Unsigned 16-bit integer.
    using u16 = unsigned short;
    /// @brief Unsigned 8-bit integer.
    using u8 = unsigned char;

    /// @brief Signed long integer.
    using ilong = long;
    /// @brief Signed 64-bit integer.
    using i64 = long long;
    /// @brief Signed 32-bit integer.
    using i32 = int;
    /// @brief Signed 16-bit integer.
    using i16 = short;
    /// @brief Signed 8-bit integer.
    using i8 = signed char;

    /// @brief Unsigned pointer-sized integer.
    using uptr = uintptr_t;
    /// @brief Signed pointer-sized integer.
    using iptr = intptr_t;
    /// @brief Pointer difference type.
    using pdiff = ptrdiff_t;
    /// @brief Maximum-width signed integer.
    using imax = intmax_t;
    /// @brief Maximum-width unsigned integer.
    using umax = uintmax_t;

    /// @brief Wide character type.
    using wchar = wchar_t;
    /// @brief Signed narrow character type.
    using schar = signed char;
    /// @brief Unsigned narrow character type.
    using uchar = unsigned char;
    /// @brief UTF-8 code unit type.
    using char8 = char8_t;
    /// @brief UTF-16 code unit type.
    using char16 = char16_t;
    /// @brief UTF-32 code unit type.
    using char32 = char32_t;

    /// @brief Extended floating-point type.
    using f128 = long double;
    /// @brief Double-precision floating-point type.
    using f64 = double;
    /// @brief Single-precision floating-point type.
    using f32 = float;
    
    /// @brief Null pointer type.
    using null_type = decltype(nullptr);

    /// @brief Unsigned counterpart alias for a scalar type.
    template <typename T>
    using add_unsigned_t = typename type_detail::add_unsigned<T>::type;

    /// @brief Signed counterpart alias for a scalar type.
    template <typename T>
    using remove_unsigned_t = typename type_detail::remove_unsigned<T>::type;

    namespace trivial_literals
    {
        /// @brief Converts an integer literal to `usize`.
        /// @param value Literal value.
        /// @return Value converted to `usize`.
        consteval usize operator""_usz(u64 value)
        {
            return static_cast<usize>(value);
        }

        /// @brief Converts an integer literal to `isize`.
        /// @param value Literal value.
        /// @return Value converted to `isize`.
        consteval usize operator""_isz(u64 value)
        {
            return static_cast<isize>(value);
        }
    } // namespace trivial_type_literals
} // namespace hsd
