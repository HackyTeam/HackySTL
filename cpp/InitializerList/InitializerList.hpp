#pragma once

template< typename T >
class initializer_list
{
private:
    T* _data = nullptr;
    size_t _size = 0;
public:
    using const_iterator = const T*;

    template< typename... U >
    constexpr initializer_list(const T& value, const U&... values)
    {
        _size = 1 + sizeof...(U);
        _data = new T[_size];
        T arr[] = {value, values...};
        std::copy(arr, arr + _size, _data); 
    }

    constexpr initializer_list(const initializer_list& other)
    {
        _size = other._size;
        _data = new T[_size];
        std::copy(other._data, other._data + _size, _data); 
    }

    constexpr ~initializer_list()
    {
        delete[] _data;
    }

    constexpr size_t size()
    {
        return _size;
    }

    constexpr const_iterator begin()
    {
        return _data;
    }

    constexpr const_iterator end()
    {
        return begin() + size();
    }
};

namespace InitListTest
{
    template< typename T >
    static void some(initializer_list<T> l) {}

    static void Test()
    {
        initializer_list t = {1, 2, 3, 4};

        some(t);

        for(auto i : t)
        {
            printf("%d\n", i);
        }
    }
}