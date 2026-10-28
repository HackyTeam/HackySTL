#pragma once

/// @file Reference.hpp
/// @brief Lightweight non-owning references used in result values.
/// @details Wraps an existing object without extending its lifetime and exposes
/// pointer/reference access for APIs that return references through results.

#include "../Base/Utility.hpp"

namespace hsd
{
    /// @brief Non-owning reference wrapper used in result-returning APIs.
    template <typename T>
    class reference
    {
    private:
        /// @brief Non-owning pointer to the referenced object.
        T* _ptr = nullptr;

    public:
        /// @brief Referenced value type.
        using value_type = T;

        /// @brief Binds the wrapper to an existing object.
        /// @param value Object whose lifetime must exceed this reference.
        constexpr reference(T& value)
            : _ptr(hsd::addressof(value))
        {}
        
        /// @brief Copies the referenced address without taking ownership.
        constexpr reference(const reference&) = default;
        /// @brief Copies the referenced address without taking ownership.
        /// @return Reference to this object.
        constexpr reference& operator=(const reference&) = default;

        /// @brief Converts to the referenced object.
        /// @return The referenced object.
        constexpr operator T&() const 
        { 
            return *_ptr; 
        }

        /// @brief Returns the referenced object.
        /// @return The referenced object.
        constexpr T& get() const
        { 
            return *_ptr; 
        }

        /// @brief Returns the underlying non-owning pointer.
        /// @return Pointer to the referenced object.
        constexpr T* get_ptr() const
        { 
            return _ptr; 
        }
    };
} // namespace hsd
