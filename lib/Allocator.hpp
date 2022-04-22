#pragma once

#include "Result.hpp"
#include "Math.hpp"
#include "StackArray.hpp"

#include <malloc.h>
#include <new>

namespace hsd
{
    namespace allocator_detail
    {
        class allocator_error
        {
        private:
            const char* _err = nullptr;

        public:
            constexpr allocator_error(const char* error)
                : _err{error}
            {}

            const char* operator()() const
            {
                return _err;
            }
        };
    } // namespace allocator_detail
    
    struct mallocator
    {
        template <typename T, typename... Args>
        static inline void construct_at(T* ptr, Args&&... args)
        {
            new (ptr) T{forward<Args>(args)...};
        }

        template <typename T, typename... Args>
        [[nodiscard]] static inline auto allocate_single(Args&&... args)
            -> Result< T*, allocator_detail::allocator_error >
        {
            T* _result = static_cast<T*>(malloc(sizeof(T)));

            if (_result == nullptr)
            {
                return {allocator_detail::allocator_error{"No space left in RAM"}, err_value{}};
            }
            else
            {
                construct_at(_result, forward<Args>(args)...);
                return {_result, ok_value{}};
            }
        }

        template <typename T, typename U, usize N>
        [[nodiscard]] static inline auto allocate_multiple(usize size, const U (&arr)[N])
            -> Result< T*, allocator_detail::allocator_error >
        {
            if (size > limits<usize>::max / sizeof(T))
            {
                return {allocator_detail::allocator_error{"Bad length for allocation"}, err_value{}};
            }
            else
            {
                T* _result = static_cast<T*>(malloc(sizeof(T) * size));

                if (_result == nullptr)
                {
                    return {allocator_detail::allocator_error{"No space left in RAM"}, err_value{}};
                }
                else
                {
                    if (size < N)
                    {
                        return {allocator_detail::allocator_error{"Array too small"}, err_value{}};
                    }
                    else
                    {
                        usize _index = 0;

                        for (; _index < N; _index++)
                        {
                            construct_at(_result + _index, arr[_index]);
                        }
                        for (; _index < size; _index++)
                        {
                            construct_at(_result + _index);
                        }
                    }

                    return {_result, ok_value{}};
                }
            }
        }

        template <typename T, typename... Args>
        [[nodiscard]] static inline auto allocate_multiple(usize size, Args&&... args)
            -> Result< T*, allocator_detail::allocator_error >
        {
            if (size > limits<usize>::max / sizeof(T))
            {
                return {allocator_detail::allocator_error{"Bad length for allocation"}, err_value{}};
            }
            else
            {
                T* _result = static_cast<T*>(malloc(sizeof(T) * size));

                if (_result == nullptr)
                {
                    return {allocator_detail::allocator_error{"No space left in RAM"}, err_value{}};
                }
                else
                {
                    if (size < sizeof...(args))
                    {
                        return {allocator_detail::allocator_error{"Array too small"}, err_value{}};
                    }
                    else
                    {
                        usize _index = sizeof...(args);

                        [&]<usize... Ints>(hsd::index_sequence<Ints...>)
                        {
                            (construct_at(_result + Ints, forward<Args>(args)), ...);
                        }(index_sequence_for<Args...>{});

                        for (; _index < size; _index++)
                        {
                            construct_at(_result + _index);
                        }
                    }

                    return {_result, ok_value{}};
                }
            }
        }

        static inline void deallocate(void* ptr)
        {
            free(ptr);
        }
    };    

    template <typename T>
    class buffered_allocator
    {
    private:
        uchar* _buf = nullptr;
        usize _size = 0;

        template <typename U>
        friend class buffered_allocator;

        // thanks qookie
        struct block 
        {
            u64 in_use : 1;
            u64 size : 8 * sizeof(u64) - 1;
        };

        auto* _get_next(block* ptr, usize sz)
        {
            return reinterpret_cast<block*>(
                reinterpret_cast<uchar*>(ptr) + sz + sizeof(block)
            );
        }

        auto* _get_data(block* ptr)
        {
            return reinterpret_cast<T*>(
                reinterpret_cast<uchar*>(ptr) + sizeof(block)
            );
        }
        
    public:
        using pointer_type = T*;
        using value_type = T;

        inline buffered_allocator(uchar* buf, usize size)
            : _buf{buf}, _size{size}
        {
            if (_buf == nullptr)
            {
                hsd::panic("Buffer is nullptr");
            }
            else if (_size <= sizeof(block))
            {
                hsd::panic("Buffer is too small to contain data");
            }
            else
            {
                block* _block = reinterpret_cast<block*>(_buf);

                if (_block->in_use == 0 && _block->size == 0)
                {
                    _block->size = _size - sizeof(block);
                }
            }
        }

        template <typename U>
        inline buffered_allocator(const buffered_allocator<U>& other)
            : _buf{other._buf}, _size{other._size}
        {}
        
        template <typename U>
        inline buffered_allocator(buffered_allocator<U>&& other)
        {
            _buf = exchange(other._buf, nullptr);
            _size = exchange(other._size, 0u);
        }

        template <typename U>
        inline buffered_allocator& operator=(const buffered_allocator<U>& other)
        {
            _buf = other._buf;
            _size = other._size;
        }

        template <typename... Args>
        static inline void construct_at(T* ptr, Args&&... args)
        {
            new (ptr) T{forward<Args>(args)...};
        }

