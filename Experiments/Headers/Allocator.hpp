#pragma once

#include "Limits.hpp"
#include "Result.hpp"
#include "Types.hpp"
#include "Utility.hpp"


namespace hsd_test
{
    using namespace hsd;

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

            const char* pretty_error() const
            {
                return _err;
            }
        };
    } // namespace allocator_detail

    struct mallocator
    {
        template <typename T>
        [[nodiscard]] static inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
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

        static inline void deallocate(void* ptr)
        {
            free(ptr);
        }
    };    

    class allocator
    {
    private:
        struct core_alloc_unit
        {
            usize size;
            usize alignment;
        };

        template <typename T>
        static inline auto* _get_data(core_alloc_unit* ptr)
        {
            return reinterpret_cast<T*>(
                reinterpret_cast<uchar*>(ptr) + sizeof(core_alloc_unit)
            );
        }

        template <typename T>
        static constexpr auto _get_max_align()
        {
            return sizeof(usize) > sizeof(T) ? sizeof(usize) : sizeof(T);
        }

    public:
        template <typename T>
        [[nodiscard]] static inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
            using pointer_type = core_alloc_unit*;
            using value_type = core_alloc_unit;

            if (size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for allocation"};
            }
            else
            {
                auto _align = _get_max_align<T>();
                auto _alloc_size = sizeof(value_type) + size * sizeof(T);
                auto* _result = reinterpret_cast<pointer_type>(
                    new_allocate(_alloc_size, _align)
                );

                _result->size = _alloc_size;
                _result->alignment = _align;

                if (_result == nullptr)
                {
                    return allocator_detail::allocator_error{"No space left in RAM"};
                }
                else
                {
                    return _get_data<T>(_result);
                }
            }
        }

        template <typename T>
        static inline void deallocate(T* ptr)
        {
            using pointer_type = core_alloc_unit*;
            using value_type = core_alloc_unit;

            auto* _alloc_value = reinterpret_cast<pointer_type>(
                reinterpret_cast<uchar*>(ptr) - sizeof(value_type)
            );

            delete_deallocate(
                _alloc_value, _alloc_value->size, _alloc_value->alignment
            );
        }
    };

    class buffered_allocator
    {
    private:
        uchar* _buf = nullptr;
        usize _size = 0;

        // thanks qookie
        struct block 
        {
            u64 in_use : 1;
            u64 size : 8 * sizeof(u64) - 1;
        };

        static inline auto* _get_next(block* ptr, usize sz)
        {
            return reinterpret_cast<block*>(
                reinterpret_cast<uchar*>(ptr) + sz + sizeof(block)
            );
        }

        template <typename T>
        static inline auto* _get_data(block* ptr)
        {
            return reinterpret_cast<T*>(
                reinterpret_cast<uchar*>(ptr) + sizeof(block)
            );
        }
        
    public:
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

        inline buffered_allocator(const buffered_allocator& other)
            : _buf{other._buf}, _size{other._size}
        {}
        
        inline buffered_allocator(buffered_allocator&& other)
        {
            _buf = exchange(other._buf, nullptr);
            _size = exchange(other._size, 0u);
        }

        inline buffered_allocator& operator=(const buffered_allocator& other)
        {
            _buf = other._buf;
            _size = other._size;

            return *this;
        }

        inline buffered_allocator& operator=(buffered_allocator&& other)
        {
            _buf = exchange(other._buf, nullptr);
            _size = exchange(other._size, 0u);

            return *this;
        }

        template <typename T>
        [[nodiscard]] inline auto allocate(usize size)
            -> result<T*, allocator_detail::allocator_error>
        {
            size *= sizeof(T);
            usize _free_size = 0;
            auto* _block_ptr = reinterpret_cast<block*>(_buf);
            block* _prev_block = nullptr;

            if (size > _size)
            {
                return allocator_detail::allocator_error {
                    "No space left in buffer"
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

                        return {_get_data<T>(_block_ptr)};
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

                        return {_get_data<T>(_prev_block)};
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

            return allocator_detail::allocator_error{"Insufficient memory"};
        }

        template <typename T>
        inline auto deallocate(T* ptr, usize)
            -> option_err<allocator_detail::allocator_error>
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
    };
}