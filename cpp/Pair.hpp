#pragma once

#include "Utility.hpp"

namespace hsd
{
    template< typename T1, typename T2 >
    struct pair
    {
        T1 first;
        T2 second;

        HSD_CONSTEXPR bool operator==(const pair& rhs)
        {
            return first == rhs.first && second == rhs.second;
        }
    };
} // namespace hsd