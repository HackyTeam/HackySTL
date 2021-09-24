#pragma once

#include "Concepts.hpp"

namespace hsd
{
    template <typename T1, typename T2>
    struct pair
    {
        T1 first;
        T2 second;

        constexpr ~pair() = default;
        constexpr pair() = default;

        template <typename _T1, typename _T2>
        constexpr pair(_T1&& first_val, _T2&& second_val)
            : first{forward<_T1>(first_val)}, second{forward<_T2>(second_val)}
        {}

        constexpr pair(const pair& other)
            : first{other.first}, second{other.second}
        {}
        
        constexpr pair(pair&& other)
            : first{move(other.first)}, second{move(other.second)}
        {}

        constexpr pair& operator=(const pair& rhs)
        {
            first = rhs.first;
            second = rhs.second;
            return *this;
        }
        
        constexpr pair& operator=(pair&& rhs)
        {
            swap(first, rhs.first);
            swap(second, rhs.second);
            return *this;
        }

        constexpr bool operator==(const pair& rhs)
        {
            return first == rhs.first && second == rhs.second;
        }
    };

    template <typename T1, typename T2> pair(const T1&, const T2&) 
        -> pair<decay_t<T1>, decay_t<T2>>;
    template <typename T1, typename T2> requires (!is_const<T1>::value)
        pair(T1&&, const T2&) -> pair<decay_t<T1>, decay_t<T2>>;
    template <typename T1, typename T2> requires (is_const<T1>::value)
        pair(T1&&, const T2&) -> pair<const decay_t<T1>, decay_t<T2>>;
    template <typename T1, typename T2> requires (!is_const<T2>::value)
        pair(const T1&, T2&&) -> pair<decay_t<T1>, decay_t<T2>>;
    template <typename T1, typename T2> requires (is_const<T2>::value)
        pair(const T1&, T2&&) -> pair<decay_t<T1>, const decay_t<T2>>;

    template <typename T1, typename T2> 
    requires (!is_const<T1>::value && !is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<decay_t<T1>, decay_t<T2>>;

    template <typename T1, typename T2> 
    requires (is_const<T1>::value && !is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<const decay_t<T1>, decay_t<T2>>;

    template <typename T1, typename T2> 
    requires (!is_const<T1>::value && is_const<T2>::value)
        pair(T1&&, T2&&)-> pair<decay_t<T1>, const decay_t<T2>>;

    template <typename T1, typename T2>
    requires (is_const<T1>::value && is_const<T2>::value)
        pair(T1&&, T2&&) -> pair<const decay_t<T1>, const decay_t<T2>>;
} // namespace hsd