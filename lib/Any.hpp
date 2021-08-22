#pragma once

// define HSD_ANY_ENABLE_TYPEINFO to enable type_info

#ifdef HSD_ANY_ENABLE_TYPEINFO
#   include <typeinfo>
#endif

#include "Result.hpp"
#include "Types.hpp"
#include "Utility.hpp"
#include "UniquePtr.hpp"

namespace hsd
{
    namespace any_detail
    {
        template <typename T>
        concept Copyable = std::is_copy_constructible_v<T>;
    } // namespace any_detail
    

    struct bad_any_cast
    {
        const char* operator()() 
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

    template <any_detail::Copyable T>
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
        inline any() noexcept = default;

        template <any_detail::Copyable T>
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
            -> Result<reference<T>, bad_any_cast>
        {
            using type = typename std::remove_pointer<T>::type;

            if (auto* p_base = dynamic_cast<_any_derived<type>*>(_data.get()))
            {
                return p_base->_value;
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

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        inline const std::type_info& type() const noexcept
        {
            return _data != nullptr ? _data->get_typeinfo() : typeid(void);
        }
        #endif

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
