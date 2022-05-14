#pragma once

#include "Allocator.hpp"

namespace hsd
{
    namespace harray_detail
    {
        struct bad_access
        {
            const char* pretty_error() const
            {
                return "Tried to access elements out of bounds";
            }
        };
        
    } // namespace harray_detail

    template <typename T, usize N>
    class heap_array
    {
    private:
        T* _array = nullptr;

    public:
        using iterator = T*;
        using const_iterator = const T*;

        inline heap_array()
        {
            _array = mallocator::allocate_multiple<T>(N).unwrap();
        }

        template <usize L> requires (L == N)
        inline heap_array(const T (&arr)[L])
        {
            _array = mallocator::allocate_multiple<T>(N).unwrap();
            copy_n(arr, N, _array);
        }

        template <usize L> requires (L == N)
        inline heap_array(T (&&arr)[L])
        {
            _array = mallocator::allocate_multiple<T>(N).unwrap();
            move(arr, arr + N, _array);
        }

        inline heap_array(const heap_array& other)
        {
            _array = mallocator::allocate_multiple<T>(N).unwrap();
            copy_n(other.data(), N, _array);
        }

        inline heap_array(heap_array&& other)
        {
            _array = other._array;
            other._array = nullptr;
        }

        inline ~heap_array()
        {
            mallocator::deallocate(_array);
        }

        inline heap_array& operator=(const heap_array& rhs)
        {
            copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        inline heap_array& operator=(heap_array&& other)
        {
            _array = other._array;
            other._array = nullptr;
            return *this;
        }

        template <usize L>
        inline heap_array& operator=(const T (&arr)[L])
        {
            copy(arr, arr + N, _array);
            return *this;
        }

        template <usize L>
        inline heap_array& operator=(T (&&arr)[L])
        {
            move(arr, arr + N, _array);
            return *this;
        }

        inline T& operator[](usize index)
        {
            return _array[index];
        }
        
        inline const T& operator[](usize index) const
        {
            return _array[index];
        }

        inline auto at(usize index) 
            -> result<reference<T>, harray_detail::bad_access>
        {
            if (index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
        }

        inline auto at(usize index) const 
            -> result<reference<const T>, harray_detail::bad_access>
        {
            if (index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
        }

        constexpr usize size() const
        {
            return N;
        }

        inline iterator data()
        {
            return _array;
        }

        inline iterator data() const
        {
            return _array;
        }

        inline iterator begin()
        {
            return data();
        }

        inline iterator begin() const
        {
            return data();
        }

        inline iterator end()
        {
            return begin() + size();
        }

        inline iterator end() const
        {
            return begin() + size();
        }

        inline const_iterator cbegin()
        {
            return begin();
        }

        inline const_iterator cbegin() const
        {
            return begin();
        }

        inline const_iterator cend()
        {
            return end();
        }

        inline const_iterator cend() const
        {
            return end();
        }
    };

    template < typename T, usize N > heap_array(const T (&)[N]) -> heap_array<T, N>;
    template < typename T, usize N > heap_array(T (&&)[N]) -> heap_array<T, N>;
}