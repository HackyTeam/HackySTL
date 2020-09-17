#pragma once

#include "TypeTraits.hpp"

namespace hsd
{
    template<typename T>
    static constexpr rev_ref_t<T>&& move(T&& val)
    {
        return static_cast<rev_ref_t<T>&&>(val);
    }

    template <typename T>
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

    template< typename InIt, typename OutIt >
    static constexpr OutIt move(InIt first, InIt last, OutIt dest)
    {
        while (first != last) 
        {
            *dest++ = hsd::move(*first++);
        }
        return dest;
    }

    template<typename _Type>
    static constexpr _Type* addressof(_Type& value)
    {
        return reinterpret_cast<_Type*>(&reinterpret_cast<char&>(value));
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

    template <typename T>
    static constexpr T&& forward(rev_ref_t<T>&& val)
    {
        static_assert(!std::is_lvalue_reference<T>::value,
                      "Can not forward an rvalue as an lvalue.");

        return static_cast<T&&>(val);
    }

    template<typename T1>
    static constexpr void swap(T1& first, T1& second) noexcept
    {
        auto _tmp = hsd::move(first);
        first = hsd::move(second);
        second = hsd::move(_tmp);
    }
    
    template< typename InIt, typename OutIt >
    static constexpr OutIt copy(InIt first, InIt last, OutIt dest)
    {
        while(first != last) 
        {
            *dest++ = *first++;
        }
        return dest;
    }

    template< typename InIt, typename OutIt >
    static constexpr OutIt copy_n(InIt first, usize n, OutIt dest)
    {
        for(usize _index = 0; _index != n; _index++) 
        {
            *dest++ = *first++;
        }
        return dest;
    }

    template< typename InIt, typename ValueType >
    static constexpr void set(InIt first, InIt last, ValueType value)
    {
        for(; first != last; *first++ = value);
    }

    template< typename InIt, typename OutIt, typename Pred >
    static constexpr OutIt copy_if(InIt first, InIt last, OutIt dest, Pred pred)
    {
        while(first != last) 
        {
            if(pred(*first))
                *dest++ = *first;
                
            first++;
        }
        return dest;
    }
}
