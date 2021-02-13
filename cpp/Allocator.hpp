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
            HSD_CONSTEXPR allocator_error(const char* error)
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
            usize size{};
            bool in_use{};
            uchar data[];
        };

    protected:
        T* _data = nullptr;

    public:
        using pointer_type = T*;
        using value_type = T;

        HSD_CONSTEXPR buffered_allocator(uchar* buf, usize size)
            : _buf{buf}, _size{size}
        {}

        template <typename U>
        HSD_CONSTEXPR buffered_allocator(const buffered_allocator<U>& other)
            : _buf{other._buf}, _size{other._size}
        {}
        
        template <typename U>
        HSD_CONSTEXPR buffered_allocator(buffered_allocator<U>&& other)
        {
            _buf = exchange(other._buf, nullptr);
            _size = exchange(other._size, 0u);
        }

        template <typename U>
        HSD_CONSTEXPR buffered_allocator& operator=(const buffered_allocator<U>& other)
        {
            _buf = other._buf;
            _size = other._size;
        }

        [[nodiscard]] constexpr auto allocate(usize size)
            -> Result< T*, allocator_detail::allocator_error >
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
                        if(_block_back->in_use == true)
                            _block_back = _block_ptr;

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
                    _block_back = reinterpret_cast<block*>(
                        reinterpret_cast<uchar*>(_block_ptr) + 
                        _block_ptr->size + sizeof(block)
                    );
                }
                
                _block_ptr = reinterpret_cast<block*>(
                    reinterpret_cast<uchar*>(_block_ptr) + 
                    _block_ptr->size + sizeof(block)
                );
            }

            return allocator_detail::allocator_error{"Insufficient memory"};
        }

        auto deallocate(T* ptr, usize)
            -> Result< void, allocator_detail::allocator_error >
        {
            if(ptr != nullptr)
            {
                if(reinterpret_cast<uchar*>(ptr) >= _buf && 
                    reinterpret_cast<uchar*>(ptr) < _buf + _size)
                {
                    auto* _block_ptr = reinterpret_cast<block*>(
                        reinterpret_cast<uchar*>(ptr) - (sizeof(usize) + 1u)
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

        void print_buffer() const
        {
            usize _vlen = static_cast<usize>(math::sqrt(static_cast<f64>(_size)));
            usize _hlen = _vlen + (_size - _vlen * _vlen);

            for (usize _hindex = 0; _hindex < _hlen; _hindex++)
            {
                for (usize _vindex = 0; _vindex < _vlen; _vindex++)
                    printf("%02x ", _buf[_hindex * _hlen + _vindex]);

                puts("");
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
        HSD_CONSTEXPR allocator() = default;
        HSD_CONSTEXPR allocator(const allocator&) = delete;
        HSD_CONSTEXPR allocator(allocator&&) = delete;

        template <typename U>
        HSD_CONSTEXPR allocator(const allocator<U>&) = delete;
        template <typename U>
        HSD_CONSTEXPR allocator(allocator<U>&&) = delete;

        [[nodiscard]] inline auto allocate(usize size)
            -> Result< T*, allocator_detail::allocator_error >
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for allocation"};
            }
            else
            {
                return static_cast<pointer_type>(::operator new(size * sizeof(T)));
            }
        }

        inline auto deallocate(pointer_type ptr, usize size)
            -> Result< void, allocator_detail::allocator_error >
        {
            if(size > limits<usize>::max / sizeof(T))
            {
                return allocator_detail::allocator_error{"Bad length for deallocation"};
            }
            else
            {
                ::operator delete(ptr);
                return {};
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

        HSD_CONSTEXPR constexpr_allocator() = default;
        HSD_CONSTEXPR constexpr_allocator(const constexpr_allocator&) = delete;
        HSD_CONSTEXPR constexpr_allocator(constexpr_allocator&&) = delete;

        static constexpr usize limit()
        {
            return MaxSize;
        }
    };
} // namespace hsd
