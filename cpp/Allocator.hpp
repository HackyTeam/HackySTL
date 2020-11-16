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
    class allocator
    {
    protected:
        T* _data = nullptr;

    public:
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

    template < typename T, usize MaxSize >
    class constexpr_allocator
    {
    protected:
        using data_type = stack_array<T, MaxSize>;
        data_type _data;

    public:
        using pointer_type = T*;
        using value_type = T;

        [[nodiscard]] constexpr auto allocate(usize)
        {
            return &_data[0];
        }

        constexpr void deallocate(data_type&, usize)
        {}
    };
} // mamespace hsd