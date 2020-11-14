#pragma once

#include "Limits.hpp"
#include "StackArray.hpp"
#include <new>

namespace hsd
{
    class bad_alloc
        : public std::exception
    {
        virtual const char* what() const noexcept override
        {
            return "Bad length for \'::operator new(usize)\'";
        }
    };

    template <typename T>
    struct allocator
    {
        allocator() = default;

        template <typename U>
        constexpr allocator(const allocator<U>&)
        {}

        [[nodiscard]] constexpr auto allocate(usize size)
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                throw std::bad_array_new_length();
            }
            else
            {
                #ifdef __cpp_aligned_new
                return static_cast<T*>(::operator new(size * sizeof(T), std::align_val_t(alignof(T))));
                #else
                return static_cast<T*>(::operator new(size * sizeof(T)));
                #endif
            }
        }

        constexpr void deallocate(T* ptr, usize size)
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                throw std::bad_array_new_length();
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
    private:
    
        static stack_array<T, MaxSize> _buf;
        usize _ptr_index = 0;

        constexpr auto end_buf()
        {
            return &_buf[MaxSize];
        }
    public:

        [[nodiscard]] constexpr auto allocate(usize size)
        {
            usize _old_index = _ptr_index;
            _ptr_index += size;
            return &_buf[_old_index];
        }

        constexpr void deallocate(T* ptr, usize size)
        {
            //_ptr_index -= size;
        }
    };
} // mamespace hsd