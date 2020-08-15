#pragma once
/// \brief Utility helpers for the STL library

namespace hsd {
    template <typename _Type, typename... _Args>
    inline void _construct_inplace(_Type& v, _Args&&... args) {
        new (reinterpret_cast<void*>(
               &const_cast<char&>(
                   reinterpret_cast<const volatile char&>(v)))) _Type(static_cast<_Args&&>(args)...);
    }

    template <typename _Type>
    inline void _destroy_inplace(_Type& v) {
        v.~_Type();
    }
}
