#pragma once

/// @file SharedPtr.hpp
/// @brief Atomic and non-atomic allocator-aware shared ownership.
/// @details Provides reference-counted single-object and array storage,
/// allocator-paired destruction, derived conversions, and make_shared helpers.

#include "../Base/Allocator.hpp"
#include "../Threading/Atomic.hpp"
#include "../Base/Utility.hpp"

namespace hsd
{
    namespace non_atomic_types
    {
        namespace shared_detail
        {
            /// @brief Marks an owner or counter built over a fused make_shared block.
            struct fused_tag {};

            /// @brief Marks a counter that does not own a count yet.
            struct empty_tag {};

            /// @brief Words in front of the object in a fused block.
            /// @details Word 0 is the owner count, word 1 the block length.
            inline constexpr usize fused_header_words = 2;

            /// @brief Tells whether make_shared can place `T` behind the header.
            /// @details Only the alignment of the count words is guaranteed
            ///          for the block, so over-aligned types keep two blocks.
            template <typename T>
            inline constexpr bool can_fuse = alignof(T) <= alignof(usize);

            /// @brief Accepts conversions from a derived shared owner to its base.
            template <typename Base, typename Derived>
            concept ConvertibleDerived = (
                Convertible<Derived, Base> || 
                std::is_base_of_v<Base, Derived>
            );

            /// @brief Allocator-backed object or array storage shared by owners.
            /// @tparam T Object type, possibly an unbounded array type.
            /// @tparam Allocator Allocator used to construct and release elements.
            template < typename T, typename Allocator >
            class storage
            {
            public:
                /// @brief Allocator associated with the allocation.
                using alloc_type = Allocator;
                /// @brief Pointer to the first stored element.
                using pointer_type = remove_array_t<T>*;
                /// @brief Element type constructed in the allocation.
                using value_type = remove_array_t<T>;

            private:
                /// @brief Allocator operations used for element lifetime management.
                using alloc_traits = allocator_traits<alloc_type, value_type>;
                /// @brief Allocator retained until final-owner destruction.
                alloc_type _alloc;
                /// @brief Allocation base address, or null after transfer/release.
                pointer_type _data = nullptr;
                /// @brief Number of elements represented by `_data`.
                usize _size = 0;

                template <typename U, typename Alloc>
                friend class storage;

            public:
                /// @brief Constructs empty storage with a default allocator.
                constexpr storage()
                requires (DefaultConstructible<alloc_type>) = default;

                /// @brief Adopts an existing allocation and element count.
                /// @param ptr Allocation base address.
                /// @param size Number of elements represented by `ptr`.
                constexpr storage(pointer_type ptr, usize size)
                requires (DefaultConstructible<alloc_type>)
                    : _data{ptr}, _size{size}
                {}

                /// @brief Allocates and default-constructs `size` elements.
                /// @param alloc Allocator used for allocation and construction.
                /// @param size Number of elements to allocate.
                constexpr storage(const alloc_type& alloc, usize size)
                requires (DefaultConstructible<value_type>)
                    : _alloc{alloc}, _size{size}
                {
                    _data = alloc_traits
                        ::allocate(_alloc, size)
                        .unwrap();

                    for (usize _index = 0; _index < size; _index++)
                    {
                        alloc_traits::construct(_alloc, &_data[_index]);
                    }
                }

                /// @brief Adopts an allocation together with its allocator state.
                /// @param ptr Allocation base address.
                /// @param alloc Allocator responsible for the allocation.
                /// @param size Number of represented elements.
                constexpr storage(pointer_type ptr, const alloc_type& alloc, usize size)
                    : _alloc{alloc}, _data{ptr}, _size{size}
                {}

                /// @brief Copies allocator and allocation metadata without copying elements.
                /// @param other Storage metadata to copy.
                constexpr storage(const storage& other)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                /// @brief Transfers allocation metadata and nulls the source pointer.
                /// @param other Storage whose allocation is adopted.
                constexpr storage(storage&& other)
                    : _alloc{hsd::move(other._alloc)}
                {
                    _data = hsd::exchange(other._data, nullptr);
                    hsd::swap(_size, other._size);
                }

                /// @brief Copies compatible storage metadata without copying elements.
                /// @param other Compatible storage metadata to copy.
                template <typename U = T>
                constexpr storage(const storage<U, Allocator>& other)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                /// @brief Transfers compatible storage metadata from `other`.
                /// @param other Compatible storage whose allocation is adopted.
                template <typename U = T>
                constexpr storage(storage<U, Allocator>&& other)
                    : _alloc{hsd::move(other._alloc)}
                {
                    _data = hsd::exchange(other._data, nullptr);
                    hsd::swap(_size, other._size);
                }

                /// @brief Copies storage metadata without copying elements.
                /// @param rhs Storage metadata to copy.
                /// @return `*this` after the assignment.
                constexpr storage& operator=(const storage& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                /// @brief Transfers storage metadata from `rhs`.
                /// @param rhs Storage whose allocation is adopted.
                /// @return `*this` after the assignment.
                constexpr storage& operator=(storage&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc);
                    _data = hsd::exchange(rhs._data, nullptr);                    
                    hsd::swap(_size, rhs._size);
                    return *this;
                }

                /// @brief Copies compatible storage metadata.
                /// @param rhs Compatible storage metadata to copy.
                /// @return `*this` after the assignment.
                template <typename U = T>
                constexpr storage& operator=(const storage<U, Allocator>& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                /// @brief Transfers compatible storage metadata.
                /// @param rhs Compatible storage whose allocation is adopted.
                /// @return `*this` after the assignment.
                template <typename U = T>
                constexpr storage& operator=(storage<U, Allocator>&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc); 
                    _data = hsd::exchange(rhs._data, nullptr);               
                    hsd::swap(_size, rhs._size);
                    return *this;
                }

                /// @brief Releases the allocation through the retained allocator.
                /// @return The allocator's deallocation result.
                constexpr auto deallocate()
                {
                    return alloc_traits::deallocate(_alloc, _data, _size);
                }

                /// @brief Destroys represented elements in reverse order.
                constexpr void destroy()
                {
                    for (usize _index = _size; _index > 0; --_index)
                    {
                        alloc_traits::destroy(
                            _alloc, &_data[_index - 1]
                        );
                    }
                }

                /// @brief Returns the number of represented elements.
                /// @return Number of represented elements.
                constexpr usize get_size() const
                {
                    return _size;
                }

                /// @brief Returns the allocation base address.
                /// @return Allocation base address.
                constexpr auto* get_pointer() const
                {
                    return _data;
                }

                /// @brief Replaces the tracked allocation address.
                /// @param ptr New allocation address, usually null after release.
                constexpr void set_pointer(pointer_type ptr)
                {
                    this->_data = ptr;
                }

