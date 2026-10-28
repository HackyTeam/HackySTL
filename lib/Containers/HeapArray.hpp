#pragma once

/// @file HeapArray.hpp
/// @brief Fixed-size owning arrays backed by an allocator.
/// @details Constructs exactly N elements in one allocation and provides
/// checked access, iteration, copy/move operations, and allocator propagation.

#include "../Base/Allocator.hpp"
#include "../Wrappers/Reference.hpp"

namespace hsd
{
    namespace harray_detail
    {
        ///
        /// @brief Error reported by `heap_array::at` for an out-of-bounds index.
        ///
        struct bad_access
        {
            /// @brief Gets a human-readable description of this error.
            /// @return A stable diagnostic string.
            const char* pretty_error() const
            {
                return "Tried to access elements out of bounds";
            }
        };
        
    } // namespace harray_detail

    ///
    /// @brief Fixed-size owning array whose storage is provided by an allocator.
    ///
    /// Unlike `stack_array`, the elements are stored in one heap or resource
    /// allocation. The allocator always travels with that allocation.
    ///
    /// @tparam T - Element type.
    /// @tparam N - Number of elements.
    /// @tparam Allocator - Storage provider used for the element array.
    ///
    template <typename T, usize N, typename Allocator = allocator>
    /// @brief Fixed-size allocator-backed owning array.
    class heap_array
    {
    private:
        /// @brief Allocator type used for the owned storage.
        using alloc_type = Allocator;
        /// @brief Allocator-traits customization point for `alloc_type`/`T`.
        using alloc_traits = allocator_traits<alloc_type, T>;

        /// @brief Allocator instance associated with `_array`'s storage.
        alloc_type _alloc;
        /// @brief Pointer to the single allocation holding all `N` elements.
        T* _array = nullptr;

    public:
        /// @brief Allocator type used for the owned storage.
        using allocator_type = Allocator;
        /// @brief Mutable element iterator, a plain pointer to `T`.
        using iterator = T*;
        /// @brief Const element iterator, a plain pointer to `const T`.
        using const_iterator = const T*;

        ///
        /// @brief Allocates and default-constructs all elements.
        ///
        inline heap_array()
        requires (
            DefaultConstructible<alloc_type> &&
            DefaultConstructible<T>
        )
        {
            _array = alloc_traits::allocate(_alloc, N).unwrap();

            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(_alloc, &_array[_index]);
            }
        }

