#pragma once

#include "Types.hpp"

namespace hsd {
    template <size_t _Size, size_t _Alignment>
    struct aligned_storage {
        struct alignas(_Alignment) type {
            unsigned char _data[_Size];
        };
    };
}
