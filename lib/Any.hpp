#pragma once

#include "UniquePtr.hpp"

namespace hsd
{
    struct bad_any_cast
    {
        const char* pretty_error() const
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
    };

    template <CopyConstructible T>
    class _any_derived : public _any_base 
    {
        T _value;
        friend class any;

    public:
        inline _any_derived(T value) 
            : _value(move(value)) 
        {}

        hsd::unique_ptr<_any_base> clone() const override 
        {
            return hsd::make_unique<_any_derived>(_value);
        }
    };

    class any
    {
    private:
        hsd::unique_ptr<_any_base> _data = nullptr;

    public:
        inline any() noexcept = default;

        template <CopyConstructible T>
        inline any(T other)
        {
            _data = hsd::make_unique<_any_derived<T>>(move(other));
        }

        inline any(const any& other)
        {
            _data = other._data->clone();
        }

        inline any(any&& other)
        {
            this->swap(other);
        }

        inline ~any() = default;

        inline any& operator=(any rhs)
        {
            this->swap(rhs);
            return *this;
        }

        template <typename T>
        inline auto cast_to() const
            -> result<reference<T>, bad_any_cast>
        {
            using type = typename remove_pointer<T>::type;

            if (auto* p_base = dynamic_cast<_any_derived<type>*>(_data.get()))
            {
                return reference<T>{p_base->_value};
            }
            else
            {
                return bad_any_cast{};
            }
        }

        template <typename T>
        inline T* cast_if() const
        {
            if (auto* p_base = dynamic_cast<_any_derived<T>*>(_data.get()))
                return &p_base->_value;

            return nullptr;
        }

        template <typename T>
        inline bool holds_type() const
        {
            return dynamic_cast<_any_derived<T>*>(_data.get());
        }

        inline void swap(any& other) noexcept
        {
            hsd::swap(_data, other._data);
        }

        friend void swap(any& lhs, any& rhs) noexcept
        {
            lhs.swap(rhs);
        }

        template < typename T, typename... Args >
        inline void emplace(Args&&... args)
        {
            _data = hsd::make_unique<_any_derived<T>>(T(forward<Args>(args)...));
        }

        inline bool has_value() const
        {
            return _data != nullptr;
        }

        inline void reset()
        {
            _data = nullptr;
        }
    };
}
