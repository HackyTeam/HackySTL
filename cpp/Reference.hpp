#pragma once

#include "Utility.hpp"
#include <stdexcept>

namespace hsd
{   
    template <typename T>
    class reference 
    {
    private:
        T* _ptr = nullptr;

    public:
        constexpr reference() noexcept = default;

        constexpr reference(T& value) noexcept
        {
            if(std::is_constant_evaluated())
            {
                _ptr = &value;
            }
            else
            {
                _ptr = addressof(value);
            }
        }
        
        reference(const reference&) noexcept = default;
        reference& operator=(const reference& x) noexcept = default;
    
        constexpr operator T&() const 
        { 
            if(_ptr == nullptr)
            {
                throw std::runtime_error(
                    "Dereferencing a nullptr in hsd::reference"
                );
            }

            return *_ptr; 
        }

        constexpr T& get() const
        { 
            if(_ptr == nullptr)
            {
                throw std::runtime_error(
                    "Dereferencing a nullptr in hsd::reference"
                );
            }

            return *_ptr; 
        }
    };
} // namespace hsd
