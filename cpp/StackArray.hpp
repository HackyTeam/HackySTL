#pragma once

#include "Result.hpp"
#include "Utility.hpp"

namespace hsd
{
    template < typename T, usize N >
    struct stack_array
    {
        T _array[N]{};
        using iterator = T*;
        using const_iterator = const T*;

        constexpr T& operator[](usize index)
        {
            return _array[index];
        }
        
        constexpr const T& operator[](usize index) const
        {
            return _array[index];
        }

        constexpr auto at(usize index)
            -> Result< reference<T>, runtime_error >
        {
            if (index >= N)
            {
                return runtime_error{"Tried to access elements out of bounds"};
            }

            return {_array[index]};
        }

        constexpr auto at(usize index) const
            -> Result< reference<const T>, runtime_error >
        {
            if (index >= N)
            {
                return runtime_error{"Tried to access elements out of bounds"};
            }

            return {_array[index]};
        }

        constexpr usize size() const
        {
            return N;
        }

        constexpr iterator data()
        {
            return _array;
        }

        constexpr const_iterator data() const
        {
            return _array;
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr const_iterator end() const
        {
            return cend();
        }

        constexpr const_iterator cbegin() const
        {
            return data();
        }

        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }
    };

    template < typename L, typename... U > stack_array(L, U...) -> stack_array<L, 1 + sizeof...(U)>;

    template <typename T, usize N>
    static constexpr void swap(stack_array<T, N>& first, stack_array<T, N>& second)
    {
        for (usize _index = 0; _index < N; _index++)
        {
            swap(first[_index], second[_index]);
        }
    }
}