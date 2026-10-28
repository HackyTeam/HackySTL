#pragma once

/// @file List.hpp
/// @brief Allocator-aware doubly linked list.
/// @details Provides stable node links, bidirectional iteration, front/back
/// insertion, erasure, copying, moving, and complete node lifetime handling.

#include "../Base/Allocator.hpp"

namespace hsd
{
    template < typename T, typename Allocator > class list;

    namespace list_detail
    {
        /// @brief Error tag returned when an operation targets a null iterator.
        struct bad_iterator
        {
            /// @brief Diagnostic message describing a null iterator access.
            /// @return Human-readable description of the failure.
            const char* operator()() const
            {
                return "Null pointer access denied";
            }
        };

        /// @brief Doubly linked node owning one element of a `list`.
        /// @tparam T - Type of the stored element.
        template <typename T>
        struct list_impl
        {
            /// @brief Element stored in this node.
            T _value;
            /// @brief Next node in the list, or `nullptr` if this is the tail.
            list_impl* _next = nullptr;
            /// @brief Previous node in the list, or `nullptr` if this is the head.
            list_impl* _back = nullptr;

            /// @brief Constructs `_value` in place from @p args.
            /// @tparam Args - Types forwarded to `T`'s constructor.
            /// @param args - Arguments forwarded to construct `_value`.
            template <typename... Args>
            constexpr list_impl(Args&&... args)
                : _value{hsd::forward<Args>(args)...}
            {}
        };
    
        /// @brief Non-owning bidirectional iterator over a `list`'s nodes.
        /// @tparam T - Value type of the parent list.
        /// @tparam Allocator - Allocator type of the parent list.
        template < typename T, typename Allocator >
        class iterator
        {
        private:
            /// @brief Node type this iterator walks.
            using impl_type = list_impl<T>;
            /// @brief Node currently referenced, or `nullptr` past-the-end.
            impl_type* _iterator = nullptr;

            /// @brief Grants the owning list access to `_iterator`.
            friend class list<T, Allocator>;
        
        public:
            /// @brief Constructs a past-the-end iterator.
            constexpr iterator() {}
            /// @brief Constructs a past-the-end iterator from `null_type`.
            constexpr iterator(hsd::null_type) {}

            /// @brief Copy-constructs an iterator at the same position.
            /// @param other - Iterator whose position is copied.
            constexpr iterator(const iterator& other)
            {
                _iterator = other._iterator;
            }
        
            /// @brief Assigns this iterator to another iterator's position.
            /// @param rhs - Iterator whose position is copied.
            /// @return Reference to `*this`.
            constexpr iterator& operator=(const iterator& rhs)
            {
                _iterator = rhs._iterator;
                return *this;
            }

            /// @brief Compares two iterators for equal node position.
            /// @param lhs - First iterator.
            /// @param rhs - Second iterator.
            /// @return `true` if both reference the same node (or are both
            /// past-the-end).
            constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator == rhs._iterator;
            }

            /// @brief Compares two iterators for differing node position.
            /// @param lhs - First iterator.
            /// @param rhs - Second iterator.
            /// @return `true` if the iterators reference different nodes.
            constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator != rhs._iterator;
            }

            /// @brief Advances to the next node in the list.
            /// @return Reference to `*this` after advancing.
            /// @details Follows the node's `_next` link; calling this on a
            /// past-the-end iterator is undefined behavior.
            constexpr auto& operator++()
            {
                _iterator = _iterator->_next;
                return *this;
            }

            /// @brief Moves to the previous node in the list.
            /// @return Reference to `*this` after moving back.
            /// @details Follows the node's `_back` link; calling this on an
            /// iterator referencing the head node (or on a past-the-end
            /// iterator that was never positioned within the list) is
            /// undefined behavior.
            constexpr auto& operator--()
            {
                _iterator = _iterator->_back;
                return *this;
            }

            /// @brief Advances to the next node, returning the prior position.
            /// @return Iterator at the position held before advancing.
            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            /// @brief Moves to the previous node, returning the prior position.
            /// @return Iterator at the position held before moving back.
            constexpr iterator operator--(i32)
            {
                iterator tmp = *this;
                operator--();
                return tmp;
            }