        [[nodiscard]] inline auto allocate(usize size)
            -> Result< T*, allocator_detail::allocator_error >
        {
            size *= sizeof(T);
            usize _free_size = 0;
            auto* _block_ptr = reinterpret_cast<block*>(_buf);
            block* _prev_block = nullptr;

            if (size > _size)
            {
                return {
                    allocator_detail::allocator_error{
                        "No space left in buffer"
                    }, err_value{}
                };
            }

            while (reinterpret_cast<uchar*>(_block_ptr) < _buf + _size)
            {
                if (_block_ptr->in_use == 0)
                {
                    if (_block_ptr->size >= size + sizeof(block))
                    {
                        _block_ptr->in_use = 1;

                        if (_block_ptr->size > size)
                        {
                            auto* _next_block = _get_next(_block_ptr, size);
                            _next_block->in_use = 0;

                            _next_block->size = {
                                _block_ptr->size - size - sizeof(block)
                            };

                            _block_ptr->size = size;
                        }

                        return {_get_data(_block_ptr), ok_value{}};
                    }
                    else if (_block_ptr->size >= size)
                    {
                        _block_ptr->in_use = 1;
                    }
                    else
                    {
                        if (_prev_block == nullptr)
                        {
                            _prev_block = _block_ptr;
                            _free_size += _block_ptr->size;
                        }
                        else
                        {
                            _free_size += _block_ptr->size;
                            _free_size += sizeof(block);
                        }

                        _block_ptr = _get_next(
                            _block_ptr, _block_ptr->size
                        );
                    }
                }
                else if (_prev_block != nullptr)
                {
                    if (_free_size >= size)
                    {
                        _prev_block->in_use = 1;
                        _prev_block->size = size;

                        if (_free_size > size + sizeof(block))
                        {
                            auto* _next_block = _get_next(_prev_block, size);

                            _next_block->in_use = 0;
                            _next_block->size = {
                                _free_size - size - sizeof(block)
                            };
                        }

                        return {_get_data(_prev_block), ok_value{}};
                    }
                    else
                    {
                        _free_size = 0;
                        _prev_block = nullptr;

                        _block_ptr = _get_next(
                            _block_ptr, _block_ptr->size
                        );
                    }
                }
                else
                {
                    _block_ptr = _get_next(
                        _block_ptr, _block_ptr->size
                    );
                }
            }

            return {
                allocator_detail::allocator_error{
                    "Insufficient memory"
                }, err_value{}
            };
        }

        inline auto deallocate(T* ptr, usize)
            -> Result< void, allocator_detail::allocator_error >
        {
            if (ptr != nullptr)
            {
                if (
                    reinterpret_cast<uchar*>(ptr) >= _buf && 
                    reinterpret_cast<uchar*>(ptr) < _buf + _size)
                {
                    auto* _block_ptr = reinterpret_cast<block*>(
                        reinterpret_cast<uchar*>(ptr) - sizeof(block)
                    );
                    
                    _block_ptr->in_use = 0;
                }
                else
                {
                    return allocator_detail::allocator_error{"Pointer out of bounds"};
                }
            }

            return {};
        }

        #ifndef NDEBUG
        inline void print_buffer() const
        {
            usize _vlen = static_cast<usize>(
                math::sqrt(static_cast<f64>(_size))
            );

            usize _hlen = _vlen + (_size - _vlen * _vlen);

            for (usize _hindex = 0; _hindex < _hlen; _hindex++)
            {
                for (usize _vindex = 0; _vindex < _vlen; _vindex++)
                    printf("%02x ", _buf[_hindex * _hlen + _vindex]);

                putc('\n', stdout);
            }
            
        }
        #endif
    };

    template <typename T>
    class allocator
    {
    private:
        usize _type_size = sizeof(T);
        usize _alignment = alignof(T);
        
        template <typename U>
        friend class allocator;

    public:
        using pointer_type = T*;
        using value_type = T;
        inline allocator() = default;

        template <typename U = T>
        inline allocator(const allocator<U>& other)
            : _type_size{other._type_size}, _alignment{other._alignment}
        {}

        template <typename U = T>
        inline allocator& operator=(const allocator<U>& rhs)
        {
            _type_size = rhs._type_size;
            _alignment = rhs._alignment;
            return *this;
        }

        [[nodiscard]] inline auto allocate(usize size)
            -> Result< T*, allocator_detail::allocator_error >
        {
            if (size > limits<usize>::max / sizeof(T))
            {
                return {allocator_detail::allocator_error{"Bad length for allocation"}, err_value{}};
            }
            else
            {
                #ifdef __cpp_aligned_new
                T* _result = static_cast<pointer_type>(::operator new(
                    size * _type_size, static_cast<std::align_val_t>(_alignment), std::nothrow
                ));
                #else
                T* _result = static_cast<pointer_type>(
                    ::operator new(size * _type_size, std::nothrow)
                );
                #endif

                if (_result == nullptr)
                {
                    return {allocator_detail::allocator_error{"No space left in RAM"}, err_value{}};
                }
                else
                {
                    return {_result, ok_value{}};
                }
            }
        }

        inline auto deallocate(pointer_type ptr, usize size)
            -> Result< void, allocator_detail::allocator_error >
        {
            if (size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for deallocation"};
            }
            else
            {
                #ifdef __cpp_sized_deallocation
                ::operator delete(ptr, size * _type_size, static_cast<std::align_val_t>(_alignment));
                #else
                ::operator delete(ptr, static_cast<std::align_val_t>(_alignment));
                #endif

                return {};
            }
        }

        template <typename... Args>
        static inline void construct_at(T* ptr, Args&&... args)
        {
            new (ptr) T{forward<Args>(args)...};
        }
    };
} // namespace hsd
