#pragma once

#include <stdexcept>
#include <initializer_list>

#include "Utility.hpp"
#include "Types.hpp"

namespace hsd
{
    template < typename T, usize N >
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

        template < typename L, typename... U >
        HSD_CONSTEXPR heap_array(const L& value, const U&... values)
        {
            _array = new T[N]{value, values...};
        }

        template < typename L, typename... U >
        HSD_CONSTEXPR heap_array(L&& value, U&&... values)
        {
            _array = new T[N]{move(value), move(values)...};
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

        constexpr heap_array& operator=(const heap_array& rhs)
        {
            hsd::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        constexpr heap_array& operator=(const std::initializer_list<T>& rhs)
        {
            if(rhs.size() != N)
            {
                throw std::out_of_range("");
            }

            hsd::copy(rhs.begin(), rhs.begin() + N, _array);
            return *this;
        }

        constexpr heap_array& operator=(std::initializer_list<T>&& rhs)
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

        constexpr const T& at(usize index) const
        {
            if(index >= N)
            {
                throw std::out_of_range("");
            }

            return _array[index];
        }

        template < usize U, usize L >
        HSD_CONSTEXPR auto gen_range()
        {
            static_assert(L - U <= N, "Out of range\n");

            return heap_array<T, L - U>(&_array[U]);
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

    template < typename L, typename... U > heap_array(L, U...) -> heap_array<L, 1 + sizeof...(U)>;
}