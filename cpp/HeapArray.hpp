#pragma once

#include "Result.hpp"
#include "Utility.hpp"
#include "Types.hpp"

namespace hsd
{
    namespace harray_detail
    {
        struct bad_access
        {
            const char* operator()() const
            {
                return "Tried to access elements out of bounds";
            }
        };
        
    } // namespace harray_detail

    template < typename T, usize N >
    class heap_array
    {
    private:
        T* _array = nullptr;

    public:
        using iterator = T*;
        using const_iterator = const T*;

        constexpr heap_array()
        {
            _array = new T[N]{};
        }

        template <usize L>
        constexpr heap_array(const T (&arr)[L])
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{arr[Ints]...};
            }(make_index_sequence<N>{});
        }

        template <usize L>
        constexpr heap_array(T (&&arr)[L])
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{move(arr[Ints])...};
            }(make_index_sequence<N>{});
        }

        constexpr heap_array(const heap_array& other)
        {
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                _array = new T[N]{other[Ints]...};
            }(make_index_sequence<N>{});;
        }

        constexpr heap_array(heap_array&& other)
        {
            _array = other._array;
            other._array = nullptr;
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

        constexpr auto at(usize index) 
            -> Result<reference<T>, harray_detail::bad_access>
        {
            if(index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
        }

        constexpr auto at(usize index) const 
            -> Result<const reference<T>, harray_detail::bad_access>
        {
            if(index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
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

    template < typename T, usize N > heap_array(const T (&)[N]) -> heap_array<T, N>;
    template < typename T, usize N > heap_array(T (&&)[N]) -> heap_array<T, N>;
}