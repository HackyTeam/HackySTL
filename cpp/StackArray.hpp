#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "InitializerList.hpp"

namespace hsd
{
    template< typename T, usize N >
    class stack_array
    {
    private:
        T _array[N];
        
    public:
        using iterator = T*;
        using const_iterator = const T*;

        stack_array() = default;

        template< typename L, typename... U >
        constexpr stack_array(const L& value, const U&... values)
        {
            T arr[] = {static_cast<T>(value), static_cast<T>(values)...};
            hsd::copy(arr, arr + N, begin());
        }

        constexpr stack_array(T* data)
        {
            hsd::copy(data, data + N, _array);
        }

        constexpr stack_array(const stack_array& other)
        {
            hsd::copy(other._array, other._array + N, _array);
        }

        constexpr stack_array& operator=(const stack_array& rhs)
        {
            hsd::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        constexpr stack_array& operator=(initializer_list<T>&& rhs)
        {
            if(rhs.size() != N)
            {
                throw std::out_of_range("");
            }

            hsd::move(rhs.begin(), rhs.begin() + N, _array);
            return *this;
        }

        constexpr T& operator[](usize index)
        {
            return _array[index];
        }
        
        constexpr T& operator[](usize index) const
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

        template< usize U, usize L >
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

    template< typename L, typename... U > stack_array(const L&, const U&...) -> stack_array<L, 1 + sizeof...(U)>;
}