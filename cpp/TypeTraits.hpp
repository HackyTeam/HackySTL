#pragma once

#include <type_traits>

#include "Types.hpp"

namespace hsd
{
    struct true_type
    {
        static constexpr bool value = true;
    };
    
    struct false_type
    {
        static constexpr bool value = false;
    };

    template<bool, typename>
    struct enable_if
    {};

    template<typename T>
    struct enable_if<true, T>
    {
        using type = T;
    };

    template<bool, typename>
    struct disable_if
    {};

    template<typename T>
    struct disable_if<false, T>
    {
        using type = T;
    };

    template<typename... Cond> requires(sizeof...(Cond) > 1)
    struct conjunction
    {
        static constexpr bool value = (Cond::value && ...);
    };

    template<typename... Cond> requires(sizeof...(Cond) > 1)
    struct disjunction
    {
        static constexpr bool value = (Cond::value || ...);
    };

    template<typename Cond>
    struct negation
    {
        static constexpr bool value = !Cond::value;
    };

    template<bool Cond>
    struct conditional
    {
        static constexpr bool value = Cond;
    };
    

    template< typename, typename >
    struct is_same 
        : public false_type
    {};
    
    template<typename T>
    struct is_same<T, T>
        : public true_type
    {};

    template<typename T>
    struct remove_reference
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T&>
    {
        using type = T;
    };

    template<typename T>
    struct remove_reference<T&&>
    {
        using type = T;
    };

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
        : public false_type
    {};

    template<>
    struct is_char_pointer<char[]> 
        : public true_type
    {};

    template<usize N>
    struct is_char_pointer<char[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<char*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char* const> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<wchar[]> 
        : public true_type
    {};

    template<usize N>
    struct is_char_pointer<wchar[N]> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<wchar*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const wchar*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const wchar* const> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<char16[]> 
        : public true_type
    {};

    template<usize N>
    struct is_char_pointer<char16[N]> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<char16*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char16*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char16* const> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<char32[]> 
        : public true_type
    {};

    template<usize N>
    struct is_char_pointer<char32[N]> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<char32*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char32*> 
        : public true_type
    {};

    template<>
    struct is_char_pointer<const char32* const> 
        : public true_type
    {};

    template<typename>
    struct is_char
        : public false_type
    {};

    template<>
    struct is_char<char> 
        : public true_type
    {};

    template<>
    struct is_char<const char> 
        : public true_type
    {};

    template<>
    struct is_char<uchar> 
        : public true_type
    {};

    template<>
    struct is_char<const uchar> 
        : public true_type
    {};

    template<>
    struct is_char<wchar> 
        : public true_type
    {};

    template<>
    struct is_char<const wchar> 
        : public true_type
    {};

    template<>
    struct is_char<char16> 
        : public true_type
    {};

    template<>
    struct is_char<const char16> 
        : public true_type
    {};

    template<>
    struct is_char<char32> 
        : public true_type
    {};

    template<>
    struct is_char<const char32> 
        : public true_type
    {};

    template< typename Condition, typename Value >
    using ResolvedType = typename enable_if< Condition::value, Value >::type;

    template< typename From, typename To, typename Result >
	using EnableIfConvertible = ResolvedType< std::is_convertible<From, To>, Result >;

    template< typename First, typename Second, typename Result >
	using EnableIfSame = ResolvedType< is_same<First, Second>, Result >;

    template<typename T>
    using rev_ref_t = typename remove_reference<T>::type;
} // namespace hsd
