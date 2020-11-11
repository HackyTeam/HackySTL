#pragma once

#include "Utility.hpp"
#include "Types.hpp"

namespace hsd
{
    namespace unique_detail
    {
        template <typename T, typename U>
        concept Derived = std::is_base_of_v<T, U>;
        template <typename T>
        concept ArrayPointer = is_array<T>::value;
        template <typename T>
        concept ValuePointer = !ArrayPointer<T>;

        template <typename T>
        struct deleter
        {
            HSD_CONSTEXPR void operator()(T*& ptr) requires(ValuePointer<T>)
            {
                delete ptr;
                ptr = nullptr;
            }

            HSD_CONSTEXPR void operator()(remove_array_t<T>*& ptr) requires(ArrayPointer<T>)
            {
                delete[] ptr;
                ptr = nullptr;
            }
        };

        template <typename T, typename Deleter>
        struct storage : private Deleter
        {
            remove_array_t<T>* _ptr = nullptr;

            constexpr Deleter& get_deleter()
            {
                return *this;
            }
        };

    } // namespace unique_detail

    template < typename T, typename Deleter = unique_detail::deleter<T> >
    class unique_ptr
    {
    private:
        unique_detail::storage<T, Deleter> _value;

        template <typename U, typename Del>
        friend class unique_ptr;

        HSD_CONSTEXPR void _delete()
        {
            _value.get_deleter()(_value._ptr);
        }

    public:
        template <typename U>
        using pointer = U*;
        
        template <typename U>
        using reference = U&;

        template <typename U>
        using remove_array_pointer = remove_array_t<U>*;
        
        unique_ptr() = default;
        constexpr unique_ptr(NullType) {}
        unique_ptr(unique_ptr&) = delete;
        unique_ptr(const unique_ptr&) = delete;
;
        constexpr unique_ptr(pointer<T> ptr) requires(unique_detail::ValuePointer<T>)
        {
            _value._ptr = ptr;
        }

        constexpr unique_ptr(remove_array_pointer<T> ptr) requires(unique_detail::ArrayPointer<T>)
        {
            _value._ptr = ptr;
        }

        constexpr unique_ptr(unique_ptr&& other)
        {
            _value._ptr = other._value._ptr;
            other._value._ptr = nullptr;
        }

        template <typename U> requires(unique_detail::Derived<T, U>)
        constexpr unique_ptr(unique_ptr<U>&& other)
        {
            _value._ptr = other._value._ptr;
            other._value._ptr = nullptr;
        }

        HSD_CONSTEXPR ~unique_ptr()
        {
            _delete();
        }

        HSD_CONSTEXPR unique_ptr& operator=(NullType)
        {
            _delete();
            return *this;
        }

        HSD_CONSTEXPR unique_ptr& operator=(unique_ptr&& rhs)
        {
            _delete();
            _value._ptr = rhs._value._ptr;
            rhs._value._ptr = nullptr;
            return *this;
        }

        template <typename U> requires(unique_detail::Derived<T, U>)
        HSD_CONSTEXPR unique_ptr& operator=(unique_ptr<U>&& rhs)
        {
            _delete();
            _value._ptr = rhs._value._ptr;
            rhs._value._ptr = nullptr;
            return *this;
        }

        constexpr Deleter& get_deleter()
        {
            return _value.get_deleter();
        }

        constexpr remove_array_pointer<T> get()
        {
            return _value._ptr;
        }

        constexpr remove_array_pointer<T> get() const
        {
            return _value._ptr;
        }

        constexpr bool operator!=(NullType) const
        {
            return _value._ptr != nullptr;
        }

        constexpr pointer<T> operator->()
        {
            return get();
        }

        constexpr pointer<T> operator->() const
        {
            return get();
        }

        constexpr reference<T> operator*()
        {
            return *get();
        }

        constexpr reference<T> operator*() const
        {
            return *get();
        }
    };

    template <typename T>
    struct MakeUniq
    {
        using single_object = unique_ptr<T>;
    };

    template <typename T>
    struct MakeUniq<T[]>
    {
        using array = unique_ptr<T[]>;
    };
    
    template <typename T, usize N>
    struct MakeUniq<T[N]>
    {
        struct invalid_type {};  
    };
    
    template <typename T, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T>::single_object 
    make_unique(Args&&... args)
    {
        return unique_ptr<T>(new T(hsd::forward<Args>(args)...));
    }

    template <typename T>
    static HSD_CONSTEXPR typename MakeUniq<T>::array 
    make_unique(usize size)
    {
        using ptr_type = remove_array_t<T>;
        return unique_ptr<T>(new ptr_type[size]{});
    }

    template <typename T, typename... Args>
    static HSD_CONSTEXPR typename MakeUniq<T>::invalid_type 
    make_unique(Args&&...) = delete;
} // namespace hsd
