#pragma once

/// @file Deque.hpp
/// @brief Allocator-aware circular contiguous double-ended queue.
/// @details Owns one contiguous allocation while tracking logical front and
/// back positions, preserving element lifetimes through growth and erasure.

#include "../Base/Allocator.hpp"
#include "../Base/ReverseIterator.hpp"
#include "../Wrappers/Reference.hpp"

namespace hsd 
{
    /// @brief Allocator-aware circular double-ended sequence container.
    /// @details Logical index zero begins at `_front`; iterators map logical
    /// positions through the contiguous allocation and wrap at `_capacity`.
    /// @tparam T Element type.
    /// @tparam Allocator Allocator used for element storage.
    template <typename T, typename Allocator = allocator>
    class deque
    {
    private:
        /// @brief Allocator type retained by the container.
        using alloc_type = Allocator;
        /// @brief Allocator operations for element lifetime.
        using alloc_traits = allocator_traits<alloc_type, T>;
        /// @brief Allocator state used for all element storage.
        alloc_type _alloc;
        /// @brief Contiguous allocation containing the ring buffer.
        T* _data = nullptr;
        /// @brief Number of constructed element slots available.
        usize _capacity = 0;
        /// @brief Number of live elements in logical order.
        usize _size = 0;

        static constexpr usize _npos = ~static_cast<usize>(0);

        static constexpr usize top_bit = ~(_npos >> 1);

        /// @brief Selects a geometric power-of-two capacity holding `required`.
        static constexpr usize _growth_capacity(usize current, usize required)
        {
            usize capacity = current ? pow2_ceil(current) : 1;
            const usize max_capacity = _npos;

            while (capacity < required)
            {
                if (capacity > max_capacity / 2)
                {
                    return required;
                }

                capacity *= 2;
            }

            return capacity;
        }

        /// @brief Iterator translating logical positions into ring addresses.
        class iterator
        {
        private:
            /// @brief Ring allocation base address.
            T* _data = nullptr;
            /// @brief Logical iterator position.
            usize _current = 0;
            /// @brief Ring capacity used for address wrapping.
            usize _capacity = 0;
            friend class deque<T, Allocator>;

            /// @brief Returns the iterator one logical position earlier.
            /// @return Iterator at the previous logical position.
            constexpr iterator _get_previous() const
            {
                return {_data, _current - 1, _capacity};
            }

            /// @brief Resolves the current logical position to storage.
            /// @return Mutable element address.
            constexpr T* _get_address()
            {
                // Capacity is a power of two, so the mask also wraps the
                // unsigned underflow produced by stepping before position 0.
                return (_data + (_current & (_capacity - 1)));
            }

            /// @brief Resolves the current logical position for const access.
            /// @return Const element address.
            constexpr const T* _get_address() const
            {
                return (_data + (_current & (_capacity - 1)));
            }

            /// @brief Constructs a ring iterator from storage metadata.
            /// @param data Ring allocation base.
            /// @param current Logical position.
            /// @param capacity Ring capacity.
            constexpr iterator(T* data, usize current, usize capacity)
                : _data{data}, _current{current}, _capacity{capacity}
            {}

            /// @brief Returns the logical distance between iterators.
            /// @param rhs Iterator subtracted from this iterator.
            /// @return Logical position difference.
            usize operator-(const iterator rhs) const
            {
                return _current - rhs._current;
            }

        public:
            constexpr iterator() = default;

            /// @brief Compares iterator storage and logical position.
            /// @param lhs Left iterator.
            /// @param rhs Right iterator.
            /// @return True when both identify the same logical position.
            constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return (
                    lhs._data == rhs._data && 
                    lhs._current == rhs._current
                );
            }

            /// @brief Tests iterator inequality.
            /// @param lhs Left iterator.
            /// @param rhs Right iterator.
            /// @return True when logical positions differ.
            constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return !(lhs == rhs);
            }

            /// @brief Advances to the next logical position.
            /// @return Reference to this iterator after advancing.
            constexpr auto& operator++()
            {
                _current++;
                return *this;
            }

            /// @brief Retreats to the previous logical position.
            /// @return Reference to this iterator after retreating.
            constexpr auto& operator--()
            {
                _current--;
                return *this;
            }

