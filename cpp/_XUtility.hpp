#pragma once
/// \brief Utility helpers for the STL library

#include "Utility.hpp"

namespace hsd 
{
    template<typename _Type>
    static constexpr _Type* addressof(_Type& value)
    {
        return reinterpret_cast<_Type*>(&reinterpret_cast<char&>(value));
    }

    template <typename _Type, typename... _Args>
    static constexpr void _construct_inplace(_Type& value, _Args&&... args) 
    {
        new (addressof(value)) _Type(hsd::forward<_Args>(args)...);
    }

    template <typename _Type>
    static constexpr void _destroy_inplace(_Type& v) 
    {
        v.~_Type();
    }
}
