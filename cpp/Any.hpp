#pragma once

// define HSD_ANY_ENABLE_TYPEINFO to enable type_info

#ifdef HSD_ANY_ENABLE_TYPEINFO
#   include <typeinfo>
#endif

#include <type_traits>
#include <exception>

#include "Types.hpp"
#include "Utility.hpp"
#include "UniquePtr.hpp"

namespace hsd
{
    template<typename T>
    concept Copyable = std::is_copy_constructible_v<T>;

    struct bad_any_cast : public std::exception 
    {
        virtual const char* what() const noexcept override 
        {
            return "Illegal casting:\n"
            "Cannot cast into different type";
        }
    };

    class _any_base 
    {
    public:
        virtual ~_any_base() = default;
        virtual hsd::unique_ptr<_any_base> clone() const = 0;

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        virtual const std::type_info& get_typeinfo() const noexcept = 0;
        #endif
    };

    template<Copyable T>
    class _any_derived : public _any_base 
    {
        T _value;
        friend class any;

    public:
        constexpr _any_derived(T value) : _value(move(value)) {}

        hsd::unique_ptr<_any_base> clone() const override 
        {
            return hsd::make_unique<_any_derived>(_value);
        }

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        const std::type_info& get_typeinfo() const noexcept override 
        {
            return typeid(T);
        }
        #endif
    };

    class any
    {
    private:
        hsd::unique_ptr<_any_base> _data = nullptr;

    public:
        constexpr any() noexcept = default;

        template<Copyable T>
        constexpr any(T other)
        {
            _data = hsd::make_unique<_any_derived<T>>(move(other));
        }

        any(const any& other)
        {
            _data = other._data->clone();
        }

        any(any&& other)
        {
            this->swap(other);
        }

        ~any() = default;

        any& operator=(any rhs)
        {
            this->swap(rhs);
            return *this;
        }

        template<typename T>
        constexpr auto cast_to() const
        {
            typedef typename std::remove_pointer<T>::type type;

            if(auto* p_base = dynamic_cast<_any_derived<type>*>(_data.get()))
            {
                return static_cast<T>(p_base->_value);
            }
            else
            {
                throw bad_any_cast();
            }
        }

        template<class T>
        T* cast_if() const
        {
            if (auto* p_base = dynamic_cast<_any_derived<T>*>(_data.get()))
                return &p_base->_value;

            return nullptr;
        }

        template<class T>
        bool holds_type() const
        {
            return dynamic_cast<_any_derived<T>*>(_data.get());
        }

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        const std::type_info& type() const noexcept
        {
            return _data != nullptr ? _data->get_typeinfo() : typeid(void);
        }
        #endif

        void swap(any& o) noexcept
        {
            hsd::swap(_data, o._data);
        }

        friend void swap(any& lhs, any& rhs) noexcept
        {
            lhs.swap(rhs);
        }

        template< typename T, typename... Args >
        void emplace(Args&&... args)
        {
            _data = hsd::make_unique<_any_derived<T>>(T(forward<Args>(args)...));
        }

        bool has_value() const
        {
            return _data != nullptr;
        }

        constexpr void reset()
        {
            _data = nullptr;
        }
    };
}
