#pragma once

// define HSD_ANY_ENABLE_TYPEINFO to enable type_info

#ifdef HSD_ANY_ENABLE_TYPEINFO
#   include <typeinfo>
#endif

#include <type_traits>
#include <exception>
#include <memory>

#include "Types.hpp"
#include "Utility.hpp"
//#include "UniquePtr.hpp"

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

    #if 0
    class type_info
    {
    private:
        const char* _name;

        constexpr int strcmp(const char* lhs, const char* rhs)
        {
            size_t i = 0;

	        while(lhs[i] && rhs[i])
	        {
	           	if(lhs[i] < rhs[i])
	            {
                    return -1;
                }
	            else if(lhs[i] > rhs[i])
	            {
                    return 1;
                }
	            i++;
            } 

	        if(lhs[i] < rhs[i])
	        {
                return -1;
            }
	        else if(lhs[i] > rhs[i])
	        {
                return 1;
            }
	        else
	        {
                return 0;
            }
        }
    public:
        type_info() = delete;

        type_info(const std::type_info& other)
            : _name{other.name()}
        {}

        constexpr type_info(const type_info& other)
            : _name{other.name()}
        {}

        type_info& operator=(const std::type_info& rhs)
        {
            _name = rhs.name();
            return *this;
        }

        constexpr type_info& operator=(const type_info& rhs)
        {
            _name = rhs.name();
            return *this;
        }

        bool operator==(const std::type_info& rhs)
        {
            return ((_name == rhs.name()) || (_name[0] != '*' 
                        && strcmp(_name, rhs.name()) == 0));
        }

        constexpr bool operator==(const type_info& rhs)
        {
            return ((_name == rhs._name) || (_name[0] != '*' 
                        && strcmp(_name, rhs._name) == 0));
        }

        bool operator!=(const std::type_info& rhs)
        {
            return !operator==(rhs);
        }

        constexpr bool operator!=(const type_info& rhs)
        {
            return !operator==(rhs);
        }

        constexpr const char* name() const
        {
            return _name;
        }
    };
    #endif // disabled code

    class _any_base {
    public:
        virtual ~_any_base() = default;
        virtual std::unique_ptr<_any_base> clone() const = 0;

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        virtual const std::type_info& get_typeinfo() const noexcept = 0;
        #endif
    };

    template<Copyable T>
    class _any_derived : public _any_base {
        T _value;
        friend class any;
    public:
        constexpr _any_derived(T value) : _value(move(value)) {}

        std::unique_ptr<_any_base> clone() const override {
            return std::make_unique<_any_derived>(_value);
        }

        #ifdef HSD_ANY_ENABLE_TYPEINFO
        const std::type_info& get_typeinfo() const noexcept override {
            return typeid(T);
        }
        #endif
    };

    class any
    {
    private:
        std::unique_ptr<_any_base> _data;
    public:
        constexpr any() noexcept = default;

        template<Copyable T>
        constexpr any(T other)
        {
            _data = std::make_unique<_any_derived<T>>(move(other));
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

        template<class T>
        constexpr decltype(auto) cast_to() const
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
            return _data ? _data->get_typeinfo() : typeid(void);
        }
        #endif

        void swap(any& o) noexcept
        {
            std::swap(_data, o._data);
        }

        friend void swap(any& lhs, any& rhs) noexcept
        {
            lhs.swap(rhs);
        }

        template< typename T, typename... Args >
        void emplace(Args&&... args)
        {
            _data = std::make_unique<_any_derived<T>>(T(forward<Args>(args)...));
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