                /// @brief Replaces the represented element count.
                /// @param size New element count.
                constexpr void set_size(usize size)
                {
                    _size = size;
                }
            };

            /// @brief Non-atomic reference count allocated beside shared storage.
            template <typename Allocator>
            class counter
            {
            public:
                /// @brief Allocator used for the count allocation.
                using alloc_type = Allocator;
                /// @brief Pointer to the count value.
                using pointer_type = usize*;
                /// @brief Stored count type.
                using value_type = usize;

            private:
                /// @brief Allocator operations used for count lifetime management.
                using alloc_traits = allocator_traits<alloc_type, value_type>;
                /// @brief Allocator retained until the count allocation is released.
                alloc_type _alloc;
                // Set when the count heads one block that also holds the
                // object; releasing the count then frees that whole block.
                bool _fused = false;
                /// @brief Shared count address, or null after ownership transfer.
                pointer_type _data = nullptr;              

            public:
                /// @brief Allocates and initializes a count to one.
                constexpr counter()
                requires (DefaultConstructible<alloc_type>)
                {
                    _data = alloc_traits
                        ::allocate(_alloc, 1)
                        .unwrap();

                    alloc_traits::construct(_alloc, _data, 1u);
                }

                /// @brief Allocates a count using `alloc`.
                /// @param alloc Allocator used for the count allocation.
                constexpr counter(const alloc_type& alloc)
                    : _alloc{alloc}
                {
                    _data = alloc_traits
                        ::allocate(_alloc, 1)
                        .unwrap();
                    
                    alloc_traits::construct(_alloc, _data, 1u);
                }

                /// @brief Adopts a count and increments it for a new owner.
                /// @param ptr Existing shared count.
                constexpr counter(pointer_type ptr)
                requires (DefaultConstructible<alloc_type>)
                    : _data{ptr}
                {
                    (*_data)++;
                }

                /// @brief Adopts a count with an explicit allocator and increments it.
                /// @param alloc Allocator responsible for the count.
                /// @param ptr Existing shared count.
                constexpr counter(const alloc_type& alloc, usize* ptr)
                    : _alloc{alloc}, _data{ptr}
                {
                    (*_data)++;
                }

                /// @brief Copies count metadata and increments the shared count.
                /// @param other Counter whose count is shared.
                /// @brief Constructs a counter that owns no count.
                constexpr counter(empty_tag)
                requires (DefaultConstructible<alloc_type>)
                {}

                /// @brief Wraps the first word of a fused make_shared block.
                /// @details The count in that word is already one.
                constexpr counter(const alloc_type& alloc, usize* block, fused_tag)
                    : _alloc{alloc}, _fused{true}, _data{block}
                {}

                constexpr counter(const counter& other)
                    : _alloc{other._alloc}, _fused{other._fused}, _data{other._data}
                {
                    if (_data != nullptr) (*_data)++;
                }

                /// @brief Transfers count metadata and clears the source pointer.
                /// @param other Counter whose count is adopted.
                constexpr counter(counter&& other)
                    : _alloc{hsd::move(other._alloc)}, _fused{other._fused}, _data{other._data}
                {
                    other._data = nullptr;
                }

                /// @brief Shares `rhs`'s count and increments it.
                /// @param rhs Counter whose count is shared.
                /// @return `*this` after sharing the count.
                constexpr counter& operator=(const counter& rhs)
                {
                    _alloc = rhs._alloc;
                    _fused = rhs._fused;
                    _data = rhs._data;
                    if (_data != nullptr) (*_data)++;
                    return *this;
                }

                /// @brief Transfers count metadata from `rhs`.
                /// @param rhs Counter whose count is adopted.
                /// @return `*this` after the transfer.
                constexpr counter& operator=(counter&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc);
                    hsd::swap(_fused, rhs._fused);
                    hsd::swap(_data, rhs._data);
                    return *this;
                }

                /// @brief Returns mutable access to the shared count value.
                /// @return Mutable reference to the shared count.
                constexpr usize& operator*()
                {
                    return *_data;
                }

                /// @brief Destroys and deallocates the count object.
                /// @return The allocator's deallocation result.
                /// @brief Tells whether releasing the count frees the object's block too.
                constexpr bool is_fused() const
                {
                    return _fused;
                }

                constexpr auto deallocate()
                {
                    // A fused block recorded its own length after the count.
                    const usize _words = _fused ? static_cast<usize>(_data[1]) : 1u;
                    alloc_traits::destroy(_alloc, _data);
                    return alloc_traits::deallocate(_alloc, _data, _words);
                }

                /// @brief Returns the allocation size used for the count object.
                /// @return Allocation size used for the count object.
                constexpr usize get_size() const
                {
                    return 1u;
                }

                /// @brief Returns the shared count address.
                /// @return Shared count address.
                constexpr auto* get_pointer() const
                {
                    return _data;
                }

                /// @brief Replaces the tracked count address.
                /// @param ptr New count address, usually null after release.
                constexpr void set_pointer(pointer_type ptr)
                {
                    _data = ptr;
                    if (ptr == nullptr) _fused = false;
                }
            };
            
        } // namespace shared_detail

        /// @brief Non-atomic allocator-aware shared owner.
        template <typename T, typename Allocator = allocator>
        class shared_ptr
        {
        private:
            /// @brief Object/array allocation metadata owned by this handle.
            using storage_type = shared_detail::storage<T, Allocator>;
            /// @brief Non-atomic count metadata shared by all owners.
            using counter_type = shared_detail::counter<Allocator>;

        public:
            /// @brief Construction, access, reference counting, reset, and moves.
            /// @details Copies share the allocation and increment the non-atomic
            /// count. The final owner destroys elements, releases storage, and
            /// releases the count allocation.
            using alloc_type = Allocator;
            /// @brief Allocator type retained by the control block.
            using pointer_type = typename storage_type::pointer_type;
            /// @brief Stored element type.
            using value_type = typename storage_type::value_type;
            /// @brief Mutable reference to a stored element.
            using reference_type = typename storage_type::value_type&;

        private:
            /// @brief Allocation metadata for the owned object or array.
            storage_type _value;
            /// @brief Shared non-atomic owner count.
            // An empty owner has no count; adopting constructors add one.
            counter_type _count{shared_detail::empty_tag{}};

            template <typename U, typename Alloc>
            friend class shared_ptr;

            /// @brief Releases one owner and destroys the allocation at zero.
            constexpr void _delete()
            {
                if (_count.get_pointer() != nullptr)
                {
                    (*_count)--;

                    if (*_count == 0)
                    {
                        if (get() != nullptr) 
                        {
                            _value.destroy();
                        }
                        // A fused block owns the object's memory as well.
                        if (!_count.is_fused())
                        {
                            _value.deallocate().unwrap();
                        }
                        _count.deallocate().unwrap();
                    }

                    _value.set_pointer(nullptr);
                    _count.set_pointer(nullptr);
                }
            }