            /// @brief Accesses the value stored in the referenced node.
            /// @return Reference to the stored value.
            /// @details Dereferencing a past-the-end iterator is undefined
            /// behavior.
            constexpr auto& operator*()
            {
                return _iterator->_value;
            }

            /// @brief Accesses the value stored in the referenced node.
            /// @return Const reference to the stored value.
            /// @details Dereferencing a past-the-end iterator is undefined
            /// behavior.
            constexpr const auto& operator*() const
            {
                return _iterator->_value;
            }

            /// @brief Accesses a member of the value stored in the
            /// referenced node.
            /// @return Pointer to the stored value.
            /// @details Dereferencing a past-the-end iterator is undefined
            /// behavior.
            constexpr auto* operator->()
            {
                return hsd::addressof(_iterator->_value);
            }

            /// @brief Accesses a member of the value stored in the
            /// referenced node.
            /// @return Const pointer to the stored value.
            /// @details Dereferencing a past-the-end iterator is undefined
            /// behavior.
            constexpr const auto* operator->() const
            {
                return hsd::addressof(_iterator->_value);
            }
        };
    }

    /// @brief Allocator-aware doubly linked sequence container.
    /// @tparam T - Type of the stored elements.
    /// @tparam Allocator - Allocator type used to obtain node storage.
    template <typename T, typename Allocator = allocator>
    class list
    {
    public:
        /// @brief Bidirectional iterator type used to traverse this list.
        using iterator = list_detail::iterator<T, Allocator>;
        /// @brief Read-only bidirectional iterator type used to traverse this list.
        using const_iterator = const iterator;
        /// @brief Allocator type used to obtain node storage.
        using alloc_type = Allocator;

    private:
        /// @brief Node type allocated for every element.
        using impl_type = typename iterator::impl_type;
        /// @brief Allocator traits used to allocate/construct/destroy nodes.
        using alloc_traits = allocator_traits<alloc_type, impl_type>;
        /// @brief Iterator referencing the first node, or past-the-end if empty.
        iterator _head;
        /// @brief Iterator referencing the last node, or past-the-end if empty.
        iterator _tail;
        /// @brief Allocator used to allocate/construct/destroy every node.
        alloc_type _alloc;
        /// @brief Number of elements currently stored.
        usize _size = 0;

    public:
        /// @brief Constructs an empty list with a default-constructed
        /// allocator.
        constexpr list()
        requires (DefaultConstructible<alloc_type>) = default;

        ///
        /// @brief Constructs an empty list using the supplied allocator.
        ///
        /// @param alloc - Allocator used for every node in the list.
        ///
        constexpr list(const alloc_type& alloc)
            : _alloc{alloc}
        {}

        /// @brief Constructs a list by deep-copying every element of @p other.
        /// @param other - List whose elements are copied.
        /// @details The allocator is obtained via
        /// `allocator_traits::select_on_container_copy_construction`; new
        /// nodes are then allocated from this list's own allocator while
        /// copying each element of @p other in traversal order.
        constexpr list(const list& other)
            : _alloc{
                alloc_traits::select_on_container_copy_construction(
                    other._alloc
                )
            }
        {
            for(const auto& _element : other)
                push_back(_element);
        }

        /// @brief Takes ownership of another list's nodes and allocator.
        /// @param other - List to move from; left empty afterward.
        /// @details No nodes are allocated or copied; @p other's head, tail
        /// and size are transferred and then reset to an empty state.
        constexpr list(list&& other)
            : _head{hsd::exchange(other._head, nullptr)},
            _tail{hsd::exchange(other._tail, nullptr)},
            _alloc{hsd::move(other._alloc)},
            _size{hsd::exchange(other._size, 0u)}
        {}

        /// @brief Destroys and deallocates every remaining node.
        constexpr ~list()
        {
            clear();
        }

        /// @brief Constructs a list by copying an array, using a
        /// default-constructed allocator.
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the list.
        template <usize N>
        constexpr list(const T (&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : list{arr, alloc_type{}}
        {}

        ///
        /// @brief Constructs a list by copying an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the list.
        /// @param alloc - Allocator used for every new node.
        ///
        template <usize N>
        constexpr list(const T (&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(arr[_index]);
        }

        /// @brief Constructs a list by moving an array's elements, using a
        /// default-constructed allocator.
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the list.
        template <usize N>
        constexpr list(T (&&arr)[N])
        requires (DefaultConstructible<alloc_type>)
            : list{hsd::move(arr), alloc_type{}}
        {}

        ///
        /// @brief Constructs a list by moving an array with an allocator.
        ///
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the list.
        /// @param alloc - Allocator used for every new node.
        ///
        template <usize N>
        constexpr list(T (&&arr)[N], const alloc_type& alloc)
            : _alloc{alloc}
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(hsd::move(arr[_index]));
        }

        /// @brief Replaces the contents with a deep copy of @p rhs.
        /// @param rhs - List whose elements are copied.
        /// @return Reference to `*this`.
        /// @details Implemented via copy-and-swap: a temporary copy of
        /// @p rhs is built first and then swapped into `*this`, so this
        /// list's previous nodes are released once the temporary is
        /// destroyed.
        constexpr list& operator=(const list& rhs)
        {
            if (this != &rhs)
            {
                list _copy{rhs};
                hsd::swap(_alloc, _copy._alloc);
                hsd::swap(_head, _copy._head);
                hsd::swap(_tail, _copy._tail);
                hsd::swap(_size, _copy._size);
            }
            
            return *this;
        }

        /// @brief Replaces the contents by swapping ownership with @p rhs.
        /// @param rhs - List to move from.
        /// @return Reference to `*this`.
        /// @details No new nodes are allocated; `*this` and @p rhs simply
        /// swap their head, tail, size and allocator, so @p rhs ends up
        /// holding this list's previous contents.
        constexpr list& operator=(list&& rhs)
        {
            if (this != &rhs)
            {
                hsd::swap(_alloc, rhs._alloc);
                hsd::swap(_head, rhs._head);
                hsd::swap(_tail, rhs._tail);
                hsd::swap(_size, rhs._size);
            }
            
            return *this;
        }

        /// @brief Replaces the contents with a copy of @p arr's elements.
        /// @tparam N - Number of source elements.
        /// @param arr - Array copied into the list.
        /// @return Reference to `*this`.
        /// @details Existing nodes are cleared first, then a node is
        /// appended for each element of @p arr in order.
        template <usize N>
        constexpr list& operator=(const T (&arr)[N])
        {
            clear();
            usize _index = 0;

            for (auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = arr[_index];

            for (; _index < N; _index++)
                push_back(arr[_index]);

            return *this;
        }

        /// @brief Replaces the contents by moving @p arr's elements in.
        /// @tparam N - Number of source elements.
        /// @param arr - Array moved into the list.
        /// @return Reference to `*this`.
        /// @details Existing nodes are cleared first, then a node is
        /// appended for each moved element of @p arr in order.
        template <usize N>
        constexpr list& operator=(T (&&arr)[N])
        {
            clear();
            usize _index = 0;

            for (auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = hsd::move(arr[_index]);

            for (; _index < N; _index++)
                push_back(hsd::move(arr[_index]));

            return *this;
        }

        /// @brief Removes the node at @p pos from the list.
        /// @param pos - Iterator to the element to remove.
        /// @return On success, an iterator to the element following the
        /// removed one (or `end()` if the last element was removed); on
        /// failure, a `runtime_error` describing why the node could not be
        /// erased.
        /// @details Unlinks the node by relinking its neighbors' `_next`/
        /// `_back` pointers, then deallocates it through this list's
        /// allocator. Removing the tail or head delegates to `pop_back()`/
        /// `pop_front()` respectively. Fails only if @p pos is a
        /// past-the-end iterator, since there is no reliable way to detect
        /// otherwise whether it belongs to this list.
        constexpr auto erase(const_iterator pos)
            -> result<iterator, runtime_error>
        {
            if (pos._iterator == nullptr)
            {
                // this in the only situation when
                // .erase() will "throw" because
                // there is no fast way to check
                // if it belongs to this list or not
                return runtime_error{"Accessed an null element"};
            }
            
            if (pos == _tail)
            {
                pop_back();
                return end();
            }
            
            if (pos == _head)
            {
                pop_front();
                return begin();
            }

            iterator _next_iter, _back_iter;
            _next_iter._iterator = pos._iterator->_next;
            _back_iter._iterator = pos._iterator->_back;
            _next_iter._iterator->_back = _back_iter._iterator;
            _back_iter._iterator->_next = _next_iter._iterator;

            _size--;
            
            alloc_traits::destroy(_alloc, pos._iterator);

            alloc_traits
            ::deallocate(_alloc, pos._iterator, 1)
            .unwrap();
            
            return _next_iter;
        }

        /// @brief Appends a copy of @p value to the end of the list.
        /// @param value - Value copied into the new tail node.
        constexpr void push_back(const T& value)
        {
            emplace_back(value);
        }

        /// @brief Appends @p value to the end of the list by moving it.
        /// @param value - Value moved into the new tail node.
        constexpr void push_back(T&& value)
        {
            emplace_back(hsd::move(value));
        }

        /// @brief Constructs a new element in place at the end of the list.
        /// @tparam Args - Types forwarded to `T`'s constructor.
        /// @param args - Arguments forwarded to construct the new element.
        /// @details Allocates a node through this list's allocator,
        /// constructs `T` in it, and links it after the current tail (or
        /// as the sole node if the list was empty).
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if (empty())
            {
                _head._iterator = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();

                alloc_traits::construct(
                    _alloc, _head._iterator, hsd::forward<Args>(args)...
                );
                
                _tail = _head;
            }
            else
            {
                iterator _new_tail;
                
                _new_tail._iterator = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();

                alloc_traits::construct(
                    _alloc, _new_tail._iterator, hsd::forward<Args>(args)...
                );
                
                _new_tail._iterator->_back = _tail._iterator;
                _tail._iterator->_next = _new_tail._iterator;
                _tail = _new_tail;
            }

            _size++;
        }

        /// @brief Removes and destroys the last element of the list.
        /// @details Does nothing if the list is empty. Deallocates the old
        /// tail node through this list's allocator, then relinks the new
        /// tail's `_next` pointer to `nullptr`; if the list becomes empty,
        /// `_head` is reset as well.
        constexpr void pop_back()
        {
            if (empty()) return;

            auto _old_tail = _tail--;
            alloc_traits::destroy(_alloc, _old_tail._iterator);

            alloc_traits
            ::deallocate(_alloc, _old_tail._iterator, 1)
            .unwrap();

            _size--;
                
            if (_tail != end()) _tail._iterator->_next = nullptr;
            
            if (_size == 0)
            {
                _head._iterator = nullptr;
            }
        }

        /// @brief Prepends a copy of @p value to the start of the list.
        /// @param value - Value copied into the new head node.
        constexpr void push_front(const T& value)
        {
            emplace_front(value);
        }

        /// @brief Prepends @p value to the start of the list by moving it.
        /// @param value - Value moved into the new head node.
        constexpr void push_front(T&& value)
        {
            emplace_front(hsd::move(value));
        }

        /// @brief Constructs a new element in place at the start of the list.
        /// @tparam Args - Types forwarded to `T`'s constructor.
        /// @param args - Arguments forwarded to construct the new element.
        /// @details Allocates a node through this list's allocator,
        /// constructs `T` in it, and links it before the current head (or
        /// as the sole node if the list was empty).
        template <typename... Args>
        constexpr void emplace_front(Args&&... args)
        {
            if (empty())
            {
                _head._iterator = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();

                alloc_traits::construct(
                    _alloc, _head._iterator, hsd::forward<Args>(args)...
                );
                
                _tail = _head;
            }
            else
            {
                iterator _new_head;
                
                _new_head._iterator = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();

                alloc_traits::construct(
                    _alloc, _new_head._iterator, hsd::forward<Args>(args)...
                );
                
                _new_head._iterator->_next = _head._iterator;
                _head._iterator->_back = _new_head._iterator;
                _head = _new_head;
            }

            _size++;
        }

        /// @brief Removes and destroys the first element of the list.
        /// @details Does nothing if the list is empty. Deallocates the old
        /// head node through this list's allocator, then relinks the new
        /// head's `_back` pointer to `nullptr`; if the list becomes empty,
        /// `_tail` is reset as well.
        constexpr void pop_front()
        {
            if (empty()) return;

            auto _old_head = _head++;
            alloc_traits::destroy(_alloc, _old_head._iterator);

            alloc_traits
            ::deallocate(_alloc, _old_head._iterator, 1)
            .unwrap();
                
            _size--;

            if (_head != end()) _head._iterator->_back = nullptr;

            if (_size == 0)
            {
                _tail._iterator = nullptr;
            }
        }

        /// @brief Removes and destroys every element, leaving the list
        /// empty.
        /// @details Repeatedly calls `pop_front()` until the list is empty.
        constexpr void clear()
        {
            for (; !empty(); pop_front())
                ;
        }

        /// @brief Returns the number of elements currently stored.
        /// @return Element count.
        constexpr usize size() const
        {
            return _size;
        }

        ///
        /// @brief Returns the allocator associated with this list's nodes.
        ///
        /// @return Copy of the allocator used for this list's nodes.
        ///
        constexpr alloc_type get_allocator() const
        {
            return _alloc;
        }

        /// @brief Checks whether the list holds no elements.
        /// @return `true` if `size() == 0`.
        constexpr bool empty()
        {
            return size() == 0;
        }

        /// @brief Accesses the first element of the list.
        /// @return Reference to the first element.
        /// @details Calling this on an empty list is undefined behavior.
        constexpr T& front()
        {
            return *_head;
        }

        /// @brief Accesses the last element of the list.
        /// @return Reference to the last element.
        /// @details Calling this on an empty list is undefined behavior.
        constexpr T& back()
        {
            return *_tail;
        }

        /// @brief Returns an iterator positioned at the first element.
        /// @return Iterator to the first element, or `end()` if the list
        /// is empty.
        constexpr iterator begin()
        {
            return _head;
        }

        /// @brief Returns a const iterator positioned at the first element.
        /// @return Const iterator to the first element, or `end()` if the
        /// list is empty.
        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        /// @brief Returns the past-the-end sentinel iterator.
        /// @return Iterator that compares equal to any iterator that has
        /// been advanced past the last element.
        constexpr iterator end()
        {
            return {nullptr};
        }

        /// @brief Returns the past-the-end sentinel const iterator.
        /// @return Const iterator that compares equal to any const iterator
        /// that has been advanced past the last element.
        constexpr const_iterator end() const
        {
            return cend();
        }

        /// @brief Returns a const iterator positioned at the first element.
        /// @return Const iterator to the first element, or `cend()` if the
        /// list is empty.
        constexpr const_iterator cbegin() const
        {
            return _head;
        }

        /// @brief Returns the past-the-end sentinel const iterator.
        /// @return Const iterator that compares equal to any const iterator
        /// that has been advanced past the last element.
        constexpr const_iterator cend() const
        {
            return {nullptr};
        }

        /// @brief Returns an iterator positioned at the last element.
        /// @return Iterator to the last element, or `end()` if the list is
        /// empty.
        /// @details This reuses the same `iterator` type as `begin()`, whose
        /// `operator++` always follows `_next` (toward the tail) regardless
        /// of where the iterator started. Because of that, incrementing
        /// from `rbegin()` reaches `end()` after a single step, so the
        /// `[rbegin(), rend())` range visits only the last element rather
        /// than the whole list in reverse order; walking backward requires
        /// explicit `operator--` calls following `_back`.
        constexpr iterator rbegin()
        {
            return _tail;
        }

        /// @brief Returns a const iterator positioned at the last element.
        /// @return Const iterator equivalent to `crbegin()`.
        constexpr const_iterator rbegin() const
        {
            return crbegin();
        }

        /// @brief Returns the past-the-end sentinel iterator for reverse
        /// traversal.
        /// @return Iterator that compares equal to any iterator that has
        /// been advanced past the last element.
        constexpr iterator rend()
        {
            return {nullptr};
        }

        /// @brief Returns the past-the-end sentinel const iterator for
        /// reverse traversal.
        /// @return Const iterator equivalent to `crend()`.
        constexpr const_iterator rend() const
        {
            return crend();
        }

        /// @brief Returns a const iterator positioned at the last element.
        /// @return Const iterator to the last element, or `cend()` if the
        /// list is empty.
        /// @details Same forward-only `operator++` caveat as `rbegin()`
        /// applies here.
        constexpr const_iterator crbegin() const
        {
            return _tail;
        }

        /// @brief Returns the past-the-end sentinel const iterator for
        /// reverse traversal.
        /// @return Const iterator that compares equal to any const iterator
        /// that has been advanced past the last element.
        constexpr const_iterator crend() const
        {
            return {nullptr};
        }
    };
}