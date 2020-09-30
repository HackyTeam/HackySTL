#pragma once

#include "Utility.hpp"

namespace hsd
{   
    template <typename T>
    class reference 
    {
    private:
        T* _ptr;

    public:
        constexpr reference(T& value) noexcept
            : _ptr(addressof(value)) 
        {}
        
        reference(const reference&) noexcept = default;
        reference& operator=(const reference& x) noexcept = default;
    
        constexpr operator T&() const noexcept 
        { 
            return *_ptr; 
        }

        constexpr T& get() const noexcept 
        { 
            return *_ptr; 
        }
    };
} // namespace hsd
