#pragma once

#include "Utility.hpp"

namespace hsd
{
    template< typename T1, typename T2 >
    struct pair
    {
        T1 first;
        T2 second;

        constexpr pair() {}

        constexpr pair(const T1& _first, const T2& _second)
            : first{_first}, second{_second}
        {}

        constexpr pair(T1&& _first, T2&& _second)
            : first{hsd::move(_first)}, second{hsd::move(_second)}
        {}

        constexpr bool operator==(const pair& rhs)
        {
            return first == rhs.first && second == rhs.second;
        }
    };
} // namespace hsd