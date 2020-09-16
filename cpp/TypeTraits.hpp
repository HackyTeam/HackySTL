#pragma once

#include <type_traits>

#include "Types.hpp"

namespace hsd
{
    template<typename T>
    struct remove_array
    {
        using type = T;
    };

    template<typename T>
    struct remove_array<T[]>
    {
        using type = T;
    };

    template<typename T, usize N>
    struct remove_array<T[N]>
    {
        using type = T;
    };
    
    template<typename T>
    using remove_array_t = typename remove_array<T>::type;

    template<typename>
    struct is_char_pointer 
        : public std::false_type
    {};

    template<>
    struct is_char_pointer<char[]> 
        : public std::true_type
    {};

    template<usize N>
    struct is_char_pointer<char[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char* const> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<wchar[]> 
        : public std::true_type
    {};

    template<usize N>
    struct is_char_pointer<wchar[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<wchar*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const wchar*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const wchar* const> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char16[]> 
        : public std::true_type
    {};

    template<usize N>
    struct is_char_pointer<char16[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char16*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char16*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char16* const> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char32[]> 
        : public std::true_type
    {};

    template<usize N>
    struct is_char_pointer<char32[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char32*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char32*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const char32* const> 
        : public std::true_type
    {};

    template<typename>
    struct is_char
        : public std::false_type
    {};

    template<>
    struct is_char<char> 
        : public std::true_type
    {};

    template<>
    struct is_char<const char> 
        : public std::true_type
    {};

    template<>
    struct is_char<uchar> 
        : public std::true_type
    {};

    template<>
    struct is_char<const uchar> 
        : public std::true_type
    {};

    template<>
    struct is_char<wchar> 
        : public std::true_type
    {};

    template<>
    struct is_char<const wchar> 
        : public std::true_type
    {};

    template<>
    struct is_char<char16> 
        : public std::true_type
    {};

    template<>
    struct is_char<const char16> 
        : public std::true_type
    {};

    template<>
    struct is_char<char32> 
        : public std::true_type
    {};

    template<>
    struct is_char<const char32> 
        : public std::true_type
    {};

    template< typename Condition, typename Value >
    using ResolvedType = typename std::enable_if< Condition::value, Value >::type;

    template< typename From, typename To, typename Result >
	using EnableIfConvertible = ResolvedType<std::is_convertible<From, To>, Result>;

    template< typename First, typename Second, typename Result >
	using EnableIfSame = ResolvedType<std::is_same<First, Second>, Result>;

    template<typename T>
    using rev_ref_t = typename std::remove_reference<T>::type;
} // namespace hsd
