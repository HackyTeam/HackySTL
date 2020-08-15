#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "Types.hpp"
#include "InitializerList.hpp"

namespace hsd
{
    template< typename T, size_t N >
    class heap_array
    {
    private:
        T* _array = nullptr;

    public:
        using iterator = T*;
        using const_iterator = const T*;

        constexpr heap_array()
        {
            _array = new T[N];
        }

        constexpr heap_array(T* data)
        {
            _array = new T[N];
            hsd::copy(data, data + N, _array);
        }

        template< typename L, typename... U >
        constexpr heap_array(const L& value, const U&... values)
        {
            _array = new T[N];
            T arr[] = {value, values...};
            hsd::copy(arr, arr + N, begin());
        }

        constexpr heap_array(const heap_array& other)
        {
            _array = new T[N];
            hsd::copy(other.begin(), other.end(), begin());
        }

        constexpr ~heap_array()
        {
            delete[] _array;
        }

        constexpr heap_array& operator=(const heap_array& rhs)
        {
            hsd::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        constexpr heap_array& operator=(initializer_list<T>&& rhs)
        {
            if(rhs.size() != N)
            {
                throw std::out_of_range("");
            }

            hsd::move(rhs.begin(), rhs.begin() + N, _array);
            return *this;
        }

        constexpr T& operator[](size_t index)
        {
            return _array[index];
        }

        constexpr T& at(size_t index)
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        template< size_t U, size_t L >
        constexpr auto gen_range()
        {
            static_assert(L - U <= N, "Out of range\n");

            return heap_array<T, L - U>(&_array[U]);
        }

        constexpr size_t size()
        {
            return N;
        }

        constexpr iterator data()
        {
            return _array;
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr const_iterator cbegin()
        {
            return begin();
        }

        constexpr const_iterator cend()
        {
            return end();
        }
    };

    template< typename L, typename... U > heap_array(const L&, const U&...) -> heap_array<L, 1 + sizeof...(U)>;
}