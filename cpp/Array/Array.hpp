#pragma once

#include <stdio.h>
#include <utility>
#include <stdexcept>

#include "../InitializerList/InitializerList.hpp"

template< typename T, size_t N >
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
        T arr[] = {value, values...};
        std::copy(arr, arr + N, begin());
    }

    constexpr stack_array(T* data)
    {
        std::copy(data, data + N, _array);
    }

    constexpr stack_array(const stack_array& other)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    constexpr stack_array& operator=(const stack_array& rhs)
    {
        std::copy(rhs.begin(), rhs.end(), begin());
        return *this;
    }

    constexpr stack_array& operator=(initializer_list<T>&& rhs)
    {
        if(rhs.size() != N)
        {
            throw std::out_of_range("");
        }

        std::move(rhs.begin(), rhs.begin() + N, _array);
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

        return stack_array<T, L - U>(&_array[U]);
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
        std::copy(data, data + N, _array);
    }

    template< typename L, typename... U >
    constexpr heap_array(const L& value, const U&... values)
    {
        _array = new T[N];
        T arr[] = {value, values...};
        std::copy(arr, arr + N, begin());
    }

    constexpr heap_array(const heap_array& other)
    {
        _array = new T[N];
        std::copy(other.begin(), other.end(), begin());
    }

    constexpr ~heap_array()
    {
        delete[] _array;
    }

    constexpr heap_array& operator=(const heap_array& rhs)
    {
        std::copy(rhs.begin(), rhs.end(), begin());
        return *this;
    }

    constexpr heap_array& operator=(initializer_list<T>&& rhs)
    {
        if(rhs.size() != N)
        {
            throw std::out_of_range("");
        }

        std::move(rhs.begin(), rhs.begin() + N, _array);
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

template< typename L, typename... U > stack_array(const L&, const U&...) -> stack_array<L, 1 + sizeof...(U)>;
template< typename L, typename... U > heap_array(const L&, const U&...) -> heap_array<L, 1 + sizeof...(U)>;

namespace ArrTest
{
    static void Test()
    {
        stack_array sarr = {1, 2, 3, 4, 5};

        for(auto i : sarr)
        {
            printf("%d\n", i);
        }

        auto sarr2 = sarr.gen_range<0, 3>();
        puts("");

        for(auto i : sarr2)
        {
            printf("%d\n", i);
        }

        puts("");
        heap_array harr = {1, 2, 3, 4, 5};

        for(auto i : harr)
        {
            printf("%d\n", i);
        }

        auto harr2 = harr.gen_range<0, 3>();
        puts("");

        for(auto i : harr2)
        {
            printf("%d\n", i);
        }
    }
}