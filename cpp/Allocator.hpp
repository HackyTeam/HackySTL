#pragma once

#include "Result.hpp"
#include "Math.hpp"
#include "StackArray.hpp"
#include <string.h>
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
            bool in_use{};
            u32 size{};
            uchar data[];
        };

    public:
        using pointer_type = T*;
        using value_type = T;

        inline buffered_allocator(uchar* buf, usize size)
            : _buf{buf}, _size{size}
        {}

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
            new (ptr) T(forward<Args>(args)...);
        }

        [[nodiscard]] constexpr auto allocate(usize size)
            -> Result< T*, allocator_detail::allocator_error >
        {
            auto* _block_ptr = bit_cast<block*>(_buf);
            auto* _block_end = bit_cast<block*>(
                _buf + _size - 
                _block_ptr->size - 
                sizeof(block)
            );

            auto* _block_back = bit_cast<block*>(_buf);
            usize _free_size = 0u;
            
            while (_block_ptr < _block_end) 
            {
                if (!_block_ptr->in_use)
                {
                    if (_block_ptr->size == 0) 
                    {
                        _block_ptr->size = size * sizeof(T);
                        _block_ptr->in_use = true;
                        return bit_cast<T*>(_block_ptr->data);
                    }
                    else if (_block_ptr->size >= size * sizeof(T))
                    {
                        _block_ptr->in_use = true;
                        return bit_cast<T*>(_block_ptr->data);
                    }
                    else
                    {
                        if (_block_back->in_use == true)
                            _block_back = _block_ptr;

                        _free_size = static_cast<usize>(_block_ptr - _block_back) + _block_ptr->size;

                        if (_free_size >= size * sizeof(T))
                        {
                            _block_back->size = _free_size;
                            _block_back->in_use = true;
                            return bit_cast<T*>(_block_back->data);
                        }
                    }
                }
                else
                {
                    _free_size = 0u;
                    _block_back = bit_cast<block*>(
                        bit_cast<uchar*>(_block_ptr) + 
                        _block_ptr->size + sizeof(block)
                    );
                }
                
                _block_ptr = bit_cast<block*>(
                    bit_cast<uchar*>(_block_ptr) + 
                    _block_ptr->size + sizeof(block)
                );
            }

            return allocator_detail::allocator_error{"Insufficient memory"};
        }

        auto deallocate(T* ptr, usize)
            -> Result< void, allocator_detail::allocator_error >
        {
            if (ptr != nullptr)
            {
                if (bit_cast<uchar*>(ptr) >= _buf && bit_cast<uchar*>(ptr) < _buf + _size)
                {
                    auto* _block_ptr = bit_cast<block*>(
                        bit_cast<uchar*>(ptr) - sizeof(block)
                    );

                    _block_ptr->in_use = false;
                }
                else
                {
                    return allocator_detail::allocator_error{"Pointer out of bounds"};
                }
            }

            return {};
        }

        #ifndef NDEBUG
        void print_buffer() const
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
                return {static_cast<pointer_type>(::operator new(
                    size * _type_size, static_cast<std::align_val_t>(_alignment))), ok_value{}};
                #else
                return {static_cast<pointer_type>(::operator new(size * _type_size)), ok_value{}};
                #endif
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
            new (ptr) T(forward<Args>(args)...);
        }
    };
} // namespace hsd
