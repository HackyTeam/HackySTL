#pragma once

/// @file UniquePtr.hpp
/// @brief Move-only allocator-aware unique ownership for objects and arrays.
/// @details Owns allocated storage exclusively, performs typed destruction,
/// supports derived-to-base moves, and preserves custom allocator state.

#include "../Base/Allocator.hpp"
#include "../Base/Concepts.hpp"

namespace hsd
{
    namespace unique_detail
    {
        /// @brief Accepts conversions from a derived unique owner to its base.
        template <typename T, typename U>
        concept ConvertibleDerived = Convertible<U, T> || std::is_base_of_v<T, U>;

        /// @brief Allocator-backed object or array storage owned by a unique_ptr.
        /// @tparam T Object type, possibly an unbounded array type.
        /// @tparam Allocator Allocator used to construct and release elements.
        template < typename T, typename Allocator >
        class storage
        {
        public:
            /// @brief Allocator type retained by the storage.
            using alloc_type = Allocator;
            /// @brief Pointer to the first stored element.
            using pointer_type = remove_array_t<T>*;
            /// @brief Element type constructed in the allocation.
            using value_type = remove_array_t<T>;

        private:
            /// @brief Allocator operations used for element lifetime management.
            using alloc_traits = allocator_traits<alloc_type, value_type>;
            /// @brief Allocator retained until the allocation is released.
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

            /// @brief Adopts an existing allocation and element count with a default allocator.
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

                for(usize _index = 0; _index < size; _index++)
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

            /// @brief Deleted: storage is exclusively owned and never copied.
            constexpr storage(const storage&) = delete;

            /// @brief Transfers compatible storage metadata and nulls the source.
            /// @param other Compatible storage whose allocation is adopted.
            template <typename U = T>
            constexpr storage(storage<U, Allocator>&& other)
                : _alloc{hsd::move(other._alloc)}, 
                _data{hsd::exchange(other._data, nullptr)},
                _size{hsd::exchange(other._size, 0)}
            {}

            /// @brief Releases this storage's allocation, then adopts `rhs`'s.
            /// @param rhs Compatible storage whose allocation is transferred; left null.
            /// @return `*this` after the transfer.
            template <typename U = T>
            constexpr storage& operator=(storage<U, Allocator>&& rhs)
            {
                deallocate().unwrap();
                hsd::swap(rhs._alloc, _alloc);
                _data = hsd::exchange(rhs._data, nullptr);
                hsd::swap(_size, rhs._size);
                return *this;
            }

            /// @brief Releases the allocation through the retained allocator.
            /// @details Does not destroy elements; callers must invoke `destroy()`
            /// first if the elements are still live.
            /// @return The allocator's deallocation result.
            constexpr auto deallocate()
            {
                return alloc_traits::deallocate(_alloc, _data, _size);
            }

            /// @brief Destroys represented elements in reverse order.
            /// @details Does not deallocate storage; callers must invoke
            /// `deallocate()` separately to release the allocation.
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
            /// @return Allocation base address, or null when empty.
            constexpr auto* get_pointer() const
            {
                return _data;
            }

            /// @brief Replaces the tracked allocation address.
            /// @param ptr New allocation address, usually null after release.
            constexpr void set_pointer(pointer_type ptr)
            {
                _data = ptr;
            }

            /// @brief Replaces the represented element count.
            /// @param size New element count.
            constexpr void set_size(usize size)
            {
                _size = size;
            }
        };
    } // namespace unique_detail

    /// @brief Move-only allocator-aware unique owner.
    /// @tparam T Object type, possibly an unbounded array type.
    /// @tparam Allocator Allocator used to construct and release the owned element(s).
    template <typename T, typename Allocator = allocator>
    class unique_ptr
    {
    private:
        /// @brief Object/array allocation metadata owned by this handle.
        using storage_type = unique_detail::storage<T, Allocator>;
        /// @brief Allocation, allocator, and size state exclusively owned by this handle.
        storage_type _value;

        template <typename U, typename Alloc>
        friend class unique_ptr;

