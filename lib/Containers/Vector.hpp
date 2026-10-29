#pragma once

/// @file Vector.hpp
/// @brief Allocator-aware dynamic and fixed-capacity vectors.
/// @details Provides contiguous storage, growth, reserve/resize, erasure,
/// iterator access, constexpr static_vector support, and lifetime-safe moves.

#include "../Base/Allocator.hpp"
#include "Span.hpp"
#include "StackArray.hpp"
#include "../Wrappers/Reference.hpp"
#include "../Base/IntegerSequence.hpp"
#include "../Base/ReverseIterator.hpp"

namespace hsd
{
    /// @brief Allocator-aware contiguous dynamic sequence container.
    template <typename T, typename Allocator = allocator>
    class vector
    {
    private:
        /// @brief Allocator type used to obtain and release element storage.
        using alloc_type = Allocator;
        /// @brief Allocator operations used to construct and destroy elements.
        using alloc_traits = allocator_traits<alloc_type, T>;
        /// @brief Allocator instance owning the storage referenced by `_data`.
        alloc_type _alloc;
        /// @brief Owning pointer to contiguous storage, or `nullptr` when unallocated.
        T* _data = nullptr;
        /// @brief Number of constructible slots currently allocated at `_data`.
        usize _capacity = 0;

        /// @brief Moves every element into `new_buf`, leaving the old slots destroyed.
        /// @details Trivially copyable elements are copied as bytes; the
        ///          allocator traits construct with `construct_at`, so no
        ///          allocator hook is skipped. Constant evaluation keeps the
        ///          element-wise loop.
        /// @param new_buf Uninitialized storage with room for `_size` elements.
        constexpr void _relocate_into(T* new_buf)
        {
            if constexpr (is_trivially_copyable<T>::value)
            {
                if (!is_constant_evaluated())
                {
                    copy_trivial_n(_data, _size, new_buf);
                    return;
                }
            }

            for (usize _index = 0; _index < _size; ++_index)
            {
                auto& _value = at_unchecked(_index);
                alloc_traits::construct(
                    _alloc, &new_buf[_index], hsd::move(_value)
                );
                alloc_traits::destroy(_alloc, &_value);
            }
        }

    protected:
        /// @brief Number of live, constructed elements occupying `[0, _size)`.
        usize _size = 0;

        /// @brief Error tag produced by checked accessors on an out-of-bounds index.
        struct bad_access
        {
            ///
            /// @brief Returns a human-readable description of the failure.
            ///
            /// @return Pointer to a static, null-terminated error message.
            ///
            constexpr const char* pretty_error() const
            {
                return "Tried to access an element out of bounds";
            }
        };

    public:
        /// @brief Public ownership, access, iterator, and storage operations.
        using value_type = T;
        /// @brief Mutable random-access iterator; a plain pointer to `T`.
        using iterator = T*;
        /// @brief Const random-access iterator; a plain pointer to `const T`.
        using const_iterator = const T*;
        /// @brief Reverse iterator over the elements; the end sentinel never precedes the storage.
        using reverse_iterator = contiguous_reverse_iterator<iterator>;
        /// @brief Const reverse iterator over the elements.
        using const_reverse_iterator = contiguous_reverse_iterator<const_iterator>;

        /// @brief Destroys all live elements and releases the owned storage.
        constexpr ~vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                alloc_traits::destroy(_alloc, &_data[_index - 1]);
                
            alloc_traits::deallocate(_alloc, _data, _capacity).unwrap();
        }

        /// @brief Constructs an empty vector with a default-constructed allocator.
        constexpr vector() 
        requires (DefaultConstructible<alloc_type>) = default;

