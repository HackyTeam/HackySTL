#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename T1, typename T2>
    struct pair
    {
        T1 first;
        T2 second;

        constexpr ~pair() = default;
        constexpr pair() = default;

        constexpr pair(const T1& first_val, const T2& second_val)
            : first{first_val}, second{second_val}
        {}

        constexpr pair(T1&& first_val, T2&& second_val)
            : first{forward<T1>(first_val)}, second{forward<T2>(second_val)}
        {}

        constexpr pair(const pair& other)
            : first{other.first}, second{other.second}
        {}
        
        constexpr pair(pair&& other)
            : first{forward<T1>(other.first)}, second{forward<T2>(other.second)}
        {}

        constexpr pair& operator=(const pair& rhs)
        {
            first = rhs.first;
            second = rhs.second;
            return *this;
        }
        
        constexpr pair& operator=(pair&& rhs)
        {
            first = forward<T1>(rhs.first);
            second = forward<T2>(rhs.second);
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