    public:
        /// @brief Allocator type retained by the storage.
        using alloc_type = Allocator;
        /// @brief Pointer to the managed object or array's first element.
        using pointer_type = typename storage_type::pointer_type;
        /// @brief Managed element type.
        using value_type = typename storage_type::value_type;
        /// @brief Mutable reference to the managed element.
        using reference_type = typename storage_type::value_type&;

    private:
        /// @brief Destroys and deallocates any owned allocation, then nulls the pointer.
        /// @details Safe to call on an empty handle; destruction and deallocation
        /// are skipped or produce a no-op result respectively.
        constexpr void _delete()
        {
            if (get() != nullptr)
            {
                _value.destroy();
            }
            
            _value.deallocate().unwrap();
            _value.set_pointer(nullptr);
        }

    public:     
        /// @brief Constructs an empty owner.
        constexpr unique_ptr() = default;
        /// @brief Constructs an empty owner from the null sentinel.
        constexpr unique_ptr(null_type) {}
        /// @brief Deleted: unique ownership cannot be copied.
        unique_ptr(unique_ptr&) = delete;
        /// @brief Deleted: unique ownership cannot be copied.
        unique_ptr(const unique_ptr&) = delete;

        /// @brief Adopts an existing object allocation with a default-constructed allocator.
        /// @param ptr Object allocation to manage.
        constexpr unique_ptr(pointer_type ptr) 
            : _value{ptr, 1u}
        {}

        /// @brief Adopts an existing object or array allocation with a default-constructed allocator.
        /// @param ptr Allocation base address.
        /// @param size Number of elements represented by the allocation.
        constexpr unique_ptr(pointer_type ptr, usize size) 
            : _value{ptr, size}
        {}

        /// @brief Allocates and default-constructs one object.
        /// @param alloc Allocator used for the object.
        constexpr unique_ptr(const alloc_type& alloc) 
            : _value{alloc, 1u}
        {}

        /// @brief Allocates and default-constructs an array.
        /// @param alloc Allocator used for the array.
        /// @param size Number of elements to allocate.
        constexpr unique_ptr(const alloc_type& alloc, usize size) 
            : _value{alloc, size}
        {}

        /// @brief Adopts an allocation together with its responsible allocator.
        /// @param ptr Allocation base address.
        /// @param alloc Allocator responsible for the allocation.
        /// @param size Number of represented elements.
        constexpr unique_ptr(pointer_type ptr, 
            const alloc_type& alloc, usize size) 
            : _value{ptr, alloc, size}
        {}

        /// @brief Moves ownership from a convertible owner, leaving it empty.
        /// @param other Owner whose allocation, allocator, and size are transferred.
        template <typename U = T> 
        requires(unique_detail::ConvertibleDerived<T, U>)
        constexpr unique_ptr(unique_ptr<U, Allocator>&& other)
            : _value{hsd::move(other._value)}
        {}

        /// @brief Destroys and releases any owned allocation.
        constexpr ~unique_ptr()
        {
            _delete();
        }

        /// @brief Releases any owned allocation and becomes empty.
        /// @return `*this` after becoming empty.
        constexpr unique_ptr& operator=(null_type)
        {
            _delete();
            return *this;
        }