        ///
        /// @brief Allocates with a supplied allocator and default-constructs
        /// all elements.
        ///
        /// @param alloc - Allocator associated with the new storage.
        ///
        inline heap_array(const alloc_type& alloc)
        requires (DefaultConstructible<T>)
            : _alloc{alloc}
        {
            _array = alloc_traits::allocate(_alloc, N).unwrap();

            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(_alloc, &_array[_index]);
            }
        }

        /// @brief Delegates to the allocator-taking array constructor with a
        /// default-constructed allocator.
        /// @tparam L - Number of source elements, which must equal `N`.
        /// @param arr - Array copied into the owned storage.
        template <usize L> requires (L == N)
        inline heap_array(const T (&arr)[L])
        requires (DefaultConstructible<alloc_type>)
            : heap_array{arr, alloc_type{}}
        {}
        /// @brief Allocates with a supplied allocator and copies an array.
        ///
        /// @tparam L - Number of source elements, which must equal `N`.
        /// @param arr - Array copied into the owned storage.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize L> requires (L == N)
        inline heap_array(const T (&arr)[L], const alloc_type& alloc)
            : _alloc{alloc}
        {
            _array = alloc_traits::allocate(_alloc, N).unwrap();

            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(
                    _alloc, &_array[_index], arr[_index]
                );
            }
        }

        /// @brief Delegates to the allocator-taking array constructor with a
        /// default-constructed allocator.
        /// @tparam L - Number of source elements, which must equal `N`.
        /// @param arr - Array moved into the owned storage.
        template <usize L> requires (L == N)
        inline heap_array(T (&&arr)[L])
        requires (DefaultConstructible<alloc_type>)
            : heap_array{hsd::move(arr), alloc_type{}}
        {}

        /// @brief Allocates with a supplied allocator and moves an array.
        ///
        /// @tparam L - Number of source elements, which must equal `N`.
        /// @param arr - Array moved into the owned storage.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize L> requires (L == N)
        inline heap_array(T (&&arr)[L], const alloc_type& alloc)
            : _alloc{alloc}
        {
            _array = alloc_traits::allocate(_alloc, N).unwrap();

            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(
                    _alloc, &_array[_index], hsd::move(arr[_index])
                );
            }
        }

        ///
        /// @brief Copy-constructs a new array, selecting the source's allocator
        /// via `allocator_traits::select_on_container_copy_construction`.
        ///
        /// @param other - Array whose elements are copied.
        ///
        inline heap_array(const heap_array& other)
            : _alloc{
                alloc_traits::select_on_container_copy_construction(
                    other._alloc
                )
            }
        {
            _array = alloc_traits::allocate(_alloc, N).unwrap();

            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(
                    _alloc, &_array[_index], other[_index]
                );
            }
        }

        ///
        /// @brief Move-constructs a new array, taking ownership of `other`'s
        /// allocation and allocator.
        ///
        /// @param other - Array whose allocation is taken; left empty (its
        /// internal pointer becomes `nullptr`).
        ///
        inline heap_array(heap_array&& other)
            : _alloc{hsd::move(other._alloc)}
        {
            _array = hsd::exchange(other._array, nullptr);
        }

        ///
        /// @brief Destroys all elements and releases the owned storage.
        ///
        /// A moved-from array (`_array == nullptr`) skips destruction and
        /// deallocation is a no-op.
        ///
        inline ~heap_array()
        {
            for (usize _index = N; _array != nullptr && _index > 0; --_index)
            {
                alloc_traits::destroy(_alloc, &_array[_index - 1]);
            }

            alloc_traits::deallocate(_alloc, _array, N).unwrap();
        }

        ///
        /// @brief Copy-assigns each element from `rhs` into this array's storage.
        ///
        /// The allocator and allocation are not replaced; only element values
        /// are copied.
        ///
        /// @param rhs - Array whose elements are copied.
        /// @return A reference to this array.
        ///
        inline heap_array& operator=(const heap_array& rhs)
        {
            copy(rhs.begin(), rhs.end(), begin());
            return *this;
        }

        ///
        /// @brief Move-assigns by exchanging allocator and allocation with `other`.
        ///
        /// @param other - Array whose allocator and allocation are swapped in.
        /// @return A reference to this array.
        ///
        inline heap_array& operator=(heap_array&& other)
        {
            if (this != &other)
            {
                hsd::swap(_alloc, other._alloc);
                hsd::swap(_array, other._array);
            }

            return *this;
        }

        ///
        /// @brief Copy-assigns each element from a raw array into this storage.
        ///
        /// @tparam L - Number of source elements.
        /// @param arr - Array whose elements are copied; only the first `N`
        /// destination slots are written even when `L` differs from `N`.
        /// @return A reference to this array.
        ///
        template <usize L>
        inline heap_array& operator=(const T (&arr)[L])
        {
            copy(arr, arr + N, _array);
            return *this;
        }

        ///
        /// @brief Move-assigns each element from a raw array into this storage.
        ///
        /// @tparam L - Number of source elements.
        /// @param arr - Array whose elements are moved; only the first `N`
        /// destination slots are written even when `L` differs from `N`.
        /// @return A reference to this array.
        ///
        template <usize L>
        inline heap_array& operator=(T (&&arr)[L])
        {
            hsd::move(arr, arr + N, _array);
            return *this;
        }

        ///
        /// @brief Accesses the element at `index` without bounds checking.
        ///
        /// @param index - Zero-based element index; must be less than `N`.
        /// @return A reference to the element.
        ///
        inline T& operator[](usize index)
        {
            return _array[index];
        }
        
        ///
        /// @brief const-qualified overload of `operator[]`.
        ///
        /// @param index - Zero-based element index; must be less than `N`.
        /// @return A const reference to the element.
        ///
        inline const T& operator[](usize index) const
        {
            return _array[index];
        }

        ///
        /// @brief Accesses the element at `index` with bounds checking.
        ///
        /// @param index - Zero-based element index.
        /// @return A reference wrapper to the element, or `bad_access` when
        /// `index >= N`.
        ///
        inline auto at(usize index) 
            -> result<reference<T>, harray_detail::bad_access>
        {
            if (index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
        }

        ///
        /// @brief const-qualified overload of `at`.
        ///
        /// @param index - Zero-based element index.
        /// @return A reference wrapper to the element, or `bad_access` when
        /// `index >= N`.
        ///
        inline auto at(usize index) const 
            -> result<reference<const T>, harray_detail::bad_access>
        {
            if (index >= N)
                return harray_detail::bad_access{};

            return {_array[index]};
        }

        ///
        /// @brief Gets the fixed element count `N`.
        ///
        /// @return The fixed element count `N`.
        ///
        constexpr usize size() const
        {
            return N;
        }

        ///
        /// @brief Returns the allocator associated with this array's storage.
        ///
        /// @return A copy of the associated allocator.
        ///
        constexpr allocator_type get_allocator() const
        {
            return _alloc;
        }

        ///
        /// @brief Gets a pointer to the owned element storage.
        ///
        /// @return A pointer to the first element.
        ///
        inline iterator data()
        {
            return _array;
        }

        ///
        /// @brief const-qualified overload of `data`.
        ///
        /// @return A pointer to the first element.
        ///
        inline iterator data() const
        {
            return _array;
        }

        ///
        /// @brief Gets an iterator to the first element.
        ///
        /// @return An iterator to the first element.
        ///
        inline iterator begin()
        {
            return data();
        }

        ///
        /// @brief const-qualified overload of `begin`.
        ///
        /// @return An iterator to the first element.
        ///
        inline iterator begin() const
        {
            return data();
        }

        ///
        /// @brief Gets an iterator one past the last element.
        ///
        /// @return An iterator one past the last element.
        ///
        inline iterator end()
        {
            return begin() + size();
        }

        ///
        /// @brief const-qualified overload of `end`.
        ///
        /// @return An iterator one past the last element.
        ///
        inline iterator end() const
        {
            return begin() + size();
        }

        ///
        /// @brief Gets a const iterator to the first element.
        ///
        /// @return A const iterator to the first element.
        ///
        inline const_iterator cbegin()
        {
            return begin();
        }

        ///
        /// @brief const-qualified overload of `cbegin`.
        ///
        /// @return A const iterator to the first element.
        ///
        inline const_iterator cbegin() const
        {
            return begin();
        }

        ///
        /// @brief Gets a const iterator one past the last element.
        ///
        /// @return A const iterator one past the last element.
        ///
        inline const_iterator cend()
        {
            return end();
        }

        ///
        /// @brief const-qualified overload of `cend`.
        ///
        /// @return A const iterator one past the last element.
        ///
        inline const_iterator cend() const
        {
            return end();
        }
    };

    /// @brief Deduces `heap_array` from a copied raw array with a default allocator.
    /// @tparam T - Deduced element type.
    /// @tparam N - Deduced element count.
    /// @param arr - Array used to deduce `T` and `N`.
    template < typename T, usize N >
    heap_array(const T (&arr)[N]) -> heap_array<T, N>;

    /// @brief Deduces `heap_array` from a moved raw array with a default allocator.
    /// @tparam T - Deduced element type.
    /// @tparam N - Deduced element count.
    /// @param arr - Array used to deduce `T` and `N`.
    template < typename T, usize N >
    heap_array(T (&&arr)[N]) -> heap_array<T, N>;

    /// @brief Deduces `heap_array` from a copied raw array with an explicit allocator.
    /// @tparam T - Deduced element type.
    /// @tparam N - Deduced element count.
    /// @tparam Allocator - Deduced allocator type.
    /// @param arr - Array used to deduce `T` and `N`.
    /// @param alloc - Allocator instance used to deduce `Allocator`.
    template < typename T, usize N, typename Allocator >
    heap_array(
        const T (&arr)[N], const Allocator& alloc
    ) -> heap_array<T, N, Allocator>;

    /// @brief Deduces `heap_array` from a moved raw array with an explicit allocator.
    /// @tparam T - Deduced element type.
    /// @tparam N - Deduced element count.
    /// @tparam Allocator - Deduced allocator type.
    /// @param arr - Array used to deduce `T` and `N`.
    /// @param alloc - Allocator instance used to deduce `Allocator`.
    template < typename T, usize N, typename Allocator >
    heap_array(
        T (&&arr)[N], const Allocator& alloc
    ) -> heap_array<T, N, Allocator>;
}