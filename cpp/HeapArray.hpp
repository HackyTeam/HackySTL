#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "Types.hpp"
#include "InitializerList.hpp"

namespace hsd
{
    template< typename T, usize N >
    class heap_array
    {
    private:
        T* _array = nullptr;

    public:
        using iterator = T*;
        using const_iterator = const T*;

        HSD_CONSTEXPR heap_array()
        {
            _array = new T[N];
        }

        HSD_CONSTEXPR heap_array(T* data)
        {
            _array = new T[N];
            hsd::copy(data, data + N, _array);
        }

        template< typename L, typename... U >
        HSD_CONSTEXPR heap_array(const L& value, const U&... values)
        {
            _array = new T[N];
            T arr[] = {value, values...};
            hsd::copy(arr, arr + N, begin());
        }

        HSD_CONSTEXPR heap_array(const heap_array& other)
        {
            _array = new T[N];
            hsd::copy(other.begin(), other.end(), begin());
        }

        HSD_CONSTEXPR ~heap_array()
        {
            delete[] _array;
        }

        HSD_CONSTEXPR heap_array& operator=(const heap_array& rhs)
        {
            hsd::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        HSD_CONSTEXPR heap_array& operator=(initializer_list<T>&& rhs)
        {
            if(rhs.size() != N)
            {
                throw std::out_of_range("");
            }

            hsd::move(rhs.begin(), rhs.begin() + N, _array);
            return *this;
        }

        HSD_CONSTEXPR T& operator[](usize index)
        {
            return _array[index];
        }
        
        HSD_CONSTEXPR T& operator[](usize index) const
        {
            return _array[index];
        }

        HSD_CONSTEXPR T& at(usize index)
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        HSD_CONSTEXPR T& at(usize index) const
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        template< usize U, usize L >
        HSD_CONSTEXPR auto gen_range()
        {
            static_assert(L - U <= N, "Out of range\n");

            return heap_array<T, L - U>(&_array[U]);
        }

        HSD_CONSTEXPR usize size()
        {
            return N;
        }

        HSD_CONSTEXPR usize size() const
        {
            return N;
        }

        HSD_CONSTEXPR iterator data()
        {
            return _array;
        }

        HSD_CONSTEXPR iterator data() const
        {
            return _array;
        }

        HSD_CONSTEXPR iterator begin()
        {
            return data();
        }

        HSD_CONSTEXPR iterator begin() const
        {
            return data();
        }

        HSD_CONSTEXPR iterator end()
        {
            return begin() + size();
        }

        HSD_CONSTEXPR iterator end() const
        {
            return begin() + size();
        }

        HSD_CONSTEXPR const_iterator cbegin()
        {
            return begin();
        }

        HSD_CONSTEXPR const_iterator cbegin() const
        {
            return begin();
        }

        HSD_CONSTEXPR const_iterator cend()
        {
            return end();
        }

        HSD_CONSTEXPR const_iterator cend() const
        {
            return end();
        }
    };

    template< typename L, typename... U > heap_array(const L&, const U&...) -> heap_array<L, 1 + sizeof...(U)>;
}