        public:
            /// @brief Constructs an empty owner.
            constexpr shared_ptr() = default;
            /// @brief Constructs an empty owner from the null sentinel.
            constexpr shared_ptr(null_type) {}

            /// @brief Adopts one existing object allocation.
            /// @param ptr Object allocation to manage.
            constexpr shared_ptr(pointer_type ptr) 
            requires (DefaultConstructible<alloc_type>)
                : _value{ptr, 1u}, _count{}
            {}

            /// @brief Adopts an existing object or array allocation.
            /// @param ptr Allocation base address.
            /// @param size Number of elements represented by the allocation.
            constexpr shared_ptr(pointer_type ptr, usize size) 
            requires (DefaultConstructible<alloc_type>)
                : _value{ptr, size}, _count{}
            {}

            /// @brief Allocates and default-constructs one object.
            /// @param alloc Allocator used for the object and count.
            constexpr shared_ptr(const alloc_type& alloc) 
                : _value{alloc, 1u}, _count{alloc}
            {}

            /// @brief Allocates and default-constructs an array.
            /// @param alloc Allocator used for the array and count.
            /// @param size Number of elements to allocate.
            constexpr shared_ptr(const alloc_type& alloc, usize size) 
                : _value{alloc, size}, _count{alloc}
            {}

            /// @brief Adopts storage and records its allocator and extent.
            /// @param ptr Allocation base address.
            /// @param alloc Allocator responsible for the allocation.
            /// @param size Number of represented elements.
            constexpr shared_ptr(pointer_type ptr, 
                const alloc_type& alloc, usize size) 
                : _value{ptr, alloc, size}, _count{alloc}
            {}

