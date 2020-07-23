#pragma once

#include "../Utility/Utility.hpp"

namespace hsd
{
    namespace non_atomic_types
    {
        template< typename T >
        class shared_ptr
        {
        private:
            using size_t = unsigned long int;

            struct _ptr_info
            {
                T _value;
                size_t _size = 1;
            } *_ptr = nullptr;

            constexpr void _destroy()
            {
                if(is_unique())
                {
                    delete _ptr;
                }
                else
                {
                    _ptr->_size--;
                    _ptr = nullptr;
                }
            }
        public:
            shared_ptr() = default;

            constexpr shared_ptr(const shared_ptr& ptr)
            {
                _ptr = ptr._ptr;
                _ptr->_size++;
            }

            constexpr shared_ptr(shared_ptr&& ptr)
            {
                _ptr = ptr._ptr;
                ptr._ptr = nullptr;
            }

            constexpr ~shared_ptr()
            {
                if(is_unique())
                {
                    delete _ptr;
                }
                else
                {
                    _ptr->_size--;
                    _ptr = nullptr;
                }
            }

            constexpr shared_ptr& operator=(shared_ptr& lhs)
            {
                if(_ptr != nullptr)
                {    
                    _destroy();
                }

                _ptr = lhs._ptr;
                _ptr->_size++;
                return *this;
            }

            constexpr shared_ptr& operator=(shared_ptr&& lhs)
            {
                if(_ptr != nullptr)
                {
                    _destroy();
                }
                _ptr = lhs._ptr;
                return *this;
            }

            constexpr T* get()
            {
                return &_ptr->_value;
            }

            constexpr T* get() const
            {
                return &_ptr->_value;
            }

            constexpr T* operator->()
            {
                return get();
            }

            constexpr T* operator->() const
            {
                return get();
            }

            constexpr T& operator*()
            {
                return *get();
            }

            constexpr T& operator*() const
            {
                return *get();
            }

            constexpr size_t get_size()
            {
                return _ptr->_size;
            }

            constexpr bool is_unique()
            {
                return get_size() == 1;
            }

            static constexpr shared_ptr make_shared()
            {
                shared_ptr __value;
                __value._ptr = new _ptr_info;
                return __value;
            }

            static constexpr shared_ptr make_shared(const T& val)
            {
                shared_ptr __value;
                __value._ptr = new _ptr_info;
                __value._ptr->_value = val;
                return __value;
            }

            template<typename... Args>
            static constexpr shared_ptr make_shared(Args&&... args)
            {
                shared_ptr __value;
                __value._ptr = new _ptr_info;
                __value._ptr->_value.~T();
                new (&__value._ptr->_value) 
                T(hsd::forward<Args>(args)...);
                return __value;
            }
        };
    }
}