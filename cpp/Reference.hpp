#pragma once

#include "Utility.hpp"
#include "Result.hpp"

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
        reference& operator=(const reference&) noexcept = delete;

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
