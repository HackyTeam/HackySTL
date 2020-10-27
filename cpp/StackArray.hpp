#pragma once

#include <stdexcept>

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

        constexpr T& at(usize index)
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        constexpr T& at(usize index) const
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        template < usize U, usize L >
        constexpr auto gen_range()
        {
            static_assert(L - U <= N, "Out of range\n");

            return stack_array<T, L - U>(&_array[U]);
        }

        constexpr usize size()
        {
            return N;
        }

        constexpr usize size() const
        {
            return N;
        }

        constexpr iterator data()
        {
            return _array;
        }

        constexpr iterator data() const
        {
            return _array;
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr iterator begin() const
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr iterator end() const
        {
            return begin() + size();
        }

        constexpr const_iterator cbegin()
        {
            return begin();
        }

        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend()
        {
            return end();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }
    };

    template < typename L, typename... U > stack_array(L, U...) -> stack_array<L, 1 + sizeof...(U)>;
}