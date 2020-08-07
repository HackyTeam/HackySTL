#pragma once

#include <type_traits>
#include <stddef.h>

#include "Types.hpp"

namespace hsd
{
    template< typename T1, typename T2 >
    struct pair
    {
        T1 fist;
        T2 second;
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

    template<typename T, size_t N>
    struct remove_array<T[N]>
    {
        using type = T;
    };
    
    template<typename T>
    using remove_array_t = typename remove_array<T>::type;

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
    static constexpr T1 min(T1 first, T1 second)
    {
        return first < second ? first : second;
    }

    template<typename T1>
    static constexpr T1 max(T1 first, T1 second)
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
    static constexpr void swap(T1& first, T1& second)
    {
        auto _tmp = first;
        first = second;
        second = _tmp;
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