            /// @brief Returns the current iterator and then advances it.
            /// @return Iterator value before advancing.
            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            /// @brief Returns the current iterator and then retreats it.
            /// @return Iterator value before retreating.
            constexpr iterator operator--(i32)
            {
                iterator tmp = *this;
                operator--();
                return tmp;
            }

            /// @brief Dereferences the current mutable element.
            /// @return Mutable reference to the current element.
            constexpr auto& operator*()
            {
                return *_get_address();
            }

            /// @brief Dereferences the current element for const access.
            /// @return Const reference to the current element.
            constexpr const auto& operator*() const
            {
                return *_get_address();
            }

            /// @brief Returns the current mutable element address.
            /// @return Mutable current element pointer.
            constexpr auto* operator->()
            {
                return _get_address();
            }

            /// @brief Returns the current element address for const access.
            /// @return Const current element pointer.
            constexpr const auto* operator->() const
            {
                return _get_address();
            }
        };

        /// @brief Error returned for an invalid deque position.
        struct bad_access
        {
            /// @brief Returns the invalid-position diagnostic.
            /// @return A stable diagnostic string.
            constexpr const char* pretty_error() const
            {
                return "Tried to access an element out of bounds";
            }
        };

        /// @brief Iterator at the logical first element.
        iterator _front = {};
        /// @brief Iterator one position past the logical last element.
        iterator _back = {};

    public:
        /// @brief Element type stored by the deque.
        using value_type = T;
        /// @brief Const iterator over logical element order.
        using const_iterator = const iterator;
        /// @brief Mutable reverse iterator, from the last element toward the first.
        using reverse_iterator = reverse_bidirectional_iterator<iterator>;
        /// @brief Const reverse iterator, from the last element toward the first.
        using const_reverse_iterator = reverse_bidirectional_iterator<const_iterator>;

        constexpr ~deque()
        {
            for (auto _idx = begin(); _idx != end(); ++_idx)
                alloc_traits::destroy(_alloc, _idx._get_address());

            alloc_traits::deallocate(_alloc, _data, _capacity).unwrap();
        }

        constexpr deque()
        requires (DefaultConstructible<alloc_type>) = default;

        /// @brief Constructs a deque with `size` default-constructed elements.
        /// @param size Initial logical size.
        constexpr deque(usize size)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            resize(size);
        }

        /// @brief Constructs an empty deque using `alloc`.
        /// @param alloc Allocator copied into the container.
        constexpr deque(const alloc_type& alloc)
            : _alloc{alloc}
        {}

        /// @brief Constructs a sized deque using `alloc`.
        /// @param size Initial logical size.
        /// @param alloc Allocator copied into the container.
        constexpr deque(usize size, const alloc_type& alloc)
            : _alloc{alloc}
        {
            resize(size);
        }

        /// @brief Copies storage values and selects a copy-construction allocator.
        /// @param other Deque whose elements are copied.
        constexpr deque(const deque& other)
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

            _front = {_data, 0u, _capacity};
            _back = {_data, _size, _capacity};
        }

        /// @brief Transfers allocation and ring state from `other`.
        /// @param other Deque whose storage is moved.
        constexpr deque(deque&& other)
            : _alloc{hsd::move(other._alloc)},
            _data{hsd::exchange(other._data, nullptr)},
            _capacity{hsd::exchange(other._capacity, 0u)},
            _size{hsd::exchange(other._size, 0u)},
            _front{hsd::exchange(other._front, iterator{})},
            _back{hsd::exchange(other._back, iterator{})}
        {}

        /// @brief Constructs a deque from a copied array with a default allocator.
        /// @param arr Source array copied into the deque.
        template <usize N>
        constexpr deque(const T (&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : deque{arr, alloc_type{}}
        {}

        ///
        /// @brief Constructs a deque by copying an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the deque.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize N>
        constexpr deque(const T (&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}, _capacity{pow2_ceil(N)}, _size{N}
        {
            _data = alloc_traits
                ::allocate(_alloc, _capacity)
                .unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                alloc_traits::construct(_alloc, &_data[_index], arr[_index]);

            _front = {_data, 0u, _capacity};
            _back = {_data, _size, _capacity};
        }

        /// @brief Constructs a deque from a moved array with a default allocator.
        /// @param arr Source array moved into the deque.
        template <usize N>
        constexpr deque(T (&&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : deque{hsd::move(arr), alloc_type{}}
        {}

        ///
        /// @brief Constructs a deque by moving an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the deque.
        /// @param alloc - Allocator associated with the new storage.
        ///
        template <usize N>
        constexpr deque(T (&&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}, _capacity{pow2_ceil(N)}, _size{N}
        {
            _data = alloc_traits
                ::allocate(_alloc, _capacity)
                .unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                alloc_traits::construct(
                    _alloc, &_data[_index], hsd::move(arr[_index])
                );

            _front = {_data, 0u, _capacity};
            _back = {_data, _size, _capacity};
        }

        /// @brief Copy-assigns values from an array.
        /// @param arr Source array copied into the deque.
        /// @return `*this` after replacement.
        template <usize N>
        constexpr deque& operator=(const T (&arr)[N])
        {
            clear();

            if (_capacity < N)
            {
                reserve(N);
            }
            
            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(_alloc, &_data[_index], arr[_index]);
            }
                
            _size = N;
            _front = {_data, 0u, _capacity};
            _back = {_data, _size, _capacity};

            return *this;
        }

        /// @brief Move-assigns values from an array.
        /// @param arr Source array moved into the deque.
        /// @return `*this` after replacement.
        template <usize N>
        constexpr deque& operator=(T (&&arr)[N])
        {
            clear();

            if (_capacity < N)
            {
                reserve(N);
            }
            
            for (usize _index = 0; _index < N; ++_index)
            {
                alloc_traits::construct(
                    _alloc, &_data[_index], hsd::move(arr[_index])
                );
            }
                
            _size = N;
            _front = {_data, 0u, _capacity};
            _back = {_data, _size, _capacity};

            return *this;
        }

        /// @brief Copy-assigns elements and allocator/storage state.
        /// @param rhs Deque whose elements are copied.
        /// @return `*this` after replacement.
        constexpr deque& operator=(const deque& rhs)
        {
            if (this != &rhs)
            {
                deque _copy{rhs};
                hsd::swap(_alloc, _copy._alloc);
                hsd::swap(_data, _copy._data);
                hsd::swap(_capacity, _copy._capacity);
                hsd::swap(_size, _copy._size);
                hsd::swap(_front, _copy._front);
                hsd::swap(_back, _copy._back);
            }

            return *this;
        }

        /// @brief Move-assigns allocator and ring storage state.
        /// @param rhs Deque whose storage is moved.
        /// @return `*this` after replacement.
        constexpr deque& operator=(deque&& rhs)
        {
            if (this != &rhs)
            {
                hsd::swap(_alloc, rhs._alloc);
                hsd::swap(_data, rhs._data);
                hsd::swap(_capacity, rhs._capacity);
                hsd::swap(_size, rhs._size);
                hsd::swap(_front, rhs._front);
                hsd::swap(_back, rhs._back);
            }

            return *this;
        }

        /// @brief Changes logical size, allocating or destroying elements as needed.
        /// @param new_size Requested logical size.
        constexpr void resize(usize new_size)
        {
            if (new_size > _capacity)
            {
                const usize _new_capacity = _growth_capacity(
                    _capacity, new_size
                );

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
                _front = {_data, 0u, _capacity};
                _back = {_data, _size, _capacity};
            }
            else if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                {
                    alloc_traits::construct(
                        _alloc, hsd::addressof(at_unchecked(_index))
                    );
                    ++_back;
                }
                
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                {
                    --_back;
                    alloc_traits::destroy(
                        _alloc, hsd::addressof(at_unchecked(_index - 1))
                    );
                }
                
                _size = new_size;
            }
        }

        /// @brief Moves the elements, in order, to the front of `new_buf`.
        /// @details The old slots are left destroyed. Trivially copyable
        ///          elements sit in at most two contiguous runs of the ring,
        ///          so they move with two byte copies; constant evaluation
        ///          and other types move element by element.
        /// @param new_buf Uninitialized storage with room for `_size` elements.
        constexpr void _relocate_into(T* new_buf)
        {
            if constexpr (__is_trivially_copyable(T))
            {
                if (!is_constant_evaluated())
                {
                    if (_size != 0)
                    {
                        const usize _start = _front._current & (_capacity - 1);
                        const usize _first = _capacity - _start < _size
                            ? _capacity - _start : _size;

                        copy_trivial_n(_data + _start, _first, new_buf);

                        if (_first < _size)
                        {
                            copy_trivial_n(_data, _size - _first, new_buf + _first);
                        }
                    }

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

        /// @brief Ensures capacity for at least `new_cap` elements.
        /// @param new_cap Minimum requested capacity.
        constexpr void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                const usize _new_capacity = _growth_capacity(
                    _capacity, new_cap
                );

                T* _new_buf = alloc_traits
                    ::allocate(_alloc, _new_capacity)
                    .unwrap();

                _relocate_into(_new_buf);

                alloc_traits
                ::deallocate(_alloc, _data, _capacity)
                .unwrap();
                
                _data = _new_buf;
                _capacity = _new_capacity;
                _front = {_data, 0u, _capacity};
                _back = {_data, _size, _capacity};
            }
        }

        /// @brief Erases the half-open range `[from, to)`.
        /// @param from First iterator erased.
        /// @param to Iterator one past the erased range.
        /// @return Iterator to the next logical element, or `bad_access`.
        constexpr auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from._data < _front._data || to._data > _back._data || (to - from) > _size)
                return bad_access{};

            iterator _from_it = from;
            iterator _to_it = to;

            // Elements after `to` shift down over the erased range.
            for (; _to_it != _back; ++_to_it, ++_from_it)
            {
                *_from_it = hsd::move(*_to_it);
            }

            // The slots from the new end to the old end hold moved-from values.
            // Destroy them so that nothing they own is leaked.
            const iterator _new_back = _from_it;

            for (iterator _slot = _new_back; _slot != _back; ++_slot)
            {
                alloc_traits::destroy(_alloc, hsd::addressof(*_slot));
            }

            _size -= static_cast<usize>(to - from);
            _back = _new_back;
            return from;
        }

        /// @brief Erases one element at `pos`.
        /// @param pos Iterator identifying the erased element.
        /// @return Iterator to the next element, or `bad_access`.
        constexpr auto erase(const_iterator pos)
        {
            iterator _pos = pos;
            return erase_for(pos, ++_pos);
        }

        /// @brief Destroys all elements while retaining allocated capacity.
        constexpr void clear()
        {
            for (usize _index = _size; _index > 0; --_index)
                alloc_traits::destroy(
                    _alloc, hsd::addressof(at_unchecked(_index - 1))
                );
                
            _front._current = 0;
            _back._current = 0;
            _size = 0;
        }

        /// @brief Constructs an element at the logical back.
        /// @param args Arguments forwarded to the element constructor.
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            reserve(_size + 1);
            alloc_traits::construct(
                _alloc, _back._get_address(), hsd::forward<Args>(args)...
            );
            ++_size;
            ++_back;
        }

        /// @brief Constructs an element at the logical front.
        /// @param args Arguments forwarded to the element constructor.
        template <typename... Args>
        constexpr void emplace_front(Args&&... args)
        {
            reserve(_size + 1);
            alloc_traits::construct(
                _alloc, (--_front)._get_address(), hsd::forward<Args>(args)...
            );
            ++_size;
        }

        /// @brief Copies an element to the logical back.
        /// @param val Element copied into the deque.
        constexpr void push_back(const T& val)
        {
            emplace_back(val);
        }

        /// @brief Moves an element to the logical back.
        /// @param val Element moved into the deque.
        constexpr void push_back(T&& val)
        {
            emplace_back(hsd::move(val));
        }

        /// @brief Copies an element to the logical front.
        /// @param val Element copied into the deque.
        constexpr void push_front(const T& val)
        {
            emplace_front(val);
        }

        /// @brief Moves an element to the logical front.
        /// @param val Element moved into the deque.
        constexpr void push_front(T&& val)
        {
            emplace_front(hsd::move(val));
        }

        /// @brief Destroys the logical back element when non-empty.
        constexpr void pop_back() noexcept
        {
            if(_size > 0)
            {
                alloc_traits::destroy(
                    _alloc, hsd::addressof(at_unchecked(_size - 1))
                );
                --_back;
                --_size;
            }
        }

        /// @brief Destroys the logical front element when non-empty.
        constexpr void pop_front() noexcept
        {
            if(_size > 0)
            {
                alloc_traits::destroy(
                    _alloc, hsd::addressof(at_unchecked(0))
                );
                ++_front;
                --_size;
            }
        }

        /// @brief Returns the number of live logical elements.
        /// @return Logical deque size.
        constexpr usize size() const
        {
            return _size;
        }

        /// @brief Returns the allocated ring capacity.
        /// @return Number of element slots allocated.
        constexpr usize capacity() const
        {
            return _capacity;
        }

        /// @brief Returns the allocator associated with storage.
        /// @return Copy of the container allocator.
        constexpr Allocator get_allocator() const
        {
            return _alloc;
        }

        /// @brief Returns unchecked mutable access by logical index.
        /// @param index Zero-based logical index.
        /// @return Mutable element reference; caller must provide a valid index.
        constexpr auto& operator[](usize index)
        {
            return at_unchecked(index);
        }

        /// @brief Returns unchecked const access by logical index.
        /// @param index Zero-based logical index.
        /// @return Const element reference; caller must provide a valid index.
        constexpr auto& operator[](usize index) const
        {
            return at_unchecked(index);
        }

        /// @brief Returns the logical front element.
        /// @return Mutable reference to the first element.
        constexpr auto& front()
        {
            return *begin();
        }

        /// @brief Returns the logical front element for const access.
        /// @return Const reference to the first element.
        constexpr auto& front() const
        {
            return *begin();
        }

        /// @brief Returns the logical back element.
        /// @return Mutable reference to the final element.
        constexpr auto& back()
        {
            return *(end()._get_previous());
        }

        /// @brief Returns the logical back element for const access.
        /// @return Const reference to the final element.
        constexpr auto& back() const
        {
            return *(end()._get_previous());
        }

        /// @brief Returns checked mutable access by logical index.
        /// @param index Zero-based logical index.
        /// @return Element reference or `bad_access` when out of range.
        constexpr auto at(usize index)
            -> result<reference<T>, bad_access>
        {
            if (index >= _size) return bad_access{};

            return {at_unchecked(index)};
        }

        /// @brief Returns checked const access by logical index.
        /// @param index Zero-based logical index.
        /// @return Const element reference or `bad_access` when out of range.
        constexpr auto at(usize index) const
            -> result<reference<const T>, bad_access>
        {
            if (index >= _size) return bad_access{};

            return {at_unchecked(index)};
        }

        /// @brief Maps a logical index to mutable ring storage without checking.
        /// @param index Zero-based logical index.
        /// @return Mutable element reference.
        constexpr auto& at_unchecked(usize index)
        {
            usize _idx = (_front._current + index) & (_capacity - 1);

            return _data[_idx];
        }

        /// @brief Maps a logical index to const ring storage without checking.
        /// @param index Zero-based logical index.
        /// @return Const element reference.
        constexpr const auto& at_unchecked(usize index) const
        {
            usize _idx = (_front._current + index) & (_capacity - 1);

            return _data[_idx];
        }

        /// @brief Returns a mutable iterator to the first logical element.
        /// @return Mutable begin iterator.
        constexpr iterator begin()
        {
            return _front;
        }

        /// @brief Returns a const iterator to the first logical element.
        /// @return Const begin iterator.
        constexpr const_iterator begin() const
        {
            return _front;
        }

        /// @brief Returns a const iterator to the first logical element.
        /// @return Const begin iterator.
        constexpr const_iterator cbegin() const
        {
            return _front;
        }

        /// @brief Returns a mutable reverse iterator to the last element.
        /// @return Mutable reverse-begin iterator.
        constexpr reverse_iterator rbegin()
        {
            return reverse_iterator{end()};
        }

        /// @brief Returns a const reverse iterator to the last element.
        /// @return Const reverse-begin iterator.
        constexpr const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator{end()};
        }

        /// @brief Returns a mutable iterator one past the final element.
        /// @return Mutable end iterator.
        constexpr iterator end()
        {
            return _back;
        }

        /// @brief Returns a const iterator one past the final element.
        /// @return Const end iterator.
        constexpr const_iterator end() const
        {
            return _back;
        }

        /// @brief Returns a const iterator one past the final element.
        /// @return Const end iterator.
        constexpr const_iterator cend() const
        {
            return _back;
        }

        /// @brief Returns a mutable reverse iterator past the first element.
        /// @return Mutable reverse-end iterator.
        constexpr reverse_iterator rend()
        {
            return reverse_iterator{begin()};
        }

        /// @brief Returns a const reverse iterator past the first element.
        /// @return Const reverse-end iterator.
        constexpr const_reverse_iterator rend() const
        {
            return const_reverse_iterator{begin()};
        }
    };
} // namespace hsd