            /// @brief Shares ownership with a convertible owner.
            /// @param other Existing owner whose allocation and count are shared.
            template <typename U = T> 
            constexpr shared_ptr(const shared_ptr<U, Allocator>& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{other._value}, _count{other._count}
            {}

            /// @brief Moves ownership metadata from a convertible owner.
            /// @param other Owner whose allocation is transferred.
            template <typename U = T> 
            constexpr shared_ptr(shared_ptr<U, Allocator>&& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{hsd::move(other._value)}, _count{hsd::move(other._count)}
            {}

            /// @brief Releases this owner and its reference to the control block.
            /// @brief Adopts a fused block made by make_shared.
            /// @details Implementation detail: the block holds the count,
            ///          its own length, then the constructed object.
            shared_ptr(shared_detail::fused_tag, const alloc_type& alloc, usize* block)
                : _value{reinterpret_cast<pointer_type>(
                      block + shared_detail::fused_header_words), alloc, 1u},
                  _count{alloc, block, shared_detail::fused_tag{}}
            {}

            constexpr ~shared_ptr()
            {
                _delete();
            }

            /// @brief Releases ownership and becomes empty.
            /// @return `*this` after becoming empty.
            constexpr shared_ptr& operator=(null_type)
            {
                _delete();
                return *this;
            }

            /// @brief Copies the owner, sharing its allocation and count.
            /// @details Declared explicitly, because the user-declared copy-assignment
            /// below would otherwise deprecate the implicit copy constructor.
            constexpr shared_ptr(const shared_ptr&) = default;

            /// @brief Shares ownership with `rhs` after releasing this one.
            /// @details Declared explicitly: the implicit copy-assignment copies
            /// the count without releasing the previous owner, so assigning from
            /// a const lvalue leaked the old object.
            /// @param rhs Owner whose allocation and count are shared.
            /// @return `*this` after sharing ownership.
            constexpr shared_ptr& operator=(const shared_ptr& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            /// @brief Shares ownership with a convertible owner after releasing this one.
            /// @param rhs Owner whose allocation and count are shared.
            /// @return `*this` after sharing ownership.
            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            constexpr shared_ptr& operator=(shared_ptr<U, Allocator>& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            /// @brief Moves ownership from a convertible owner after releasing this one.
            /// @param rhs Owner whose allocation is transferred.
            /// @return `*this` after the transfer.
            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            constexpr shared_ptr& operator=(shared_ptr<U, Allocator>&& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = hsd::move(rhs._value);
                _count = hsd::move(rhs._count);
                return *this;
            }

            /// @brief Compares two owners by managed pointer identity.
            /// @param rhs Owner to compare.
            /// @return True when both handles point at the same address.
            constexpr bool operator==(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            /// @brief Compares two owners by managed pointer identity.
            /// @param rhs Owner to compare.
            /// @return True when the managed pointers differ.
            constexpr bool operator!=(const shared_ptr& rhs) const
            {
                return get() != rhs.get();
            }

            /// @brief Tests whether this owner is empty.
            /// @return True when no object is managed.
            constexpr bool operator==(null_type) const
            {
                return get() == nullptr;
            }

            /// @brief Tests whether this owner is non-empty.
            /// @return True when an object is managed.
            constexpr bool operator!=(null_type) const
            {
                return get() != nullptr;
            }

            /// @brief Returns the managed pointer.
            /// @return Managed object or array address, possibly null.
            constexpr auto* get()
            {
                return _value.get_pointer();
            }

            /// @brief Returns the managed pointer for const access.
            /// @return Managed object or array address, possibly null.
            constexpr auto* get() const
            {
                return _value.get_pointer();
            }

            /// @brief Provides member access through the managed pointer.
            /// @return Managed object or array address.
            constexpr auto* operator->()
            {
                return get();
            }

            /// @brief Provides const member access through the managed pointer.
            /// @return Managed object or array address.
            constexpr auto* operator->() const
            {
                return get();
            }

            /// @brief Dereferences the managed object.
            /// @return Reference to the managed element.
            constexpr auto& operator*()
            {
                return *get();
            }

            /// @brief Dereferences the managed object for const access.
            /// @return Reference to the managed element.
            constexpr auto& operator*() const
            {
                return *get();
            }

            /// @brief Returns the current non-atomic owner count.
            /// @return Number of owners sharing the control block.
            constexpr usize get_count()
            {
                // An empty owner shares nothing.
                if (_count.get_pointer() == nullptr) return 0;
                return *_count;
            }

            /// @brief Tests whether this is the only owner.
            /// @return True when the shared count is one.
            constexpr bool is_unique()
            {
                return get_count() == 1;
            }
        };

        namespace shared_detail
        {
            /// @brief Builds one object and its owner count in a single block.
            /// @details Layout: [count][block length in words][object]. The
            ///          count is released together with the block, so it
            ///          needs no allocation of its own.
            template < typename T, typename Allocator, typename... Args >
            inline shared_ptr<T, Allocator> make_fused(Allocator& alloc, Args&&... args)
            {
                using value_type = remove_array_t<T>;
                using count_traits = allocator_traits<Allocator, usize>;
                using value_traits = allocator_traits<Allocator, value_type>;

                constexpr usize _value_words =
                    (sizeof(value_type) + sizeof(usize) - 1) / sizeof(usize);
                constexpr usize _words = fused_header_words + _value_words;

                auto* _block = count_traits::allocate(alloc, _words).unwrap();
                count_traits::construct(alloc, _block, 1u);
                count_traits::construct(alloc, _block + 1, _words);

                auto* _object = reinterpret_cast<value_type*>(
                    _block + fused_header_words
                );
                value_traits::construct(alloc, _object, hsd::forward<Args>(args)...);

                return shared_ptr<T, Allocator>{fused_tag{}, alloc, _block};
            }
        } // namespace shared_detail

        /// @brief Maps object and array forms to the shared_ptr result type.
        template < typename T, typename Allocator >
        struct MakeShr
        {
            /// @brief Result type for one object allocation.
            using single_object = shared_ptr<T, Allocator>;
        };

        /// @brief Maps an unbounded array to its shared_ptr array result.
        template < typename T, typename Allocator >
        struct MakeShr<T[], Allocator>
        {
            /// @brief Result type for an array allocation.
            using array = shared_ptr<T[], Allocator>;
        };

        /// @brief Rejects bounded arrays, which do not have a runtime extent.
        template < typename T, usize N, typename Allocator >
        struct MakeShr<T[N], Allocator>
        {
            /// @brief Marker used to make bounded-array make_shared calls invalid.
            struct invalid_type {};  
        };

        /// @brief Allocates and constructs one non-atomic shared object.
        /// @tparam T Object type.
        /// @tparam Allocator Allocator type.
        /// @param args Constructor arguments forwarded to `T`.
        /// @return A shared owner for the newly constructed object.
        template < typename T, typename Allocator = allocator, typename... Args >
        requires (DefaultConstructible<Allocator> && IsSame<remove_array_t<T>, T>)
        static constexpr typename MakeShr<T, Allocator>::single_object 
        make_shared(Args&&... args)
        {
            Allocator _alloc;
            if constexpr (shared_detail::can_fuse<remove_array_t<T>>)
            {
                if (!is_constant_evaluated())
                {
                    return shared_detail::make_fused<T, Allocator>(
                        _alloc, hsd::forward<Args>(args)...
                    );
                }
            }

            using alloc_traits = allocator_traits<
                Allocator, remove_array_t<T>
            >;

            auto* _ptr = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();
            
            alloc_traits::construct(
                _alloc, _ptr, hsd::forward<Args>(args)...
            );
            
            return shared_ptr<T, Allocator>{_ptr, _alloc, 1};
        }

        /// @brief Allocates and constructs one object using an explicit allocator.
        /// @param alloc Allocator used for the object and control block.
        /// @param args Constructor arguments forwarded to `T`.
        /// @return A shared owner for the newly constructed object.
        template < typename T, typename Allocator = allocator, typename U, typename... Args >
        requires (IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
        static constexpr typename MakeShr<T, Allocator>::single_object 
        make_shared(U& alloc, Args&&... args)
        {
            if constexpr (shared_detail::can_fuse<remove_array_t<T>>)
            {
                if (!is_constant_evaluated())
                {
                    return shared_detail::make_fused<T, Allocator>(
                        alloc, hsd::forward<Args>(args)...
                    );
                }
            }

            using alloc_traits = allocator_traits<
                Allocator, remove_array_t<T>
            >;
            auto* _ptr = alloc_traits
                ::allocate(alloc, 1)
                .unwrap();

            alloc_traits::construct(
                alloc, _ptr, hsd::forward<Args>(args)...
            );
            return {_ptr, alloc, 1};
        }

        /// @brief Allocates and default-constructs an unbounded array.
        /// @param size Number of elements in the array.
        /// @return A shared owner for the newly allocated array.
        template < typename T, typename Allocator = allocator >
        requires (DefaultConstructible<Allocator> && !IsSame<remove_array_t<T>, T>)
        static constexpr typename MakeShr<T, Allocator>::array
        make_shared(usize size)
        {
            Allocator _alloc;
            return {_alloc, size};
        }

        /// @brief Allocates an unbounded array using an explicit allocator.
        /// @param alloc Allocator used for the array and control block.
        /// @param size Number of elements in the array.
        /// @return A shared owner for the newly allocated array.
        template < typename T, typename Allocator = allocator, typename U >
        requires (!IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
        static constexpr typename MakeShr<T, Allocator>::array
        make_shared(U& alloc, usize size)
        {
            return {alloc, size};
        }

        /// @brief Deleted overload preventing unsupported bounded-array objects.
        template < typename T, typename Allocator = allocator, typename... Args >
        static constexpr typename MakeShr<T, Allocator>::invalid_type 
        make_shared(Args&&...) = delete;
    }

    namespace atomic_types
    {
        namespace shared_detail
        {
            /// @brief Marks an owner or counter built over a fused make_shared block.
            struct fused_tag {};

            /// @brief Marks a counter that does not own a count yet.
            struct empty_tag {};

            /// @brief Words in front of the object in a fused block.
            /// @details Word 0 is the owner count, word 1 the block length.
            inline constexpr usize fused_header_words = 2;

            /// @brief Tells whether make_shared can place `T` behind the header.
            /// @details Only the alignment of the count words is guaranteed
            ///          for the block, so over-aligned types keep two blocks.
            template <typename T>
            inline constexpr bool can_fuse = alignof(T) <= alignof(atomic_usize);

            /// @brief Accepts conversions from a derived shared owner to its base.
            template <typename Base, typename Derived>
            concept ConvertibleDerived = (
                Convertible<Derived, Base> || 
                std::is_base_of_v<Base, Derived>
            );

            /// @brief Allocator-backed object or array storage shared by owners.
            /// @tparam T Object type, possibly an unbounded array type.
            /// @tparam Allocator Allocator used to construct and release elements.
            template < typename T, typename Allocator >
            class storage
            {
            public:
                /// @brief Allocator associated with the allocation.
                using alloc_type = Allocator;
                /// @brief Pointer to the first stored element.
                using pointer_type = remove_array_t<T>*;
                /// @brief Element type constructed in the allocation.
                using value_type = remove_array_t<T>;

            private:
                /// @brief Allocator operations used for element lifetime management.
                using alloc_traits = allocator_traits<alloc_type, value_type>;
                /// @brief Allocator retained until final-owner destruction.
                alloc_type _alloc;
                /// @brief Allocation base address, or null after transfer/release.
                pointer_type _data = nullptr;
                /// @brief Number of elements represented by `_data`.
                usize _size = 0;

                template <typename U, typename Alloc>
                friend class storage;

            public:
                /// @brief Constructs empty storage with a default allocator.
                constexpr storage()
                requires (DefaultConstructible<alloc_type>) = default;

                /// @brief Adopts an existing allocation and element count.
                /// @param ptr Allocation base address.
                /// @param size Number of elements represented by `ptr`.
                constexpr storage(pointer_type ptr, usize size)
                requires (DefaultConstructible<alloc_type>)
                    : _data{ptr}, _size{size}
                {}

                /// @brief Allocates and default-constructs `size` elements.
                /// @param alloc Allocator used for allocation and construction.
                /// @param size Number of elements to allocate.
                constexpr storage(const alloc_type& alloc, usize size)
                requires (DefaultConstructible<value_type>)
                    : _alloc{alloc}, _size{size}
                {
                    _data = alloc_traits
                        ::allocate(_alloc, size)
                        .unwrap();

                    for (usize _index = 0; _index < size; _index++)
                    {
                        alloc_traits::construct(_alloc, &_data[_index]);
                    }
                }

                /// @brief Adopts an allocation together with its allocator state.
                /// @param ptr Allocation base address.
                /// @param alloc Allocator responsible for the allocation.
                /// @param size Number of represented elements.
                constexpr storage(pointer_type ptr, const alloc_type& alloc, usize size)
                    : _alloc{alloc}, _data{ptr}, _size{size}
                {}

                /// @brief Copies allocator and allocation metadata without copying elements.
                /// @param other Storage metadata to copy.
                constexpr storage(const storage& other)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                /// @brief Transfers allocation metadata and nulls the source pointer.
                /// @param other Storage whose allocation is adopted.
                constexpr storage(storage&& other)
                    : _alloc{hsd::move(other._alloc)}
                {
                    _data = hsd::exchange(other._data, nullptr);
                    hsd::swap(_size, other._size);
                }

                /// @brief Copies compatible storage metadata without copying elements.
                /// @param other Compatible storage metadata to copy.
                template <typename U = T>
                constexpr storage(const storage<U, Allocator>& other)
                    : _alloc{other._alloc}, _data{other._data}, _size{other._size}
                {}

                /// @brief Transfers compatible storage metadata from `other`.
                /// @param other Compatible storage whose allocation is adopted.
                template <typename U = T>
                constexpr storage(storage<U, Allocator>&& other)
                    : _alloc{hsd::move(other._alloc)}
                {
                    _data = hsd::exchange(other._data, nullptr);
                    hsd::swap(_size, other._size);
                }

                /// @brief Copies storage metadata without copying elements.
                /// @param rhs Storage metadata to copy.
                /// @return `*this` after the assignment.
                constexpr storage& operator=(const storage& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                /// @brief Transfers storage metadata from `rhs`.
                /// @param rhs Storage whose allocation is adopted.
                /// @return `*this` after the assignment.
                constexpr storage& operator=(storage&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc);
                    hsd::swap(_data, rhs._data);                    
                    hsd::swap(_size, rhs._size);
                    return *this;
                }

                /// @brief Copies compatible storage metadata.
                /// @param rhs Compatible storage metadata to copy.
                /// @return `*this` after the assignment.
                template <typename U = T>
                constexpr storage& operator=(const storage<U, Allocator>& rhs)
                {
                    _alloc = rhs._alloc;
                    _data = rhs._data;
                    _size = rhs._size;
                    return *this;
                }

                /// @brief Transfers compatible storage metadata.
                /// @param rhs Compatible storage whose allocation is adopted.
                /// @return `*this` after the assignment.
                template <typename U = T>
                constexpr storage& operator=(storage<U, Allocator>&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc);
                    _data = hsd::exchange(rhs._data, nullptr);                    
                    hsd::swap(_size, rhs._size);
                    return *this;
                }

                /// @brief Releases the allocation through the retained allocator.
                /// @return The allocator's deallocation result.
                constexpr auto deallocate()
                {
                    return alloc_traits::deallocate(_alloc, _data, _size);
                }

                /// @brief Destroys represented elements in reverse order.
                constexpr void destroy()
                {
                    for (usize _index = _size; _index > 0; --_index)
                    {
                        alloc_traits::destroy(
                            _alloc, &_data[_index - 1]
                        );
                    }
                }

                /// @brief Returns the number of represented elements.
                /// @return Number of represented elements.
                constexpr usize get_size() const
                {
                    return _size;
                }

                /// @brief Returns the allocation base address.
                /// @return Allocation base address.
                constexpr auto* get_pointer() const
                {
                    return _data;
                }

                /// @brief Replaces the tracked allocation address.
                /// @param ptr New allocation address, usually null after release.
                constexpr void set_pointer(pointer_type ptr)
                {
                    this->_data = ptr;
                }

                /// @brief Replaces the represented element count.
                /// @param size New element count.
                constexpr void set_size(usize size)
                {
                    _size = size;
                }
            };

            /// @brief Atomic reference count allocated beside shared storage.
            template < typename Allocator >
            class counter
            {
            public:
                /// @brief Allocator used for the count allocation.
                using alloc_type = Allocator;
                /// @brief Pointer to the atomic count value.
                using pointer_type = atomic_usize*;
                /// @brief Stored atomic count type.
                using value_type = atomic_usize;

            private:
                /// @brief Allocator operations used for count lifetime management.
                using alloc_traits = allocator_traits<alloc_type, value_type>;
                /// @brief Allocator retained until the count allocation is released.
                alloc_type _alloc;
                // Set when the count heads one block that also holds the
                // object; releasing the count then frees that whole block.
                bool _fused = false;
                /// @brief Shared count address, or null after ownership transfer.
                atomic_usize* _data = nullptr;              

            public:
                /// @brief Allocates and initializes an atomic count to one.
                constexpr counter()
                requires (DefaultConstructible<alloc_type>)
                {
                    _data = alloc_traits
                        ::allocate(_alloc, 1)
                        .unwrap();
                    
                    alloc_traits::construct(_alloc, _data, 1u);
                }

                /// @brief Allocates an atomic count using `alloc`.
                /// @param alloc Allocator used for the count allocation.
                constexpr counter(const alloc_type& alloc)
                    : _alloc{alloc}
                {
                    _data = alloc_traits
                        ::allocate(_alloc, 1)
                        .unwrap();
                    
                    alloc_traits::construct(_alloc, _data, 1u);
                }

                /// @brief Adopts a count and atomically increments it for a new owner.
                /// @param ptr Existing shared count.
                constexpr counter(atomic_usize* ptr)
                requires (DefaultConstructible<alloc_type>)
                    : _data{ptr}
                {
                    (*_data)++;
                }

                /// @brief Adopts a count with an explicit allocator and increments it.
                /// @param alloc Allocator responsible for the count.
                /// @param ptr Existing shared count.
                constexpr counter(const alloc_type& alloc, atomic_usize* ptr)
                    : _alloc{alloc}, _data{ptr}
                {
                    (*_data)++;
                }

                /// @brief Copies count metadata and atomically increments the count.
                /// @param other Counter whose count is shared.
                /// @brief Constructs a counter that owns no count.
                constexpr counter(empty_tag)
                requires (DefaultConstructible<alloc_type>)
                {}

                /// @brief Wraps the first word of a fused make_shared block.
                /// @details The count in that word is already one.
                constexpr counter(const alloc_type& alloc, atomic_usize* block, fused_tag)
                    : _alloc{alloc}, _fused{true}, _data{block}
                {}

                constexpr counter(const counter& other)
                    : _alloc{other._alloc}, _fused{other._fused}, _data{other._data}
                {
                    if (_data != nullptr) (*_data)++;
                }

                /// @brief Transfers count metadata and clears the source pointer.
                /// @param other Counter whose count is adopted.
                constexpr counter(counter&& other)
                    : _alloc{hsd::move(other._alloc)}, _fused{other._fused}, _data{other._data}
                {
                    other._data = nullptr;
                }

                /// @brief Shares `rhs`'s atomic count and increments it.
                /// @param rhs Counter whose count is shared.
                /// @return `*this` after sharing the count.
                constexpr counter& operator=(const counter& rhs)
                {
                    _alloc = rhs._alloc;
                    _fused = rhs._fused;
                    _data = rhs._data;
                    if (_data != nullptr) (*_data)++;
                    return *this;
                }

                /// @brief Transfers atomic count metadata from `rhs`.
                /// @param rhs Counter whose count is adopted.
                /// @return `*this` after the transfer.
                constexpr counter& operator=(counter&& rhs)
                {
                    hsd::swap(_alloc, rhs._alloc);
                    hsd::swap(_fused, rhs._fused);
                    hsd::swap(_data, rhs._data);
                    return *this;
                }

                /// @brief Returns mutable access to the atomic count value.
                /// @return Mutable reference to the shared atomic count.
                constexpr atomic_usize& operator*()
                {
                    return *_data;
                }

                /// @brief Destroys and deallocates the atomic count object.
                /// @return The allocator's deallocation result.
                /// @brief Tells whether releasing the count frees the object's block too.
                constexpr bool is_fused() const
                {
                    return _fused;
                }

                constexpr auto deallocate()
                {
                    // A fused block recorded its own length after the count.
                    const usize _words = _fused ? static_cast<usize>(_data[1]) : 1u;
                    alloc_traits::destroy(_alloc, _data);
                    return alloc_traits::deallocate(_alloc, _data, _words);
                }

                /// @brief Returns the allocation size used for the count object.
                /// @return Allocation size used for the count object.
                constexpr usize get_size() const
                {
                    return 1u;
                }

                /// @brief Returns the shared count address.
                /// @return Shared count address.
                constexpr auto* get_pointer() const
                {
                    return _data;
                }

                /// @brief Replaces the tracked count address.
                /// @param ptr New count address, usually null after release.
                constexpr void set_pointer(pointer_type ptr)
                {
                    _data = ptr;
                    if (ptr == nullptr) _fused = false;
                }
            };
            
        } // namespace shared_detail

        /// @brief Atomic allocator-aware shared owner.
        template <typename T, typename Allocator = allocator>
        class shared_ptr
        {
        private:
            /// @brief Object/array allocation metadata owned by this handle.
            using storage_type = shared_detail::storage<T, Allocator>;
            /// @brief Atomic count metadata shared by all owners.
            using counter_type = shared_detail::counter<Allocator>;

        public:
            /// @brief Allocator type retained by the control block.
            using alloc_type = Allocator;
            /// @brief Pointer to the first stored element.
            using pointer_type = typename storage_type::pointer_type;
            /// @brief Stored element type.
            using value_type = typename storage_type::value_type;
            /// @brief Mutable reference to a stored element.
            using reference_type = typename storage_type::value_type&;

        private:
            /// @brief Allocation metadata for the owned object or array.
            storage_type _value;
            /// @brief Shared atomic owner count.
            // An empty owner has no count; adopting constructors add one.
            counter_type _count{shared_detail::empty_tag{}};

            template < typename U, typename Alloc >
            friend class shared_ptr;

            /// @brief Atomically releases one owner and destroys at zero.
            constexpr void _delete()
            {
                if (_count.get_pointer() != nullptr)
                {
                    // The decrement itself says whether this was the last
                    // owner. Reading the count again afterwards would let two
                    // owners that release together both see zero and free the
                    // same allocation twice.
                    if ((*_count)-- == 1)
                    {
                        if (get() != nullptr) 
                        {
                            _value.destroy();
                        }
                        // A fused block owns the object's memory as well.
                        if (!_count.is_fused())
                        {
                            _value.deallocate().unwrap();
                        }
                        _count.deallocate().unwrap();
                    }

                    _value.set_pointer(nullptr);
                    _count.set_pointer(nullptr);
                }
            }

        public:
            /// @brief Constructs an empty owner.
            constexpr shared_ptr() = default;
            /// @brief Constructs an empty owner from the null sentinel.
            constexpr shared_ptr(null_type) {}

            /// @brief Adopts one existing object allocation.
            /// @param ptr Object allocation to manage.
            constexpr shared_ptr(pointer_type ptr) 
            requires (DefaultConstructible<alloc_type>)
                : _value{ptr, 1u}, _count{}
            {}

            /// @brief Adopts an existing object or array allocation.
            /// @param ptr Allocation base address.
            /// @param size Number of elements represented by the allocation.
            constexpr shared_ptr(pointer_type ptr, usize size) 
            requires (DefaultConstructible<alloc_type>)
                : _value{ptr, size}, _count{}
            {}

            /// @brief Allocates and default-constructs one object.
            /// @param alloc Allocator used for the object and count.
            constexpr shared_ptr(const alloc_type& alloc) 
                : _value{alloc, 1u}, _count{alloc}
            {}

            /// @brief Allocates and default-constructs an array.
            /// @param alloc Allocator used for the array and count.
            /// @param size Number of elements to allocate.
            constexpr shared_ptr(const alloc_type& alloc, usize size) 
                : _value{alloc, size}, _count{alloc}
            {}

            /// @brief Adopts storage and records its allocator and extent.
            /// @param ptr Allocation base address.
            /// @param alloc Allocator responsible for the allocation.
            /// @param size Number of represented elements.
            constexpr shared_ptr(pointer_type ptr, 
                const alloc_type& alloc, usize size) 
                : _value{ptr, alloc, size}, _count{alloc}
            {}

            /// @brief Shares ownership with a convertible owner.
            /// @param other Existing owner whose allocation and count are shared.
            template <typename U = T> 
            constexpr shared_ptr(const shared_ptr<U, Allocator>& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{other._value}, _count{other._count}
            {}

            /// @brief Moves ownership metadata from a convertible owner.
            /// @param other Owner whose allocation is transferred.
            template <typename U = T> 
            constexpr shared_ptr(shared_ptr<U, Allocator>&& other)
            requires(shared_detail::ConvertibleDerived<T, U>)
                : _value{hsd::move(other._value)}, _count{hsd::move(other._count)}
            {}

            /// @brief Releases this owner and its reference to the control block.
            /// @brief Adopts a fused block made by make_shared.
            /// @details Implementation detail: the block holds the count,
            ///          its own length, then the constructed object.
            shared_ptr(shared_detail::fused_tag, const alloc_type& alloc, atomic_usize* block)
                : _value{reinterpret_cast<pointer_type>(
                      block + shared_detail::fused_header_words), alloc, 1u},
                  _count{alloc, block, shared_detail::fused_tag{}}
            {}

            constexpr ~shared_ptr()
            {
                _delete();
            }

            /// @brief Releases ownership and becomes empty.
            /// @return `*this` after becoming empty.
            constexpr shared_ptr& operator=(null_type)
            {
                _delete();
                return *this;
            }

            /// @brief Copies the owner, sharing its allocation and count.
            /// @details Declared explicitly, because the user-declared copy-assignment
            /// below would otherwise deprecate the implicit copy constructor.
            constexpr shared_ptr(const shared_ptr&) = default;

            /// @brief Shares ownership with `rhs` after releasing this one.
            /// @details Declared explicitly: the implicit copy-assignment copies
            /// the count without releasing the previous owner, so assigning from
            /// a const lvalue leaked the old object.
            /// @param rhs Owner whose allocation and count are shared.
            /// @return `*this` after sharing ownership.
            constexpr shared_ptr& operator=(const shared_ptr& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            /// @brief Shares ownership with a convertible owner after releasing this one.
            /// @param rhs Owner whose allocation and count are shared.
            /// @return `*this` after sharing ownership.
            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            constexpr shared_ptr& operator=(shared_ptr<U, Allocator>& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = rhs._value;
                _count = rhs._count;
                return *this;
            }

            /// @brief Moves ownership from a convertible owner after releasing this one.
            /// @param rhs Owner whose allocation is transferred.
            /// @return `*this` after the transfer.
            template <typename U = T> 
            requires(shared_detail::ConvertibleDerived<T, U>)
            constexpr shared_ptr& operator=(shared_ptr<U, Allocator>&& rhs)
            {
                // Releasing first would drop the object being assigned.
                if (static_cast<const void*>(this) == static_cast<const void*>(&rhs))
                {
                    return *this;
                }

                _delete();
                _value = hsd::move(rhs._value);
                _count = hsd::move(rhs._count);
                return *this;
            }

            /// @brief Compares two owners by managed pointer identity.
            /// @param rhs Owner to compare.
            /// @return True when both handles point at the same address.
            constexpr bool operator==(const shared_ptr& rhs) const
            {
                return get() == rhs.get();
            }

            /// @brief Compares two owners by managed pointer identity.
            /// @param rhs Owner to compare.
            /// @return True when the managed pointers differ.
            constexpr bool operator!=(const shared_ptr& rhs) const
            {
                return get() != rhs.get();
            }

            /// @brief Tests whether this owner is empty.
            /// @return True when no object is managed.
            constexpr bool operator==(null_type) const
            {
                return get() == nullptr;
            }

            /// @brief Tests whether this owner is non-empty.
            /// @return True when an object is managed.
            constexpr bool operator!=(null_type) const
            {
                return get() != nullptr;
            }

            /// @brief Returns the managed pointer.
            /// @return Managed object or array address, possibly null.
            constexpr auto* get()
            {
                return _value.get_pointer();
            }

            /// @brief Returns the managed pointer for const access.
            /// @return Managed object or array address, possibly null.
            constexpr auto* get() const
            {
                return _value.get_pointer();
            }

            /// @brief Provides member access through the managed pointer.
            /// @return Managed object or array address.
            constexpr auto* operator->()
            {
                return get();
            }

            /// @brief Provides const member access through the managed pointer.
            /// @return Managed object or array address.
            constexpr auto* operator->() const
            {
                return get();
            }

            /// @brief Dereferences the managed object.
            /// @return Reference to the managed element.
            constexpr auto& operator*()
            {
                return *get();
            }

            /// @brief Dereferences the managed object for const access.
            /// @return Reference to the managed element.
            constexpr auto& operator*() const
            {
                return *get();
            }

            /// @brief Returns the current atomic owner count.
            /// @return Number of owners sharing the control block.
            constexpr usize get_count()
            {
                // An empty owner shares nothing.
                if (_count.get_pointer() == nullptr) return 0;
                return *_count;
            }

            /// @brief Tests whether this is the only owner.
            /// @return True when the atomic shared count is one.
            constexpr bool is_unique()
            {
                return get_count() == 1;
            }
        };

        namespace shared_detail
        {
            /// @brief Builds one object and its owner count in a single block.
            /// @details Layout: [count][block length in words][object]. The
            ///          count is released together with the block, so it
            ///          needs no allocation of its own.
            template < typename T, typename Allocator, typename... Args >
            inline shared_ptr<T, Allocator> make_fused(Allocator& alloc, Args&&... args)
            {
                using value_type = remove_array_t<T>;
                using count_traits = allocator_traits<Allocator, atomic_usize>;
                using value_traits = allocator_traits<Allocator, value_type>;

                constexpr usize _value_words =
                    (sizeof(value_type) + sizeof(atomic_usize) - 1) / sizeof(atomic_usize);
                constexpr usize _words = fused_header_words + _value_words;

                auto* _block = count_traits::allocate(alloc, _words).unwrap();
                count_traits::construct(alloc, _block, 1u);
                count_traits::construct(alloc, _block + 1, _words);

                auto* _object = reinterpret_cast<value_type*>(
                    _block + fused_header_words
                );
                value_traits::construct(alloc, _object, hsd::forward<Args>(args)...);

                return shared_ptr<T, Allocator>{fused_tag{}, alloc, _block};
            }
        } // namespace shared_detail

        /// @brief Maps object and array forms to the shared_ptr result type.
        template < typename T, typename Allocator >
        struct MakeShr
        {
            /// @brief Result type for one object allocation.
            using single_object = shared_ptr<T, Allocator>;
        };

        /// @brief Maps an unbounded array to its shared_ptr array result.
        template < typename T, typename Allocator >
        struct MakeShr<T[], Allocator>
        {
            /// @brief Result type for an array allocation.
            using array = shared_ptr<T[], Allocator>;
        };

        /// @brief Rejects bounded arrays, which do not have a runtime extent.
        template < typename T, usize N, typename Allocator >
        struct MakeShr<T[N], Allocator>
        {
            /// @brief Marker used to make bounded-array make_shared calls invalid.
            struct invalid_type {};  
        };

        /// @brief Allocates and constructs one atomic shared object.
        /// @tparam T Object type.
        /// @tparam Allocator Allocator type.
        /// @param args Constructor arguments forwarded to `T`.
        /// @return A shared owner for the newly constructed object.
        template < typename T, typename Allocator = allocator, typename... Args >
        requires (DefaultConstructible<Allocator> && IsSame<remove_array_t<T>, T>)
        static constexpr typename MakeShr<T, Allocator>::single_object 
        make_shared(Args&&... args)
        {
            Allocator _alloc;
            if constexpr (shared_detail::can_fuse<remove_array_t<T>>)
            {
                if (!is_constant_evaluated())
                {
                    return shared_detail::make_fused<T, Allocator>(
                        _alloc, hsd::forward<Args>(args)...
                    );
                }
            }

            using alloc_traits = allocator_traits<
                Allocator, remove_array_t<T>
            >;

            auto* _ptr = alloc_traits
                ::allocate(_alloc, 1)
                .unwrap();
            
            alloc_traits::construct(
                _alloc, _ptr, hsd::forward<Args>(args)...
            );
            
            return shared_ptr<T, Allocator>{_ptr, _alloc, 1};
        }

        /// @brief Allocates and constructs one object using an explicit allocator.
        /// @param alloc Allocator used for the object and control block.
        /// @param args Constructor arguments forwarded to `T`.
        /// @return A shared owner for the newly constructed object.
        template < typename T, typename Allocator = allocator, typename U, typename... Args >
        requires (IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
        static constexpr typename MakeShr<T, Allocator>::single_object 
        make_shared(U& alloc, Args&&... args)
        {
            if constexpr (shared_detail::can_fuse<remove_array_t<T>>)
            {
                if (!is_constant_evaluated())
                {
                    return shared_detail::make_fused<T, Allocator>(
                        alloc, hsd::forward<Args>(args)...
                    );
                }
            }

            using alloc_traits = allocator_traits<
                Allocator, remove_array_t<T>
            >;
            auto* _ptr = alloc_traits
                ::allocate(alloc, 1)
                .unwrap();

            alloc_traits::construct(
                alloc, _ptr, hsd::forward<Args>(args)...
            );
            return {_ptr, alloc, 1};
        }

        /// @brief Allocates and default-constructs an unbounded array.
        /// @param size Number of elements in the array.
        /// @return A shared owner for the newly allocated array.
        template < typename T, typename Allocator = allocator >
        requires (DefaultConstructible<Allocator> && !IsSame<remove_array_t<T>, T>)
        static constexpr typename MakeShr<T, Allocator>::array
        make_shared(usize size)
        {
            Allocator _alloc;
            return {_alloc, size};
        }

        /// @brief Allocates an unbounded array using an explicit allocator.
        /// @param alloc Allocator used for the array and control block.
        /// @param size Number of elements in the array.
        /// @return A shared owner for the newly allocated array.
        template < typename T, typename Allocator = allocator, typename U >
        requires (!IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
        static constexpr typename MakeShr<T, Allocator>::array
        make_shared(U& alloc, usize size)
        {
            return {alloc, size};
        }

        /// @brief Deleted overload preventing unsupported bounded-array objects.
        template < typename T, typename Allocator = allocator, typename... Args >
        static constexpr typename MakeShr<T, Allocator>::invalid_type 
        make_shared(Args&&...) = delete;
    }

    /// @brief Non-atomic allocator-aware shared owner alias.
    template < typename T, typename Allocator = allocator >
    using unsafe_shared_ptr = non_atomic_types::shared_ptr<T, Allocator>;
    /// @brief Atomic allocator-aware shared owner alias.
    template < typename T, typename Allocator = allocator >
    using safe_shared_ptr = atomic_types::shared_ptr<T, Allocator>;

    /// @brief Constructs a non-atomic shared object with a default allocator.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return A non-atomic shared owner.
    template < typename T, typename Allocator = allocator, typename... Args >
    static constexpr auto make_unsafe_shared(Args&&... args)
    {
        return non_atomic_types::make_shared<T, Allocator>(
            hsd::forward<Args>(args)...
        );
    }

    /// @brief Constructs an atomic shared object with a default allocator.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return An atomic shared owner.
    template < typename T, typename Allocator = allocator, typename... Args >
    static constexpr auto make_safe_shared(Args&&... args)
    {
        return atomic_types::make_shared<T, Allocator>(
            hsd::forward<Args>(args)...
        );
    }

    /// @brief Constructs a non-atomic shared object using `alloc`.
    /// @param alloc Allocator used for the object and control block.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return A non-atomic shared owner.
    template < typename T, typename Allocator = allocator, typename U, typename... Args >
    requires (!Constructible<T, U&, Args...>)
    static constexpr auto make_unsafe_shared(U& alloc, Args&&... args)
    {
        return non_atomic_types::make_shared<T, Allocator, U, Args...>(
            alloc, hsd::forward<Args>(args)...
        );
    }

    /// @brief Constructs an atomic shared object using `alloc`.
    /// @param alloc Allocator used for the object and control block.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return An atomic shared owner.
    template < typename T, typename Allocator = allocator, typename U, typename... Args >
    requires (!Constructible<T, U&, Args...>)
    static constexpr auto make_safe_shared(U& alloc, Args&&... args)
    {
        return atomic_types::make_shared<T, Allocator, U, Args...>(
            alloc, hsd::forward<Args>(args)...
        );
    }

    /// @brief Allocates an unbounded non-atomic shared array.
    /// @param size Number of array elements.
    /// @return A non-atomic shared array owner.
    template < typename T, typename Allocator = allocator >
    static constexpr auto make_unsafe_shared(usize size)
    {
        return non_atomic_types::make_shared<T, Allocator>(size);
    }

    /// @brief Allocates an unbounded atomic shared array.
    /// @param size Number of array elements.
    /// @return An atomic shared array owner.
    template < typename T, typename Allocator = allocator >
    static constexpr auto make_safe_shared(usize size)
    {
        return atomic_types::make_shared<T, Allocator>(size);
    }

    /// @brief Allocates a non-atomic shared array using `alloc`.
    /// @param alloc Allocator used for the array and control block.
    /// @param size Number of array elements.
    /// @return A non-atomic shared array owner.
    template < typename T, typename Allocator = allocator, typename U >
    static constexpr auto make_unsafe_shared(U& alloc, usize size)
    {
        return non_atomic_types::make_shared<T, Allocator, U>(alloc, size);
    }

    /// @brief Allocates an atomic shared array using `alloc`.
    /// @param alloc Allocator used for the array and control block.
    /// @param size Number of array elements.
    /// @return An atomic shared array owner.
    template < typename T, typename Allocator = allocator, typename U >
    static constexpr auto make_safe_shared(U& alloc, usize size)
    {
        return atomic_types::make_shared<T, Allocator, U>(alloc, size);
    }
}