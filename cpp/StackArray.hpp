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
        HSD_CONSTEXPR stack_array(const L& value, const U&... values)
        {
            T arr[] = {static_cast<T>(value), static_cast<T>(values)...};
            hsd::copy(arr, arr + N, begin());
        }

        HSD_CONSTEXPR stack_array(T* data)
        {
            hsd::copy(data, data + N, _array);
        }

        HSD_CONSTEXPR stack_array(const stack_array& other)
        {
            hsd::copy(other._array, other._array + N, _array);
        }

        HSD_CONSTEXPR stack_array& operator=(const stack_array& rhs)
        {
            hsd::copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        HSD_CONSTEXPR stack_array& operator=(initializer_list<T>&& rhs)
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

            return stack_array<T, L - U>(&_array[U]);
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

    template< typename L, typename... U > stack_array(const L&, const U&...) -> stack_array<L, 1 + sizeof...(U)>;
}