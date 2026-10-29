#pragma once

/// @file Allocator.hpp
/// @brief Raw storage providers and the typed allocator customization layer.
/// @details Defines aligned allocation, buffered allocation, allocation errors,
/// and allocator_traits, which centralizes object lifetime and copy-selection
/// behavior for allocator-aware containers.

#include "Limits.hpp"
#include "Result.hpp"
#include "Types.hpp"
#include "Utility.hpp"

namespace hsd
{
    namespace allocator_detail
    {
        ///
        /// @brief Describes a failure reported by an allocator operation.
        ///
        class allocator_error
        {
        private:
            /// @brief Human-readable description of the failure.
            const char* _err = nullptr;

        public:
            ///
            /// @brief Builds an error from a static description string.
            ///
            /// @param error - Human-readable description of the failure.
            ///
            constexpr allocator_error(const char* error)
                : _err{error}
            {}

            ///
            /// @brief Gets the human-readable description of the failure.
            ///
            /// @return The description passed at construction.
            ///
            const char* pretty_error() const
            {
                return _err;
            }
        };

        ///
        /// @brief Metadata for one allocation region in a buffered allocator.
        ///
        /// A zero data offset marks a free block. The offset is stored instead
        /// of assuming that object data immediately follows the header, which
        /// allows each allocation to satisfy its own alignment requirement.
        ///
        struct block
        {
            /// @brief Size in bytes of the payload region following this header.
            usize size = 0;
            /// @brief Offset from this header to the user data, or 0 when free.
            usize data_offset = 0;
            /// @brief Next block header in address order, or `nullptr` at the end.
            block* next = nullptr;
        };

        ///
        /// @brief Rounds an address up to the requested power-of-two alignment.
        ///
        /// @param ptr - Address to align.
        /// @param alignment - Required address alignment.
        /// @return The first aligned address at or after `ptr`.
        ///
        static inline auto* align_pointer(uchar* ptr, usize alignment)
        {
            auto _address = reinterpret_cast<uptr>(ptr);
            _address = (_address + alignment - 1) & ~(alignment - 1);
            return reinterpret_cast<uchar*>(_address);
        }

        ///
        /// @brief Gets the user-visible address recorded by an allocated block.
        ///
        /// @param ptr - Block whose data address is requested.
        /// @return The allocated address, or `nullptr` when the block is free.
        ///
        static inline auto* get_block_data(block* ptr)
        {
            if (ptr->data_offset == 0) return static_cast<uchar*>(nullptr);

            return reinterpret_cast<uchar*>(ptr) + ptr->data_offset;
        }
    } // namespace allocator_detail

    ///
    /// @brief Allocator backed by `malloc` and `free`.
    ///
    /// `malloc` only guarantees fundamental alignment. Requests for types with
    /// extended alignment return an error instead of yielding invalid storage.
    ///
    struct mallocator
    {
        ///
        /// @brief Allocates uninitialized storage for a sequence of objects.
        ///
        /// @tparam T - Type stored in the allocation.
        /// @param size - Number of objects for which storage is requested.
        /// @return A pointer to the storage or an allocation error.
        ///
        template <typename T>
        [[nodiscard]] static inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
            if (size == 0) return static_cast<T*>(nullptr);

            if (size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for allocation"};
            }

            if constexpr (alignof(T) > alignof(max_align_t))
            {
                return allocator_detail::allocator_error{"Unsupported alignment"};
            }

            T* _result = static_cast<T*>(malloc(sizeof(T) * size));

            if (_result == nullptr)
            {
                return allocator_detail::allocator_error{"No space left in RAM"};
            }
            else
            {
                return _result;
            }
        }

        ///
        /// @brief Releases storage previously returned by `allocate<T>`.
        ///
        /// @param ptr - Start of the allocation, or `nullptr`.
        /// @return An empty error option.
        ///
        static inline auto deallocate(void* ptr, usize)
            -> option_err<allocator_detail::allocator_error>
        {
            free(ptr);
            return {};
        }

