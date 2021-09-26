#pragma once

#include "TypeTraits.hpp"
#include "IntegerSequence.hpp"

namespace hsd
{
    #define HSD_CPP17_NOT_REQUIRES(Cond) hsd::disable_if_t<Cond, hsd::i32> = 0
    #define HSD_CPP17_REQUIRES(Cond) hsd::enable_if_t<Cond, hsd::i32> = 0

    #define HSD_ENABLE_IF_ALL(...) HSD_CPP17_REQUIRES(hsd::conjunction<(__VA_ARGS__)>::value)
    #define HSD_DISABLE_IF_ALL(...) HSD_CPP17_NOT_REQUIRES(hsd::conjunction<(__VA_ARGS__)>::value)
    #define HSD_ENABLE_IF_SOME(...) HSD_CPP17_REQUIRES(hsd::disjunction<(__VA_ARGS__)>::value)
    #define HSD_DISABLE_IF_SOME(...) HSD_CPP17_NOT_REQUIRES(hsd::disjunction<(__VA_ARGS__)>::value)

    template <typename T>
    static constexpr remove_reference_t<T>&& move(T&& val)
    {
        return static_cast<remove_reference_t<T>&&>(val);
    }

    template <typename T>
    static constexpr remove_reference_t<T> release(T&& val)
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

    template <typename T, typename U = T>
    static constexpr T exchange(T& target, U&& new_val) noexcept
    {
        T tmp = move(target);
        target = forward<U>(new_val);
        return tmp;
    }

    template <typename InIt, typename OutIt>
    static constexpr OutIt move(InIt first, InIt last, OutIt dest)
    {
        while (first != last) 
            *dest++ = move(*first++);

        return dest;
    }

    template <typename To>
    [[nodiscard]] static constexpr auto bit_cast(auto from)
    {
        return __builtin_bit_cast(To, from);
    }

    template <typename T, typename... Args>
    static consteval void literal_construct(T& dest, Args&&... args)
    {
        dest = T{forward<Args>(args)...};
    }

    template <typename Type>
    static constexpr const Type* addressof(const Type& value)
    {
        if constexpr (requires{value.operator&();})
        {
            return bit_cast<Type*>(
                &reinterpret_cast<char&>(
                    const_cast<Type&>(value)
                )
            );
        }
        else
        {
            return &value;
        }
    }

    template <typename Type>
    static constexpr Type* addressof(Type& value)
    {
        if constexpr (requires {value.operator&();})
        {
            return bit_cast<Type*>(
                &reinterpret_cast<char&>(
                    const_cast<Type&>(value)
                )
            );
        }
        else
        {
            return &value;
        }
    }

    template <typename T1>
    static constexpr void swap(T1& first, T1& second) noexcept
    {
        auto _tmp = move(first);
        first = move(second);
        second = move(_tmp);
    }
    
    template <typename InIt, typename OutIt>
    static constexpr OutIt copy(InIt first, InIt last, OutIt dest)
    {
        while (first != last)
        { 
            *dest++ = *first++;
        }

        return dest;
    }

    template <typename InIt, typename OutIt>
    static constexpr OutIt copy_n(InIt first, usize n, OutIt dest)
    {
        using value_type = remove_reference_t<decltype(*dest)>;

        for (usize _index = 0; _index != n; _index++)
        {
            *dest++ = static_cast<value_type>(*first++);
        }
    
        return dest;
    }

    template <typename InIt, typename ValueType>
    static constexpr void set(InIt first, InIt last, ValueType value)
    {
        for (; first != last; *first++ = value)
            ;
    }

    template < typename InIt, typename OutIt, typename Pred >
    static constexpr OutIt copy_if(InIt first, InIt last, OutIt dest, Pred pred)
    {
        while (first != last) 
        {
            if (pred(*first))
            {
                *dest++ = *first;
            }
                
            first++;
        }
        return dest;
    }

    template <typename Elem, usize Count>
    static constexpr Elem* begin(Elem (&arr)[Count])
    {
        return static_cast<Elem*>(arr);
    }

    template <typename Elem, usize Count>
    static constexpr Elem* end(Elem (&arr)[Count])
    {
        return static_cast<Elem*>(arr) + Count;
    }

    template <typename Elem, usize Count>
    static constexpr const Elem* begin(const Elem (&arr)[Count])
    {
        return static_cast<const Elem*>(arr);
    }


    template <typename Elem, usize Count>
    static constexpr const Elem* end(const Elem (&arr)[Count])
    {
        return static_cast<const Elem*>(arr) + Count;
    }
} // namespace hsd
