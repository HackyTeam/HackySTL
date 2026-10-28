#pragma once

/// @file StackArray.hpp
/// @brief Fixed-size inline arrays with checked access.
/// @details Stores N values directly in the object and provides fill, data,
/// iteration, swapping, and result-based bounds checking.

#include "../Base/Result.hpp"
#include "../Wrappers/Reference.hpp"

namespace hsd
{
    /// @brief Fixed-size inline array with checked access and fill.
    /// @tparam T Element type stored inline in the array.
    /// @tparam N Number of elements; fixed at compile time.
    template <typename T, usize N>
    struct stack_array
    {
        /// @brief Inline element storage, value-initialized by default.
        T _array[N]{};

        /// @brief Mutable random-access iterator type (a raw pointer).
        using iterator = T*;

        /// @brief Read-only random-access iterator type (a raw pointer).
        using const_iterator = const T*;

        /// @brief Accesses an element without bounds checking.
        /// @param index Zero-based element index; caller must ensure it is
        /// less than N.
        /// @return Reference to the element at `index`.
        constexpr T& operator[](usize index)
        {
            return _array[index];
        }
        
        /// @brief Const-qualified overload of operator[]().
        /// @param index Zero-based element index; caller must ensure it is
        /// less than N.
        /// @return Const reference to the element at `index`.
        constexpr const T& operator[](usize index) const
        {
            return _array[index];
        }

        /// @brief Accesses an element with bounds checking.
        /// @param index Zero-based element index.
        /// @return A reference wrapper to the element on success, or an
        /// error if `index` is out of bounds.
        constexpr auto at(usize index)
            -> result<reference<T>, runtime_error>
        {
            if (index >= N)
            {
                return runtime_error{"Tried to access elements out of bounds"};
            }

            return {_array[index]};
        }

        /// @brief Const-qualified overload of at().
        /// @param index Zero-based element index.
        /// @return A const reference wrapper to the element on success, or
        /// an error if `index` is out of bounds.
        constexpr auto at(usize index) const
            -> result<reference<const T>, runtime_error>
        {
            if (index >= N)
            {
                return runtime_error{"Tried to access elements out of bounds"};
            }

            return {_array[index]};
        }

        /// @brief Assigns the same value to every element.
        /// @param value Value copied into each of the N elements.
        constexpr void fill(const T& value)
        requires (CopyConstructible<T>)
        {
            for (hsd::usize idx = 0; idx < N; ++idx)
            {
                _array[idx] = value;
            }
        }

        /// @brief Gets the fixed element count.
        /// @return N, the compile-time array size.
        static constexpr usize size()
        {
            return N;
        }

        /// @brief Gets a pointer to the underlying storage.
        /// @return Mutable pointer to the first element.
        constexpr iterator data()
        {
            return _array;
        }

        /// @brief Const-qualified overload of data().
        /// @return Const pointer to the first element.
        constexpr const_iterator data() const
        {
            return _array;
        }

        /// @brief Gets an iterator to the first element.
        /// @return Mutable iterator equal to data().
        constexpr iterator begin()
        {
            return data();
        }

        /// @brief Const-qualified overload of begin().
        /// @return Const iterator equal to cbegin().
        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        /// @brief Gets an iterator one past the last element.
        /// @return Mutable iterator equal to begin() plus size().
        constexpr iterator end()
        {
            return begin() + size();
        }

        /// @brief Const-qualified overload of end().
        /// @return Const iterator equal to cend().
        constexpr const_iterator end() const
        {
            return cend();
        }

        /// @brief Gets a const iterator to the first element.
        /// @return Const iterator equal to data().
        constexpr const_iterator cbegin() const
        {
            return data();
        }

        /// @brief Gets a const iterator one past the last element.
        /// @return Const iterator equal to cbegin() plus size().
        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }
    };

    /// @brief Deduces the element type and count from an in-place element
    /// list, e.g. `stack_array{1, 2, 3}`.
    /// @tparam L Type of the first element, used as the deduced element type.
    /// @tparam U Types of the remaining elements.
    template < typename L, typename... U > stack_array(L, U...) -> stack_array<L, 1 + sizeof...(U)>;

    /// @brief Exchanges the contents of two same-sized stack_array objects.
    /// @tparam T Element type of both arrays.
    /// @tparam N Element count of both arrays.
    /// @param first First array to swap.
    /// @param second Second array to swap.
    /// @details Swaps element-by-element via the generic hsd::swap; both
    /// arrays share the same fixed size, so no reallocation can occur.
    template <typename T, usize N>
    static constexpr void swap(stack_array<T, N>& first, stack_array<T, N>& second)
    {
        for (usize _index = 0; _index < N; _index++)
        {
            hsd::swap(first[_index], second[_index]);
        }
    }
}