        ///
        /// @brief Constructs a vector of `size` value-initialized elements,
        /// using a default-constructed allocator.
        ///
        /// @param size - Number of elements to value-initialize.
        ///
        constexpr vector(usize size)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            resize(size);
        }

        ///
        /// @brief Constructs an empty vector bound to the given allocator.
        ///
        /// @param alloc - Allocator copied and used for future storage.
        ///
        constexpr vector(const alloc_type& alloc)
            : _alloc{alloc}
        {}

        ///
        /// @brief Constructs a vector of `size` value-initialized elements
        /// using the given allocator.
        ///
        /// @param size - Number of elements to value-initialize.
        /// @param alloc - Allocator copied and used for storage.
        ///
        constexpr vector(usize size, const alloc_type& alloc)
            : _alloc{alloc}
        {
            resize(size);
        }

        ///
        /// @brief Copy-constructs from another vector, allocating fresh storage
        /// sized to `other`'s capacity and copy-constructing each element.
        ///
        /// @param other - Vector whose elements are copied.
        /// @note The allocator is obtained through
        /// `allocator_traits::select_on_container_copy_construction`.
        ///
        constexpr vector(const vector& other)
            : _alloc{
                alloc_traits::select_on_container_copy_construction(
                    other._alloc
                )
            },
            _capacity{other._capacity}, 
            _size{other._size}
        {
            _data = alloc_traits
                ::allocate(_alloc, other._capacity)
                .unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                alloc_traits::construct(_alloc, &_data[_index], other[_index]);
        }

        ///
        /// @brief Move-constructs from another vector, taking ownership of its
        /// storage and allocator without reallocating or copying elements.
        ///
        /// @param other - Vector drained of its storage; left empty (size and
        /// capacity zero, data null) and safe to destroy or reassign.
        ///
        constexpr vector(vector&& other)
            : _alloc{hsd::move(other._alloc)},
            _data{hsd::exchange(other._data, nullptr)},
            _capacity{hsd::exchange(other._capacity, 0u)},
            _size{hsd::exchange(other._size, 0u)}
        {}

        ///
        /// @brief Constructs a vector by copying an array with a
        /// default-constructed allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the vector.
        ///
        template <usize N>
        constexpr vector(const T (&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : vector{arr, alloc_type{}}
        {}

        ///
        /// @brief Constructs a vector by copying an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the vector.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize N>
        constexpr vector(const T (&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}, _capacity{N}, _size{N}
        {
            _data = alloc_traits
                ::allocate(_alloc, N)
                .unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                alloc_traits::construct(_alloc, &_data[_index], arr[_index]);
        }

        ///
        /// @brief Constructs a vector by moving an array with a
        /// default-constructed allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the vector.
        ///
        template <usize N>
        constexpr vector(T (&&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : vector{hsd::move(arr), alloc_type{}}
        {}

        ///
        /// @brief Constructs a vector by moving an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the vector.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize N>
        constexpr vector(T (&&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}, _capacity{N}, _size{N}
        {
            _data = alloc_traits
                ::allocate(_alloc, N)
                .unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                alloc_traits::construct(
                    _alloc, &_data[_index], hsd::move(arr[_index])
                );
        }

        ///
        /// @brief Replaces the contents with a copy of `arr`, reusing existing
        /// storage when it already holds enough capacity.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array whose elements are copied.
        /// @return Reference to this vector.
        /// @note Reallocates (invalidating all iterators, pointers, and
        /// references) only when current capacity is smaller than `N`;
        /// otherwise existing elements are reused in place.
        ///
        template <usize N>
        constexpr vector& operator=(const T (&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                {
                    alloc_traits::construct(_alloc, &_data[_index], arr[_index]);
                }
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    _data[_index] = arr[_index];
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        alloc_traits::destroy(_alloc, &_data[_index - 1]);
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                    {
                        alloc_traits::construct(
                            _alloc, &_data[_index], arr[_index]
                        );
                    }
                }
                
                _size = N;
            }

            return *this;
        }

        ///
        /// @brief Replaces the contents by moving from `arr`, reusing existing
        /// storage when it already holds enough capacity.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array whose elements are moved from.
        /// @return Reference to this vector.
        /// @note Reallocates (invalidating all iterators, pointers, and
        /// references) only when current capacity is smaller than `N`;
        /// otherwise existing elements are reused in place.
        ///
        template <usize N>
        constexpr vector& operator=(T (&&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                {
                    alloc_traits::construct(
                        _alloc, &_data[_index], hsd::move(arr[_index])
                    );
                }
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    _data[_index] = hsd::move(arr[_index]);
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        alloc_traits::destroy(_alloc, &_data[_index - 1]);
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                    {
                        alloc_traits::construct(
                            _alloc, &_data[_index], hsd::move(arr[_index])
                        );
                    }
                }

                _size = N;
            }

            return *this;
        }

        ///
        /// @brief Copy-assigns via copy-and-swap: builds a temporary copy of
        /// `rhs` then swaps it into this vector.
        ///
        /// @param rhs - Vector whose contents are copied.
        /// @return Reference to this vector.
        /// @note Strongly exception-safe; self-assignment is a no-op.
        /// Invalidates all iterators, pointers, and references into this
        /// vector once the swap completes.
        ///
        constexpr vector& operator=(const vector& rhs)
        {
            if (this != &rhs)
            {
                vector _copy{rhs};
                hsd::swap(_alloc, _copy._alloc);
                hsd::swap(_data, _copy._data);
                hsd::swap(_size, _copy._size);
                hsd::swap(_capacity, _copy._capacity);
            }

            return *this;
        }

        ///
        /// @brief Move-assigns by swapping allocator and storage with `rhs`,
        /// so `rhs` ends up owning this vector's prior state and is destroyed
        /// with it.
        ///
        /// @param rhs - Vector whose storage is exchanged with this vector's.
        /// @return Reference to this vector.
        /// @note Invalidates all iterators, pointers, and references into
        /// this vector (they now refer to `rhs`'s prior contents).
        /// Self-assignment is a no-op.
        ///
        constexpr vector& operator=(vector&& rhs)
        {
            if (this != &rhs)
            {
                hsd::swap(_alloc, rhs._alloc);
                hsd::swap(_data, rhs._data);
                hsd::swap(_size, rhs._size);
                hsd::swap(_capacity, rhs._capacity);
            }
    
            return *this;
        }

        /// @brief Returns the allocator paired with this vector's storage.
        /// @return Copy of the allocator currently bound to this vector.
        constexpr Allocator get_allocator() const
        {
            return _alloc;
        }

        ///
        /// @brief Returns unchecked mutable element access.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Reference to the element at `index`.
        ///
        constexpr auto& operator[](usize index)
        {
            return at_unchecked(index);
        }

        ///
        /// @brief Returns unchecked const element access.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Const reference to the element at `index`.
        ///
        constexpr auto& operator[](usize index) const
        {
            return at_unchecked(index);
        }

        /// @brief Returns the first element.
        /// @return Reference to the element at index zero.
        constexpr auto& front()
        {
            return *begin();
        }

        /// @brief Returns the first element.
        /// @return Const reference to the element at index zero.
        constexpr auto& front() const
        {
            return *begin();
        }

        /// @brief Returns the last element.
        /// @return Reference to the element at index `size() - 1`.
        constexpr auto& back()
        {
            return *(begin() + size() - 1);
        }

        /// @brief Returns the last element.
        /// @return Const reference to the element at index `size() - 1`.
        constexpr auto& back() const
        {
            return *(begin() + size() - 1);
        }

        ///
        /// @brief Erases one element and returns the following iterator.
        ///
        /// @param pos - Iterator to the element to erase; must be dereferenceable.
        /// @return The iterator following the erased element on success, or
        /// `bad_access` if `pos` is out of range.
        /// @note Invalidates iterators, pointers, and references at or after
        /// `pos`.
        ///
        constexpr auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        ///
        /// @brief Erases the half-open range `[from, to)`.
        ///
        /// @param from - Start of the range to erase.
        /// @param to - End of the range to erase (exclusive).
        /// @return An iterator to the element following the erased range on
        /// success, or `bad_access` if the range is invalid.
        /// @note Invalidates iterators, pointers, and references at or after
        /// `from`.
        ///
        constexpr auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from < begin() || from > end() || 
                to < begin() || to > end() || from > to)
                return bad_access{};

            if (to == end())
            {
                auto _first_index = static_cast<usize>(from - begin());

                for (usize _index = _first_index; _index < _size; ++_index)
                {
                    alloc_traits::destroy(_alloc, &_data[_index]);
                }
    
                _size -= static_cast<usize>(to - from);

                return end();
            }
            else
            {
                usize _old_size = _size;
                _size -= static_cast<usize>(to - from);

                usize _current_pos = static_cast<usize>(from - begin());
                usize _last_pos = static_cast<usize>(to - begin());

                for (usize _index = 0; _index < _old_size - _last_pos; _index++)
                {
                    _data[_current_pos + _index] = 
                        hsd::move(_data[_last_pos + _index]);
                }

                for (usize _index = _size; _index < _old_size; ++_index)
                {
                    alloc_traits::destroy(_alloc, &_data[_index]);
                }

                return begin() + _current_pos;
            }
        }

        ///
        /// @brief Returns checked mutable element access.
        ///
        /// @param index - Zero-based element index.
        /// @return Reference to the element at `index`, or `bad_access` if
        /// `index >= size()`.
        ///
        constexpr auto at(usize index)
            -> result<reference<T>, bad_access>
        {
            if (index >= _size) return bad_access{};

            return {_data[index]};
        }

        ///
        /// @brief Returns checked const element access.
        ///
        /// @param index - Zero-based element index.
        /// @return Const reference to the element at `index`, or `bad_access`
        /// if `index >= size()`.
        ///
        constexpr auto at(usize index) const
            -> result<reference<const T>, bad_access>
        {
            if (index >= _size) return bad_access{};

            return {_data[index]};
        }

        ///
        /// @brief Returns mutable element access without bounds checking.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Reference to the element at `index`.
        /// @note Undefined behavior if `index >= size()`.
        ///
        constexpr auto& at_unchecked(usize index)
        {
            return _data[index];
        }

        ///
        /// @brief Returns const element access without bounds checking.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Const reference to the element at `index`.
        /// @note Undefined behavior if `index >= size()`.
        ///
        constexpr const auto& at_unchecked(usize index) const
        {
            return _data[index];
        }

        /// @brief Destroys all elements while retaining allocated capacity.
        constexpr void clear()
        {
            for (usize _index = _size; _index > 0; --_index)
                alloc_traits::destroy(_alloc, &_data[_index - 1]);
                
            _size = 0;
        }

        ///
        /// @brief Ensures capacity for at least `new_cap` elements.
        ///
        /// @param new_cap - Minimum required capacity.
        /// @note Reallocates and invalidates all iterators, pointers, and
        /// references if `new_cap` exceeds the current capacity; otherwise a
        /// no-op.
        ///
        constexpr void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;

                if (_new_capacity < new_cap)
                {
                    _new_capacity = pow2_ceil(new_cap);
                }

                T* _new_buf = alloc_traits
                    ::allocate(_alloc, _new_capacity)
                    .unwrap();

                _relocate_into(_new_buf);

                alloc_traits
                ::deallocate(_alloc, _data, _capacity)
                .unwrap();
                
                _data = _new_buf;
                
                _capacity = _new_capacity;
            }
        }

        /// @brief Reallocates to exactly the current size when possible.
        constexpr void shrink_to_fit()
        {
            if (_size == 0)
            {
                T* _old_buf = hsd::exchange(_data, nullptr);
                
                alloc_traits
                ::deallocate(_alloc, _old_buf, _capacity)
                .unwrap();
                
                _capacity = 0;
            }
            else if (_size < _capacity)
            {
                T* _new_buf = alloc_traits
                    ::allocate(_alloc, _size)
                    .unwrap();

                for (usize _index = 0; _index < _size; ++_index)
                {
                    alloc_traits::construct(
                        _alloc, &_new_buf[_index], hsd::move(_data[_index])
                    );
                    alloc_traits::destroy(_alloc, &_data[_index]);
                }

                alloc_traits
                ::deallocate(_alloc, _data, _capacity)
                .unwrap();
                
                _capacity = _size;
                _data = _new_buf;
            }
        }

        /// @brief Grows with value-initialized elements or destroys a suffix.
        /// @param new_size Target logical element count.
        constexpr void resize(usize new_size)
        {
            if (new_size > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;
                
                if (_new_capacity < new_size)
                {
                    _new_capacity = pow2_ceil(new_size);
                }

                T* _new_buf = alloc_traits
                    ::allocate(_alloc, _new_capacity)
                    .unwrap();
                
                _relocate_into(_new_buf);

                for (usize _index = _size; _index < new_size; ++_index)
                {
                    alloc_traits::construct(_alloc, &_new_buf[_index]);
                }

                alloc_traits::deallocate(
                    _alloc, _data, _capacity
                ).unwrap();
                _data = _new_buf;
                _capacity = _new_capacity;
                _size = new_size;
            }
            else if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                {
                    alloc_traits::construct(_alloc, &_data[_index]);
                }
                
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    alloc_traits::destroy(_alloc, &_data[_index - 1]);
                
                _size = new_size;
            }
        }

        /// @brief Appends a copied element.
        /// @param val Element copied into the vector.
        constexpr void push_back(const T& val)
        {
            emplace_back(val);
        }

        /// @brief Appends a moved element.
        /// @param val Element moved into the vector.
        constexpr void push_back(T&& val)
        {
            emplace_back(hsd::move(val));
        }

        ///
        /// @brief Constructs an element in place at the end, growing storage
        /// via `reserve` first if needed.
        ///
        /// @tparam Args - Constructor argument types forwarded to `T`.
        /// @param args - Arguments forwarded to `T`'s constructor.
        /// @note May invalidate all iterators, pointers, and references if
        /// growth reallocates (see `reserve`).
        ///
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            reserve(_size + 1);
            alloc_traits::construct(
                _alloc, &_data[_size], hsd::forward<Args>(args)...
            );
            ++_size;
        }

        /// @brief Destroys the last element when the vector is non-empty.
        constexpr void pop_back() noexcept
        {
            if(_size > 0)
            {
                alloc_traits::destroy(_alloc, &_data[_size - 1]);
                _size--;
            }
        }

        /// @brief Returns a non-owning mutable span over the elements.
        /// @return Mutable span covering the live elements.
        constexpr auto to_span()
        {
            return span<iterator>{*this};
        }

        /// @brief Returns a non-owning const span over the elements.
        /// @return Const span covering the live elements.
        constexpr auto to_span() const
        {
            return span<const_iterator>{*this};
        }

        /// @brief Returns the number of live elements.
        /// @return Current logical size.
        constexpr usize size() const
        {
            return _size;
        }

        /// @brief Returns the allocated element capacity.
        /// @return Number of allocated element slots.
        constexpr usize capacity() const
        {
            return _capacity;
        }

        /// @brief Returns a pointer to the underlying contiguous storage.
        /// @return Mutable storage pointer, possibly null when unallocated.
        constexpr iterator data()
        {
            return _data;
        }

        /// @brief Returns a mutable iterator to the first element.
        /// @return Mutable begin iterator.
        constexpr iterator begin()
        {
            return data();
        }

        /// @brief Returns a mutable iterator one past the last element.
        /// @return Mutable end iterator.
        constexpr iterator end()
        {
            return begin() + size();
        }

        /// @brief Returns a const iterator to the first element.
        /// @return Const begin iterator.
        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        /// @brief Returns a const iterator one past the last element.
        /// @return Const end iterator.
        constexpr const_iterator end() const
        {
            return cend();
        }

        /// @brief Returns a const iterator to the first element.
        /// @return Const begin iterator.
        constexpr const_iterator cbegin() const
        {
            return _data;
        }

        /// @brief Returns a const iterator one past the last element.
        /// @return Const end iterator.
        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }

        /// @brief Returns a mutable reverse iterator to the last element.
        /// @return Mutable reverse-begin iterator.
        constexpr reverse_iterator rbegin()
        {
            return reverse_iterator{end()};
        }

        /// @brief Returns a mutable reverse iterator one before the first element.
        /// @return Mutable reverse-end iterator.
        constexpr reverse_iterator rend()
        {
            return reverse_iterator{begin()};
        }

        /// @brief Returns a const reverse iterator to the last element.
        /// @return Const reverse-begin iterator.
        constexpr const_reverse_iterator rbegin() const
        {
            return crbegin();
        }

        /// @brief Returns a const reverse iterator one before the first element.
        /// @return Const reverse-end iterator.
        constexpr const_reverse_iterator rend() const
        {
            return crend();
        }

        /// @brief Returns a const reverse iterator to the last element.
        /// @return Const reverse-begin iterator.
        constexpr const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{cend()};
        }

        /// @brief Returns a const reverse iterator one before the first element.
        /// @return Const reverse-end iterator.
        constexpr const_reverse_iterator crend() const
        {
            return const_reverse_iterator{cbegin()};
        }
    };

    /// @brief Fixed-capacity contiguous vector with constexpr support.
    template <typename T, usize N>
    class static_vector
    {
    private:
        /// @brief Inline storage for up to `N` elements; no heap allocation.
        stack_array<T, N> _data;
        /// @brief Fixed element capacity, equal to `N`.
        static constexpr usize _capacity = N;

        /// @brief Error tag produced by checked accessors on an out-of-bounds index.
        struct bad_access
        {
            ///
            /// @brief Returns a human-readable description of the failure.
            ///
            /// @return Pointer to a static, null-terminated error message.
            ///
            constexpr const char* pretty_error() const
            {
                return "Tried to access an element out of bounds";
            }
        };

    protected:
        /// @brief Number of live, constructed elements occupying `[0, _size)`.
        usize _size = 0;

    public:
        /// @brief Fixed-capacity construction, access, insertion, and removal.
        using value_type = T;
        /// @brief Mutable random-access iterator; a plain pointer to `T`.
        using iterator = T*;
        /// @brief Const random-access iterator; a plain pointer to `const T`.
        using const_iterator = const T*;
        /// @brief Reverse iterator over the elements; the end sentinel never precedes the storage.
        using reverse_iterator = contiguous_reverse_iterator<iterator>;
        /// @brief Const reverse iterator over the elements.
        using const_reverse_iterator = contiguous_reverse_iterator<const_iterator>;

        /// @brief Ends the lifetime of one element.
        /// @details Trivially destructible elements need no call. An explicit call still
        /// ends their lifetime during constant evaluation, and MSVC then rejects later
        /// reads of the same storage.
        static constexpr void _destroy_element(T& element)
        {
            if constexpr (!TriviallyDestructibleType<T>)
            {
                element.~T();
            }
        }

        /// @brief Destroys all live elements; inline storage requires no deallocation.
        constexpr ~static_vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                _destroy_element(at_unchecked(_index - 1));
        }

        /// @brief Constructs an empty static_vector.
        constexpr static_vector() = default;

        ///
        /// @brief Constructs a static_vector of `size` value-initialized elements.
        ///
        /// @param size - Number of elements to value-initialize; must be `<= N`.
        /// @note Panics (via `resize`) if `size` exceeds the fixed capacity `N`.
        ///
        constexpr static_vector(usize size)
        {
            resize(size);
        }

        ///
        /// @brief Copy-constructs by copy-assigning each live element from `other`.
        ///
        /// @param other - static_vector whose elements are copied.
        ///
        constexpr static_vector(const static_vector& other)
            : _size{other._size}
        {
            for (usize _index = 0; _index < _size; ++_index)
                _data[_index] = other[_index];
        }

        ///
        /// @brief Move-constructs by swapping inline storage with `other`.
        ///
        /// @param other - static_vector whose storage is swapped in; left
        /// holding this vector's prior (default-constructed) elements and size.
        ///
        constexpr static_vector(static_vector&& other)
        {
            hsd::swap(_data, other._data);
            hsd::swap(_size, other._size);
        }

        ///
        /// @brief Copy-assigns element-wise, reusing existing slots and
        /// destroying or copy-assigning the remainder as sizes differ.
        ///
        /// @param rhs - static_vector whose elements are copied.
        /// @return Reference to this static_vector.
        ///
        constexpr static_vector& operator=(const static_vector& rhs)
        {
            usize _index;
            usize _min_size = _size < rhs._size ? _size : rhs._size;
            
            for (_index = 0; _index < _min_size; ++_index)
                _data[_index] = rhs[_index];
            
            if (_size > rhs._size)
            {
                for (_index = _size; _index > rhs._size; --_index)
                    _destroy_element(at_unchecked(_index - 1));
            }
            else if (rhs._size > _size)
            {
                for (; _index < rhs._size; ++_index)
                    _data[_index] = rhs[_index];
            }

            _size = rhs._size;

            return *this;
        }

        ///
        /// @brief Move-assigns by clearing this vector then swapping inline
        /// storage with `rhs`.
        ///
        /// @param rhs - static_vector whose storage is exchanged; left empty
        /// (size zero) afterward.
        /// @return Reference to this static_vector.
        ///
        constexpr static_vector& operator=(static_vector&& rhs)
        {
            clear();
            hsd::swap(_data, rhs._data);
            _size = hsd::exchange(rhs._size, 0u);
    
            return *this;
        }

        /// @brief Returns unchecked mutable element access.
        /// @param index Zero-based element index.
        /// @return Mutable reference to the selected element.
        constexpr auto& operator[](usize index)
        {
            return at_unchecked(index);
        }

        /// @brief Returns unchecked const element access.
        /// @param index Zero-based element index.
        /// @return Const reference to the selected element.
        constexpr auto& operator[](usize index) const
        {
            return at_unchecked(index);
        }

        /// @brief Returns the first element.
        /// @return Mutable reference to the first live element.
        constexpr auto& front()
        {
            return *begin();
        }

        /// @brief Returns the first element.
        /// @return Const reference to the first live element.
        constexpr auto& front() const
        {
            return *begin();
        }

        /// @brief Returns the last element.
        /// @return Mutable reference to the last live element.
        constexpr auto& back() noexcept
        {
            return *(begin() + size() - 1);
        }

        /// @brief Returns the last element.
        /// @return Const reference to the last live element.
        constexpr auto& back() const
        {
            return *(begin() + size() - 1);
        }

        /// @brief Erases one element and returns the following iterator.
        /// @param pos Iterator identifying the element to erase.
        /// @return Iterator following the erased element, or `bad_access`.
        constexpr auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        /// @brief Erases the half-open range `[from, to)`.
        /// @param from First iterator in the erased range.
        /// @param to Iterator one past the erased range.
        /// @return Iterator following the erased range, or `bad_access`.
        constexpr auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            if (to == end())
            {
                for (; from != end(); from++)
                    _destroy_element(*from);
    
                return end();
            }
            else
            {
                usize _old_size = _size;
                _size -= static_cast<usize>(to - from);

                usize _current_pos = static_cast<usize>(from - begin());
                usize _last_pos = static_cast<usize>(to - begin());

                for (usize _index = 0; _index < _old_size - _last_pos; _index++)
                {
                    _data[_current_pos + _index] = 
                        hsd::move(_data[_last_pos + _index]);
                }

                return begin() + _current_pos;
            }
        }

        /// @brief Returns checked mutable element access.
        /// @param index Zero-based element index.
        /// @return Mutable reference result or `bad_access`.
        constexpr auto at(usize index)
            -> result<reference<T>, bad_access>
        {
            if (index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Returns checked const element access.
        /// @param index Zero-based element index.
        /// @return Const reference result or `bad_access`.
        constexpr auto at(usize index) const
            -> result<reference<const T>, bad_access>
        {
            if (index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        ///
        /// @brief Returns mutable element access without bounds checking.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Reference to the element at `index`.
        /// @note Undefined behavior if `index >= size()`.
        ///
        constexpr auto& at_unchecked(usize index)
        {
            return _data[index];
        }

        ///
        /// @brief Returns const element access without bounds checking.
        ///
        /// @param index - Zero-based element index; must be `< size()`.
        /// @return Const reference to the element at `index`.
        /// @note Undefined behavior if `index >= size()`.
        ///
        constexpr const auto& at_unchecked(usize index) const
        {
            return _data[index];
        }

        /// @brief Destroys all elements; retained inline capacity is unaffected.
        constexpr void clear() noexcept
        {  
            _size = 0;
        }

        ///
        /// @brief Grows with value-initialized elements or destroys a suffix.
        ///
        /// @param new_size - Target element count.
        /// @note Panics if `new_size` exceeds the fixed capacity `N`.
        ///
        constexpr void resize(usize new_size)
        {
            if (new_size > _size)
            {
                if (new_size <= _capacity)
                {
                    for (usize _index = _size; _index < new_size; ++_index)
                        _data[_index] = T{};

                    _size = new_size;
                }
                else
                {
                    panic(
                        "Error at static_vector::resize(): "
                        "new size is greater than capacity.\n"
                    );
                }
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    _destroy_element(at_unchecked(_index - 1));
                
                _size = new_size;
            }
        }

        /// @brief Appends a copied element.
        /// @param val Element copied into the vector.
        constexpr void push_back(const T& val)
        {
            emplace_back(val);
        }

        /// @brief Appends a moved element.
        /// @param val Element moved into the vector.
        constexpr void push_back(T&& val)
        {
            emplace_back(hsd::move(val));
        }

        ///
        /// @brief Constructs an element in place at the end.
        ///
        /// @tparam Args - Constructor argument types forwarded to `T`.
        /// @param args - Arguments forwarded to `T`'s constructor.
        /// @note Panics if the vector is already at capacity `N`.
        ///
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if (_size + 1 <= _capacity)
            {
                _data[_size] = T{hsd::forward<Args>(args)...};
                ++_size;
            }
            else
            {
                panic(
                    "Error at static_vector:"
                    ":emplace_back(): Vector is full\n"
                );
            }
        }

        /// @brief Destroys the last element when the vector is non-empty.
        constexpr void pop_back() noexcept
        {
            if(_size > 0)
            {
                _destroy_element(at_unchecked(_size - 1));
                _size--;
            }
        }

        /// @brief Returns a non-owning mutable span over the elements.
        /// @return Mutable span covering the live elements.
        constexpr auto to_span()
        {
            return span<iterator>{*this};
        }

        /// @brief Returns a non-owning const span over the elements.
        /// @return Const span covering the live elements.
        constexpr auto to_span() const
        {
            return span<const_iterator>{*this};
        }

        /// @brief Returns the number of live elements.
        /// @return Current logical size.
        constexpr usize size() const
        {
            return _size;
        }

        /// @brief Returns the fixed element capacity `N`.
        /// @return Number of inline element slots.
        constexpr usize capacity() const
        {
            return _capacity;
        }

        /// @brief Returns a pointer to the underlying inline storage.
        /// @return Mutable inline storage pointer.
        constexpr iterator data()
        {
            return &_data[0];
        }

        /// @brief Returns a pointer to the first element of the fixed storage.
        /// @return Const pointer to the first element.
        constexpr const_iterator data() const
        {
            return &_data[0];
        }

        /// @brief Returns a mutable iterator to the first element.
        /// @return Mutable begin iterator.
        constexpr iterator begin()
        {
            return data();
        }

        /// @brief Returns a mutable iterator one past the last element.
        /// @return Mutable end iterator.
        constexpr iterator end()
        {
            return begin() + size();
        }

        /// @brief Returns a const iterator to the first element.
        /// @return Const begin iterator.
        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        /// @brief Returns a const iterator one past the last element.
        /// @return Const end iterator.
        constexpr const_iterator end() const
        {
            return cend();
        }

        /// @brief Returns a const iterator to the first element.
        /// @return Const begin iterator.
        constexpr const_iterator cbegin() const
        {
            return &_data[0];
        }

        /// @brief Returns a const iterator one past the last element.
        /// @return Const end iterator.
        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }

        /// @brief Returns a mutable reverse iterator to the last element.
        /// @return Mutable reverse-begin iterator.
        constexpr reverse_iterator rbegin()
        {
            return reverse_iterator{end()};
        }

        /// @brief Returns a mutable reverse iterator one before the first element.
        /// @return Mutable reverse-end iterator.
        constexpr reverse_iterator rend()
        {
            return reverse_iterator{begin()};
        }

        /// @brief Returns a const reverse iterator to the last element.
        /// @return Const reverse-begin iterator.
        constexpr const_reverse_iterator rbegin() const
        {
            return crbegin();
        }

        /// @brief Returns a const reverse iterator one before the first element.
        /// @return Const reverse-end iterator.
        constexpr const_reverse_iterator rend() const
        {
            return crend();
        }

        /// @brief Returns a const reverse iterator to the last element.
        /// @return Const reverse-begin iterator.
        constexpr const_reverse_iterator crbegin() const
        {
            return const_reverse_iterator{cend()};
        }

        /// @brief Returns a const reverse iterator one before the first element.
        /// @return Const reverse-end iterator.
        constexpr const_reverse_iterator crend() const
        {
            return const_reverse_iterator{cbegin()};
        }
    };

    /// @brief Deduces `vector<T>` from a copied array with a default allocator.
    /// @param _array Source array used to deduce `T`.
    template < typename T, usize N > vector(const T (&_array)[N]) -> vector<T>;
    /// @brief Deduces `vector<T>` from a moved array with a default allocator.
    /// @param _array Source array used to deduce `T`.
    template < typename T, usize N > vector(T (&&_array)[N]) -> vector<T>;

    /// @brief Deduces `vector<T, Allocator>` from a copied array and allocator.
    /// @param _array Source array used to deduce `T`.
    /// @param _alloc Allocator used to deduce `Allocator`.
    template < typename T, usize N, typename Allocator >
    vector(const T (&_array)[N], const Allocator& _alloc)
        -> vector<T, Allocator>;

    /// @brief Deduces `vector<T, Allocator>` from a moved array and allocator.
    /// @param _array Source array used to deduce `T`.
    /// @param _alloc Allocator used to deduce `Allocator`.
    template < typename T, usize N, typename Allocator >
    vector(T (&&_array)[N], const Allocator& _alloc)
        -> vector<T, Allocator>;

    /// @brief Convenience alias for a vector backed by `buffered_allocator`.
    template < typename T > using buffered_vector = vector< T, buffered_allocator >;

    ///
    /// @brief Builds a `vector<L>` from a pack of values convertible to `L`.
    ///
    /// @tparam L - Element type of the resulting vector.
    /// @tparam U - Types of the remaining arguments; each convertible to `L`.
    /// @param first - First element, determines `L` and is forwarded in place.
    /// @param rest - Remaining elements, forwarded and converted to `L`.
    /// @return A vector containing `1 + sizeof...(U)` elements in argument order.
    ///
    template < typename L, Convertible<L>... U >
    constexpr vector<L> make_vector(L&& first, U&&... rest)
    {
        constexpr usize size = 1 + sizeof...(U);
        vector<L> vec;
        vec.reserve(size);
        
        [&vec]<usize... _index>(hsd::index_sequence<_index...>, auto&... args)
        {
            (vec.emplace_back(hsd::forward<L>(args)), ...);
        }(hsd::make_index_sequence<size>{}, first, rest...);
        
        return vec;
    }
}
