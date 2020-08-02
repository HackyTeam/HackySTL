#pragma once

#include "Utility.hpp"

namespace hsd
{
    namespace unique_detail
    {
        template<typename T>
        struct deleter
        {
            template< typename U = T, std::enable_if_t<!std::is_array_v<U>, int> = 0 >
            void operator()(remove_array_t<T>*& ptr)
            {
                delete ptr;
                ptr = nullptr;
            }

            template< typename U = T, std::enable_if_t<std::is_array_v<U>, int> = 0 >
            void operator()(remove_array_t<T>*& ptr)
            {
                delete[] ptr;
                ptr = nullptr;
            }
        };
    } // namespace unique_detail

    template< typename T, typename Deleter = unique_detail::deleter<T> >
    class unique_ptr
    {
    private:
        struct _storage : private Deleter
        {
            remove_array_t<T>* _ptr = nullptr;

            constexpr Deleter& get_deleter()
            {
                return *this;
            }
        } _value;

    public:
        using pointer = T*;
        using reference = T&;
        using nullptr_t = decltype(nullptr);
        using remove_array_pointer = remove_array_t<T>*;
        
        unique_ptr() = default;
        constexpr unique_ptr(nullptr_t) {}
        unique_ptr(unique_ptr&) = delete;
        unique_ptr(const unique_ptr&) = delete;

        template< typename U = T, std::enable_if_t<!std::is_array_v<U>, int> = 0 >
        constexpr unique_ptr(pointer ptr)
        {
            _value._ptr = ptr;
        }

        template< typename U = T, std::enable_if_t<std::is_array_v<U>, int> = 0 >
        constexpr unique_ptr(remove_array_pointer ptr)
        {
            _value._ptr = ptr;
        }

        constexpr unique_ptr(unique_ptr&& other)
        {
            _value = other._value;
            other._value._ptr = nullptr;
        }

        constexpr ~unique_ptr()
        {
            _value.get_deleter()(_value._ptr);
        }

        constexpr unique_ptr& operator=(nullptr_t)
        {
            ~unique_ptr();
            return *this;
        }

        constexpr unique_ptr& operator=(unique_ptr&& rhs)
        {
            ~unique_ptr();
            _value = rhs._value;
            rhs._value._ptr = nullptr;
            return *this;
        }

        constexpr Deleter& get_deleter()
        {
            return _value.get_deleter();
        }

        constexpr remove_array_pointer get()
        {
            return _value._ptr;
        }

        constexpr remove_array_pointer get() const
        {
            return _value._ptr;
        }

        constexpr pointer operator->()
        {
            return get();
        }

        constexpr pointer operator->() const
        {
            return get();
        }

        constexpr reference operator*()
        {
            return *get();
        }

        constexpr reference operator*() const
        {
            return *get();
        }
    };

    template<typename T>
    struct MakeUniq
    {
        using single_object = unique_ptr<T>;
    };

    template<typename T>
    struct MakeUniq<T[]>
    {
        using array = unique_ptr<T[]>;
    };
    
    template<typename T, size_t N>
    struct MakeUniq<T[N]>
    {
        struct invalid_type {};  
    };
    
    template<typename T, typename... Args>
    static constexpr typename MakeUniq<T>::single_object 
    make_unique(Args&&... args)
    {
        return unique_ptr<T>(new T(hsd::forward<Args>(args)...));
    }

    template<typename T>
    static constexpr typename MakeUniq<T>::array 
    make_unique(size_t size)
    {
        using ptr_type = remove_array_t<T>;
        return unique_ptr<T>(new ptr_type[size]());
    }

    template<typename T, typename... Args>
    static constexpr typename MakeUniq<T>::invalid_type 
    make_unique(Args&&...) = delete;
} // namespace hsd
