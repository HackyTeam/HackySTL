#pragma once

#include <stdexcept>

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

        template <usize L>
        HSD_CONSTEXPR heap_array(const T (&arr)[L])
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{arr[Ints]...};
            }(make_index_sequence<N>{});
        }

        template <usize L>
        HSD_CONSTEXPR heap_array(T (&&arr)[L])
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{move(arr[Ints])...};
            }(make_index_sequence<N>{});
        }

        HSD_CONSTEXPR heap_array(const heap_array& other)
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{other[Ints]...};
            }(make_index_sequence<N>{});;
        }

        HSD_CONSTEXPR heap_array(heap_array&& other)
        {
            _array = other._array;
            other._array = nullptr;
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

        constexpr heap_array& operator=(heap_array&& other)
        {
            _array = other._array;
            other._array = nullptr;
            return *this;
        }

        template <usize L>
        constexpr heap_array& operator=(const T (&arr)[L])
        {
            hsd::copy(arr, arr + N, _array);
            return *this;
        }

        template <usize L>
        constexpr heap_array& operator=(T (&&arr)[L])
        {
            hsd::move(arr, arr + N, _array);
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


            return [&]<usize... Ints>(index_sequence<Ints...>)
            {
                return heap_array<T, L - U>{{_array[Ints]...}};
            }(make_index_sequence<L - U>{});
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

    template < typename T, usize N > heap_array(const T (&)[N]) -> heap_array<T, N>;
    template < typename T, usize N > heap_array(T (&&)[N]) -> heap_array<T, N>;
}