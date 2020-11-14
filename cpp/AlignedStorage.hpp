#pragma once

#include "Types.hpp"

namespace hsd 
{
    template < usize _Size, usize _Alignment >
    struct aligned_storage 
    {
        struct alignas(_Alignment) type 
        {
            uchar _data[_Size];
        };
    };
}