        /// @brief Compares two `mallocator` handles.
        /// @return Always `true`, since the type is stateless.
        friend constexpr bool operator==(mallocator, mallocator) = default;
    };    

    ///
    /// @brief Stateless allocator backed by aligned `operator new`.
    ///
    class allocator
    {
    private:
        ///
        /// @brief Original allocation properties needed by aligned delete.
        ///
        /// A pointer to this header is stored immediately before the aligned
        /// object storage. This makes deallocation independent of the static
        /// pointer type, which is required by polymorphic smart pointers.
        ///
        struct allocation_header
        {
            /// @brief Total size in bytes of the underlying `operator new` allocation.
            usize size;
            /// @brief Alignment the underlying `operator new` allocation was made with.
            usize alignment;
        };

    public:
        ///
        /// @brief Allocates uninitialized storage for a sequence of objects.
        ///
        /// The returned address satisfies the alignment requirement of `T`.
        /// The caller owns the storage and must release it with `deallocate`.
        /// Allocation metadata permits deallocation through a compatible
        /// pointer whose static type differs from the originally allocated type.
        ///
        /// @tparam T - Type stored in the allocation.
        /// @param size - Number of objects for which storage is requested.
        /// @return A pointer to the storage or an allocation error.
        ///
        template <typename T>
        [[nodiscard]] static inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
            if (size == 0) return static_cast<T*>(nullptr);

            #ifndef __cpp_aligned_new
            if constexpr (alignof(T) > alignof(max_align_t))
            {
                return allocator_detail::allocator_error{"Unsupported alignment"};
            }
            #endif

            constexpr auto _allocation_alignment =
                alignof(T) > alignof(max_align_t) ?
                alignof(T) : alignof(max_align_t);
            constexpr auto _metadata_size =
                sizeof(allocation_header) + sizeof(allocation_header*);
            constexpr auto _overhead =
                _metadata_size + _allocation_alignment - 1;

            if (size > (limits<usize>::max - _overhead) / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for allocation"};
            }

            auto _alloc_size = _overhead + size * sizeof(T);
            auto* _header = static_cast<allocation_header*>(
                new_allocate(_alloc_size, _allocation_alignment)
            );

            if (_header == nullptr)
            {
                return allocator_detail::allocator_error{"No space left in RAM"};
            }

            hsd::construct_at(
                _header,
                allocation_header{_alloc_size, _allocation_alignment}
            );

            auto* _data = allocator_detail::align_pointer(
                reinterpret_cast<uchar*>(_header) + _metadata_size,
                _allocation_alignment
            );
            auto** _header_slot = reinterpret_cast<allocation_header**>(
                _data - sizeof(allocation_header*)
            );
            *_header_slot = _header;

            return reinterpret_cast<T*>(_data);
        }

        ///
        /// @brief Releases storage previously returned by `allocate<T>`.
        ///
        /// This function does not destroy objects. Their lifetimes must end
        /// before the storage is released.
        ///
        /// @tparam T - Static type of the pointer being released.
        /// @param ptr - Start of the allocation, or `nullptr`.
        /// @return An empty error option.
        ///
        template <typename T>
        static inline auto deallocate(T* ptr, usize)
            -> option_err<allocator_detail::allocator_error>
        {
            if (ptr == nullptr) return {};

            auto** _header_slot = reinterpret_cast<allocation_header**>(
                reinterpret_cast<uchar*>(ptr) - sizeof(allocation_header*)
            );
            auto* _header = *_header_slot;

            delete_deallocate(
                _header, _header->size, _header->alignment
            );

            return {};
        }

        /// @brief Compares two `allocator` handles.
        /// @return Always `true`, since the type is stateless.
        friend constexpr bool operator==(allocator, allocator) = default;
    };

    ///
    /// @brief Non-owning allocator over a caller-provided byte buffer.
    ///
    /// Constructing an allocator initializes the supplied buffer. Copies and
    /// moves are lightweight handles to the same block list, so the buffer must
    /// outlive every allocator and allocation referring to it.
    ///
    class buffered_allocator
    {
    private:
        /// @brief Start of the external buffer this handle allocates from.
        uchar* _buf = nullptr;
        /// @brief Usable size in bytes of `_buf` after alignment adjustment.
        usize _size = 0;
        /// @brief Block header type used to track free/allocated regions.
        using block_type = allocator_detail::block;

        ///
        /// @brief Gets the first block header, stored at the start of `_buf`.
        ///
        /// @return The first block header.
        ///
        auto* _first_block() const
        {
            return reinterpret_cast<block_type*>(_buf);
        }
        
    public:
        ///
        /// @brief Initializes an allocator over an external buffer.
        ///
        /// Any previous allocation state in the buffer is discarded. Create
        /// additional handles by copying this allocator instead of constructing
        /// another allocator over the same memory.
        ///
        /// @param buf - Start of the external storage.
        /// @param size - Size of the external storage in bytes.
        ///
        inline buffered_allocator(uchar* buf, usize size)
        {
            if (buf == nullptr)
            {
                panic("Buffer is nullptr");
            }

            auto* _aligned_buf = allocator_detail::align_pointer(
                buf, alignof(block_type)
            );
            auto _prefix_size = static_cast<usize>(_aligned_buf - buf);

            if (size <= _prefix_size + sizeof(block_type))
            {
                panic("Buffer is too small to contain data");
            }

            _buf = _aligned_buf;
            _size = size - _prefix_size;

            hsd::construct_at(
                _first_block(),
                block_type{_size - sizeof(block_type), 0, nullptr}
            );
        }

        /// @brief Copies a handle to the same underlying buffer.
        buffered_allocator(const buffered_allocator&) = default;
        /// @brief Takes over a handle to the same underlying buffer.
        buffered_allocator(buffered_allocator&&) = default;
        /// @brief Copies a handle to the same underlying buffer.
        /// @return A reference to this allocator.
        buffered_allocator& operator=(const buffered_allocator&) = default;
        /// @brief Takes over a handle to the same underlying buffer.
        /// @return A reference to this allocator.
        buffered_allocator& operator=(buffered_allocator&&) = default;

        ///
        /// @brief Allocates aligned, uninitialized storage from the buffer.
        ///
        /// The allocator uses first fit and splits free blocks when the
        /// remainder can hold another block header and at least one byte.
        ///
        /// @tparam T - Type stored in the allocation.
        /// @param size - Number of objects for which storage is requested.
        /// @return A pointer to the storage or an allocation error.
        ///
        template <typename T>
        [[nodiscard]] inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
            if (size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for allocation"};
            }

            if (size == 0) return static_cast<T*>(nullptr);

            auto _data_size = size * sizeof(T);

            for (auto* _block = _first_block();
                _block != nullptr; _block = _block->next)
            {
                if (_block->data_offset != 0) continue;

                auto* _payload = reinterpret_cast<uchar*>(
                    _block + 1
                );
                auto* _data = allocator_detail::align_pointer(
                    _payload, alignof(T)
                );
                auto _padding = static_cast<usize>(_data - _payload);

                if (
                    _padding > _block->size ||
                    _data_size > _block->size - _padding)
                {
                    continue;
                }

                auto* _next_address = allocator_detail::align_pointer(
                    _data + _data_size, alignof(block_type)
                );
                auto _consumed_size = static_cast<usize>(
                    _next_address - _payload
                );

                if (
                    _consumed_size <= _block->size &&
                    _block->size - _consumed_size > sizeof(block_type))
                {
                    auto* _next_block = reinterpret_cast<block_type*>(
                        _next_address
                    );
                    auto _next_size = _block->size -
                        _consumed_size - sizeof(block_type);

                    hsd::construct_at(
                        _next_block,
                        block_type{_next_size, 0, _block->next}
                    );

                    _block->size = _consumed_size;
                    _block->next = _next_block;
                }

                _block->data_offset = static_cast<usize>(
                    _data - reinterpret_cast<uchar*>(_block)
                );

                return reinterpret_cast<T*>(_data);
            }

            return allocator_detail::allocator_error{"Insufficient memory"};
        }

        ///
        /// @brief Releases an allocation and coalesces neighboring free blocks.
        ///
        /// This function does not destroy objects. Their lifetimes must end
        /// before the storage is released.
        ///
        /// @tparam T - Type used when the storage was allocated.
        /// @param ptr - Start of the allocation, or `nullptr`.
        /// @return An empty option or an error for an unknown pointer.
        ///
        template <typename T>
        inline auto deallocate(T* ptr, usize)
            -> option_err<allocator_detail::allocator_error>
        {
            if (ptr == nullptr)
            {
                return {};
            }

            auto* _raw_ptr = reinterpret_cast<uchar*>(ptr);

            if (
                _raw_ptr < _buf + sizeof(block_type) ||
                _raw_ptr >= _buf + _size)
            {
                return allocator_detail::allocator_error{"Pointer out of bounds"};
            }

            block_type* _previous = nullptr;

            for (auto* _block = _first_block();
                _block != nullptr; _block = _block->next)
            {
                if (allocator_detail::get_block_data(_block) == _raw_ptr)
                {
                    _block->data_offset = 0;

                    if (
                        _block->next != nullptr &&
                        _block->next->data_offset == 0)
                    {
                        _block->size += sizeof(block_type) +
                            _block->next->size;
                        _block->next = _block->next->next;
                    }

                    if (
                        _previous != nullptr &&
                        _previous->data_offset == 0)
                    {
                        _previous->size += sizeof(block_type) + _block->size;
                        _previous->next = _block->next;
                    }

                    return {};
                }

                _previous = _block;
            }

            return allocator_detail::allocator_error{"Pointer was not allocated by this buffer"};
        }

        ///
        /// @brief Compares whether two handles refer to the same buffer.
        ///
        friend constexpr bool operator==(
            const buffered_allocator& lhs,
            const buffered_allocator& rhs)
        {
            return lhs._buf == rhs._buf && lhs._size == rhs._size;
        }
    };

    ///
    /// @brief Typed adapter shared by allocator-aware containers.
    ///
    /// Allocators only need to provide `allocate<T>(count)` and
    /// `deallocate(pointer, count)`. Specialize this class when an allocator
    /// requires custom object construction, destruction, or copy selection.
    ///
    /// @tparam Allocator - Raw storage provider.
    /// @tparam T - Object type stored in the allocation.
    ///
    template <typename Allocator, typename T>
    struct allocator_traits
    {
        /// @brief Raw storage provider forwarded to.
        using allocator_type = Allocator;
        /// @brief Object type stored in the allocation.
        using value_type = T;

        ///
        /// @brief Allocates storage for `size` objects through the allocator.
        ///
        /// @param alloc - Allocator to allocate through.
        /// @param size - Number of objects for which storage is requested.
        /// @return A pointer to the storage or an allocation error.
        ///
        [[nodiscard]] static constexpr auto allocate(
            allocator_type& alloc, usize size)
        {
            return alloc.template allocate<value_type>(size);
        }

        ///
        /// @brief Releases typed storage through the allocator that created it.
        ///
        /// @param alloc - Allocator to release through.
        /// @param ptr - Start of the allocation, or `nullptr`.
        /// @param size - Number of objects the allocation was made for.
        /// @return An empty error option, or the allocator's own error on failure.
        ///
        static constexpr auto deallocate(
            allocator_type& alloc, value_type* ptr, usize size)
        {
            return alloc.deallocate(ptr, size);
        }

        ///
        /// @brief Starts an object lifetime in allocated storage.
        ///
        /// @tparam Args - Constructor argument types.
        /// @param alloc - Allocator associated with the storage.
        /// @param ptr - Address at which the object is constructed.
        /// @param args - Arguments forwarded to the object constructor.
        /// @return The address of the constructed object.
        ///
        template <typename... Args>
        static constexpr auto* construct(
            [[maybe_unused]] allocator_type& alloc, value_type* ptr, Args&&... args)
        {
            return hsd::construct_at(ptr, hsd::forward<Args>(args)...);
        }

        ///
        /// @brief Ends the lifetime of one object without releasing storage.
        ///
        /// @param alloc - Allocator associated with the storage.
        /// @param ptr - Object whose lifetime ends, or `nullptr` to do nothing.
        ///
        static constexpr void destroy(
            [[maybe_unused]] allocator_type& alloc, value_type* ptr)
        {
            if (ptr != nullptr) ptr->~value_type();
        }

        ///
        /// @brief Selects the allocator used by a copied container.
        ///
        /// The default preserves the source allocator. A specialization may
        /// return a distinct allocator when its resource requires that policy.
        ///
        /// @param alloc - Allocator of the container being copied.
        /// @return The allocator to use for the new container.
        ///
        static constexpr allocator_type select_on_container_copy_construction(
            const allocator_type& alloc)
        {
            return alloc;
        }

        ///
        /// @brief Tests whether storage can be exchanged between allocators.
        ///
        /// Allocators without equality support are treated as distinct so
        /// storage is never transferred without an explicit guarantee.
        ///
        /// @param lhs - First allocator to compare.
        /// @param rhs - Second allocator to compare.
        /// @return `true` if storage may be exchanged between `lhs` and `rhs`,
        /// `false` otherwise.
        ///
        static constexpr bool equal(
            const allocator_type& lhs, const allocator_type& rhs)
        {
            if constexpr (requires { lhs == rhs; })
            {
                return lhs == rhs;
            }
            else
            {
                return false;
            }
        }
    };
}