#pragma once

#include "Utility.hpp"

namespace hsd
{
    template< typename T1, typename T2 >
    struct pair
    {
        T1 first;
        T2 second;

        constexpr bool operator==(const pair& rhs)
        {
            return first == rhs.first && second == rhs.second;
        }
    };

    template< typename T1, typename T2 > pair(const T1&, const T2&) 
        -> pair<decay_t<T1>, decay_t<T2>>;
    template< typename T1, typename T2 > pair(T1&&, T2&&) 
        -> pair<decay_t<T1>, decay_t<T2>>;
} // namespace hsd