        /// @brief Releases this owner's allocation, then adopts `rhs`'s.
        /// @param rhs Convertible owner whose allocation is transferred; left empty.
        /// @return `*this` after the transfer.
        template <typename U = T> 
        requires(unique_detail::ConvertibleDerived<T, U>)
        constexpr unique_ptr& operator=(unique_ptr<U, Allocator>&& rhs)
        {
            _delete();
            _value = hsd::move(rhs._value);
            return *this;
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

        /// @brief Tests whether this owner is non-empty.
        /// @return True when an object is managed.
        constexpr bool operator!=(null_type) const
        {
            return _value.get_pointer() != nullptr;
        }

        /// @brief Tests whether this owner is empty.
        /// @return True when no object is managed.
        constexpr bool operator==(null_type) const
        {
            return _value.get_pointer() == nullptr;
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

        /// @brief Dereferences the managed pointer.
        /// @return Reference to the managed element.
        constexpr auto& operator*()
        {
            return *get();
        }

        /// @brief Dereferences the managed pointer for const access.
        /// @return Const reference to the managed element.
        constexpr auto& operator*() const
        {
            return *get();
        }
    };

    /// @brief Maps unique_ptr object and array forms to their result type.
    /// @tparam T Requested object, unbounded array, or bounded array type.
    /// @tparam Allocator Allocator type used by the resulting unique_ptr.
    template < typename T, typename Allocator >
    struct MakeUniq
    {
        /// @brief Result type for one object allocation.
        using single_object = unique_ptr<T, Allocator>;
    };

    /// @brief Maps an unbounded array to its unique_ptr array result.
    template < typename T, typename Allocator >
    struct MakeUniq<T[], Allocator>
    {
        /// @brief Result type for an array allocation.
        using array = unique_ptr<T[], Allocator>;
    };
    
    /// @brief Rejects bounded arrays, which do not have a runtime extent.
    template < typename T, usize N, typename Allocator >
    struct MakeUniq<T[N], Allocator>
    {
        /// @brief Marker used to make bounded-array make_unique calls invalid.
        struct invalid_type {};  
    };
    
    /// @brief Allocates and constructs one object with a default-constructed allocator.
    /// @tparam T Object type.
    /// @tparam Allocator Allocator type.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return A unique owner for the newly constructed object.
    template < typename T, typename Allocator = allocator, typename... Args >
    requires (DefaultConstructible<Allocator> && IsSame<remove_array_t<T>, T>)
    static constexpr typename MakeUniq<T, Allocator>::single_object 
    make_unique(Args&&... args)
    {
        Allocator _alloc;
        using alloc_traits = allocator_traits<Allocator, remove_array_t<T>>;
        auto* _ptr = alloc_traits
            ::allocate(_alloc, 1)
            .unwrap();
        
        alloc_traits::construct(_alloc, _ptr, hsd::forward<Args>(args)...);
        return unique_ptr<T, Allocator>{_ptr, _alloc, 1};
    }

    /// @brief Allocates and constructs one object using an explicit allocator.
    /// @param alloc Allocator used for the object; copied into the resulting owner.
    /// @param args Constructor arguments forwarded to `T`.
    /// @return A unique owner for the newly constructed object.
    template < typename T, typename Allocator = allocator, typename U, typename... Args >
    requires (IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
    static constexpr typename MakeUniq<T, Allocator>::single_object 
    make_unique(U& alloc, Args&&... args)
    {
        using alloc_traits = allocator_traits<Allocator, remove_array_t<T>>;
        auto* _ptr = alloc_traits
            ::allocate(alloc, 1)
            .unwrap();
        
        alloc_traits::construct(alloc, _ptr, hsd::forward<Args>(args)...);
        return unique_ptr<T, Allocator>{_ptr, alloc, 1};
    }

    /// @brief Allocates and default-constructs an unbounded array with a default-constructed allocator.
    /// @param size Number of elements in the array.
    /// @return A unique owner for the newly allocated array.
    template < typename T, typename Allocator = allocator >
    requires (DefaultConstructible<Allocator> && !IsSame<remove_array_t<T>, T>)
    static constexpr typename MakeUniq<T, Allocator>::array 
    make_unique(usize size)
    {
        Allocator _alloc;
        return {_alloc, size};
    }

    /// @brief Allocates an unbounded array using an explicit allocator.
    /// @param alloc Allocator used for the array; copied into the resulting owner.
    /// @param size Number of elements in the array.
    /// @return A unique owner for the newly allocated array.
    template < typename T, typename Allocator = allocator, typename U >
    requires (!IsSame<remove_array_t<T>, T> && IsSame<Allocator, U>)
    static constexpr typename MakeUniq<T, Allocator>::array 
    make_unique(U& alloc, usize size)
    {
        return {alloc, size};
    }

    /// @brief Deleted overload preventing unsupported bounded-array objects.
    /// @details Bounded arrays (e.g. `T[N]`) have no runtime-configurable extent,
    /// so this overload is disabled via `MakeUniq`'s `invalid_type` marker.
    template <typename T, typename Allocator = allocator, typename... Args>
    static constexpr typename MakeUniq<T, Allocator>::invalid_type 
    make_unique(const Allocator&, Args&&...) = delete;
} // namespace hsd
