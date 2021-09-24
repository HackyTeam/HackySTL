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

        constexpr reference(T& value) noexcept
        {
            _ptr = addressof(value);
        }
        
        reference(const reference&) noexcept = default;
        reference& operator=(const reference&) noexcept = default;

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
