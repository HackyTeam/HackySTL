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

    template<typename T, size_t N>
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

    template<size_t N>
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
    struct is_char_pointer<wchar_t[]> 
        : public std::true_type
    {};

    template<size_t N>
    struct is_char_pointer<wchar_t[N]> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<wchar_t*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const wchar_t*> 
        : public std::true_type
    {};

    template<>
    struct is_char_pointer<const wchar_t* const> 
        : public std::true_type
    {};

    template<typename T>
    using rev_ref_t = typename std::remove_reference<T>::type;

    template<typename T>
    static constexpr rev_ref_t<T>&& move(T&& val)
    {
        return static_cast<rev_ref_t<T>&&>(val);
    }

    template <class T>
    static constexpr T&& forward(rev_ref_t<T>& val)
    {
        return static_cast<T&&>(val);
    }

    template <class T, class U = T>
    static constexpr T exchange(T& target, U&& new_val) noexcept
    {
        T tmp = move(target);
        target = forward<U>(new_val);
        return tmp;
    }

    template<class InIt, class OutIt>
    static constexpr OutIt move(InIt first, InIt last, OutIt dest)
    {
        while (first != last) 
        {
            *dest++ = hsd::move(*first++);
        }
        return dest;
    }

    template<typename T1>
    static constexpr T1 min(T1 first, T1 second) noexcept
    {
        return first < second ? first : second;
    }

    template<typename T1>
    static constexpr T1 max(T1 first, T1 second) noexcept
    {
        return first > second ? first : second;
    }

    template <class T>
    static constexpr T&& forward(rev_ref_t<T>&& val)
    {
        static_assert(!std::is_lvalue_reference<T>::value,
                      "Can not forward an rvalue as an lvalue.");

        return static_cast<T&&>(val);
    }

    template<typename T1>
    static constexpr void swap(T1& first, T1& second) noexcept
    {
        auto _tmp = move(first);
        first = move(second);
        second = move(_tmp);
    }

    template<class InIt, class OutIt>
    static constexpr OutIt copy(InIt first, InIt last, OutIt dest)
    {
        while(first != last) 
        {
            *dest++ = *first++;
        }
        return dest;
    }

    template<class InIt, class OutIt, class Pred>
    static constexpr OutIt copy_if(InIt first, InIt last, OutIt dest, Pred pred)
    {
        while(first != last) 
        {
            if(pred(*first))
            { 
                *dest++ = *first;
            }
            first++;
        }
        return dest;
    }
}
