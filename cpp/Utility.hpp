#pragma once

#include "TypeTraits.hpp"
#include "IntegerSequence.hpp"

namespace hsd
{
    #define HSD_ENABLE_IF(...) hsd::enable_if_t<(__VA_ARGS__), hsd::i32> = 0
    #define HSD_DISABLE_IF(...) hsd::disable_if_t<(__VA_ARGS__), hsd::i32> = 0

    template <typename T>
    static constexpr remove_reference_t<T>&& move(T&& val)
    {
        return static_cast<remove_reference_t<T>&&>(val);
    }

    template <typename T>
    static constexpr T&& forward(remove_reference_t<T>& val)
    {
        return static_cast<T&&>(val);
    }

    template <typename T>
    static constexpr T&& forward(remove_reference_t<T>&& val)
    {
        return static_cast<T&&>(val);
    }

    template < typename T, typename... Args >
    static constexpr void construct_at(T* ptr, Args&&... args)
    {
        if(std::is_constant_evaluated())
        {
            (*ptr) = T{forward<Args>(args)...};
        }
        else
        {
            new (ptr) T{forward<Args>(args)...};
        }
    }

    template <class T, class U = T>
    static constexpr T exchange(T& target, U&& new_val) noexcept
    {
        T tmp = move(target);
        target = forward<U>(new_val);
        return tmp;
    }

    template < typename InIt, typename OutIt >
    static constexpr OutIt move(InIt first, InIt last, OutIt dest)
    {
        while (first != last) 
        {
            *dest++ = hsd::move(*first++);
        }
        return dest;
    }

    template <typename Type>
    static constexpr Type* addressof(const Type& value)
    {
        return reinterpret_cast<Type*>(
            &reinterpret_cast<char&>(
                const_cast<Type&>(value)
            )
        );
    }

    template <typename T1>
    static constexpr void swap(T1& first, T1& second) noexcept
    {
        auto _tmp = hsd::move(first);
        first = hsd::move(second);
        second = hsd::move(_tmp);
    }
    
    template < typename InIt, typename OutIt >
    static constexpr OutIt copy(InIt first, InIt last, OutIt dest)
    {
        while(first != last) 
        {
            *dest++ = *first++;
        }
        return dest;
    }

    template < typename InIt, typename OutIt >
    static constexpr OutIt copy_n(InIt first, usize n, OutIt dest)
    {
        using value_type = remove_reference_t<decltype(*dest)>;

        for(usize _index = 0; _index != n; _index++)
            *dest++ = static_cast<value_type>(*first++);
    
        return dest;
    }

    template < typename InIt, typename ValueType >
    static constexpr void set(InIt first, InIt last, ValueType value)
    {
        for(; first != last; *first++ = value);
    }

    template < typename InIt, typename OutIt, typename Pred >
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
