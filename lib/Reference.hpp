#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename T>
    class reference 
    {
    private:
        T* _ptr = nullptr;

    public:
        using value_type = T;

        constexpr reference(T& value)
            : _ptr(addressof(value))
        {}
        
        constexpr reference(const reference&) = default;
        constexpr reference& operator=(const reference&) = default;

        constexpr operator T&() const 
        { 
            return *_ptr; 
        }

        constexpr T& get() const
        { 
            return *_ptr; 
        }
    };
} // namespace hsd
