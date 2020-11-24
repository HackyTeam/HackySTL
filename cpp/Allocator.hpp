#pragma once

#include "Limits.hpp"
#include "StackArray.hpp"
#include <string.h>
#include <new>

namespace hsd
{
    class bad_alloc
        : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            return "Bad length for Allocation";
        }
    };

    class bad_dealloc
        : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            return "Bad length for Deallocation";
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
            usize size{};
            bool in_use{};
            uchar data[];
        };

    protected:
        T* _data = nullptr;

    public:
        using pointer_type = T*;
        using value_type = T;

        //constexpr buffered_allocator() = default;

        constexpr buffered_allocator(uchar* buf, usize size)
            : _buf{buf}, _size{size}
        {}

        template <typename U>
        constexpr buffered_allocator(const buffered_allocator<U>& other)
            : _buf{other._buf}, _size{other._size}
        {}
        
        template <typename U>
        constexpr buffered_allocator(buffered_allocator<U>&& other)
        {
            _buf = exchange(other._buf, nullptr);
            _size = exchange(other._size, 0u);
        }

        template <typename U>
        constexpr buffered_allocator& operator=(const buffered_allocator<U>& other)
        {
            _buf = other._buf;
            _size = other._size;
        }

        [[nodiscard]] constexpr T* allocate(usize size)
        {
            auto* _block_ptr = reinterpret_cast<block*>(_buf);
            auto* _block_end = reinterpret_cast<block*>(_buf + _size - sizeof(block));
            auto* _block_back = reinterpret_cast<block*>(_buf);
            usize _free_size = 0u;
            
            while (_block_ptr < _block_end) 
            {
                if (!_block_ptr->in_use)
                {
                    if(_block_ptr->size == 0) 
                    {
                        _block_ptr->size = size * sizeof(T);
                        _block_ptr->in_use = true;
                        return reinterpret_cast<T*>(_block_ptr->data);
                    }
                    else if(_block_ptr->size >= size * sizeof(T))
                    {
                        _block_ptr->in_use = true;
                        return reinterpret_cast<T*>(_block_ptr->data);
                    }
                    else
                    {
                        _free_size = static_cast<usize>(_block_ptr - _block_back) + _block_ptr->size;

                        if(_free_size >= size * sizeof(T))
                        {
                            memset(
                                reinterpret_cast<uchar*>(_block_back) + 
                                sizeof(usize) + 1u, 0, _free_size
                            );
                            _block_back->size = _free_size;
                            _block_back->in_use = true;
                            return reinterpret_cast<T*>(_block_back->data);
                        }
                    }
                }
                else
                {
                    _free_size = 0u;
                    _block_back = _block_ptr;
                }
                
                _block_ptr = reinterpret_cast<block *>(
                    reinterpret_cast<uchar*>(_block_ptr) + 
                    _block_ptr->size + sizeof(block)
                );
            }

            return nullptr;
        }

        void deallocate(T* ptr, usize)
        {
            if(ptr != nullptr)
            {
                auto* _block_ptr = reinterpret_cast<block*>(
                    reinterpret_cast<uchar*>(ptr) - (sizeof(usize) + 1u)
                );

                _block_ptr->in_use = false;
            }
        }
    };

    template <typename T>
    class allocator
    {
    protected:
        T* _data = nullptr;

    public:
        using pointer_type = T*;
        using value_type = T;
        allocator() = default;
        constexpr allocator(const allocator&) {}
        constexpr allocator(allocator&&) {}

        template <typename U>
        constexpr allocator(const allocator<U>&) {}
        template <typename U>
        constexpr allocator(allocator<U>&&) {}

        [[nodiscard]] constexpr auto* allocate(usize size)
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                throw bad_alloc();
            }
            else
            {
                #ifdef __cpp_aligned_new
                return static_cast<pointer_type>(::operator new(size * sizeof(T), std::align_val_t(alignof(T))));
                #else
                return static_cast<pointer_type>(::operator new(size * sizeof(T)));
                #endif
            }
        }

        constexpr void deallocate(pointer_type ptr, usize size)
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                throw bad_dealloc();
            }
            else
            {
                #ifdef __cpp_sized_deallocation
                ::operator delete(ptr, size * sizeof(T), std::align_val_t(alignof(T)));
                #else
                ::operator delete(ptr, std::align_val_t(alignof(T)));
                #endif
            }
        }
    };

    template < typename T, usize MaxSize >
    class constexpr_allocator
    {
    protected:
        using data_type = stack_array<T, MaxSize>;
        data_type _data;

    public:
        using pointer_type = data_type;
        using value_type = T;

        constexpr_allocator() = default;
        constexpr_allocator(const constexpr_allocator&) = delete;
        constexpr_allocator(constexpr_allocator&&) = delete;

        [[nodiscard]] constexpr auto& allocate(usize)
        {
            return _data;
        }

        constexpr void deallocate(const data_type&, usize)
        {}

        constexpr void deallocate(const T*, usize)
        {}
    };
} // mamespace hsd