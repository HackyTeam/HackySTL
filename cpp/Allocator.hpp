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
        using pointer_type = T*;
        using value_type = T;
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

    class buffered_allocator
    {
    protected:
        usize _buf_pos = 0;

    public:
        constexpr usize get_pos() const
        {
            return _buf_pos;
        }
    };
    

    template < typename T, usize MaxSize >
    class constexpr_allocator
        : public buffered_allocator
    {
    private:
        stack_array<T, MaxSize> _buf;

    public:
        using pointer_type = const T*;
        using value_type = const T;

        [[nodiscard]] constexpr auto allocate(usize size)
        {
            usize _old_pos = _buf_pos;
            _buf_pos += size;
            return &_buf[_old_pos];
        }

        constexpr auto get_data()
        {
            return _buf.data();
        }

        constexpr void deallocate(pointer_type ptr, usize size)
        {}
    };
} // mamespace hsd