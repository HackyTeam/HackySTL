#pragma once

#include <typeinfo>
#include <type_traits>
#include <exception>
#include "../Utility/Utility.hpp"

#include <any>

namespace hsd
{
    template<typename T>
    concept Copyable = std::is_copy_constructible_v<T>;

    using size_t = unsigned long int;

    struct bad_any_cast : public std::exception 
    {
        virtual const char* what() const noexcept override 
        {
            return "Illegal casting:\n"
            "Cannot cast into different type";
        }
    };

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

    class any
    {
    private:
        void* _data = nullptr;
        bool _is_allocated = false;
        void(*_destroy)(void*) = nullptr;
        type_info _id = typeid(void);

        template<typename T>
        static constexpr void destroy(void* data)
        {
            T* _val_ptr = reinterpret_cast<T*>(data);
            _val_ptr->~T();
            delete _val_ptr;
        }
    public:
        any() = default;

        template<Copyable T>
        constexpr any(const T& other)
        {
            _data = &other;
            _id = typeid(T);
        }

        template<Copyable T>
        constexpr any(T&& other)
        {
            _data = new T(other);
            _destroy = destroy<T>;
            _is_allocated = true;
            _id = typeid(T);
        }

        any(const any& other)
        {
            _data = other._data;
            _id = other._id;
        }

        any(any&& other)
        {
            swap(*this, other);
        }

        ~any()
        {
            reset();
        }

        any& operator=(const any& rhs)
        {
            reset();
            _data = rhs._data;
            _id = rhs._id;
            return *this;
        }

        any& operator=(any&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        template<class T>
        constexpr auto cast()
        {
            typename std::remove_pointer<T>::type type;

            if(_id != typeid(type))
            {
                throw bad_any_cast();
            }
            if constexpr(std::is_pointer_v<T>)
            {
                return reinterpret_cast<T>(_data);
            }
            else
            {
                return *reinterpret_cast<T*>(_data);
            }
        }

        const type_info& type()
        {
            return _id;
        }

        static constexpr void swap(any& lhs, any& rhs)
        {
            hsd::swap(lhs._data, rhs._data);
            hsd::swap(lhs._id, rhs._id);
            hsd::swap(lhs._destroy, rhs._destroy);
            hsd::swap(lhs._is_allocated, rhs._is_allocated);
        }

        template< typename T, typename... Args >
        void emplace(Args&&... args)
        {
            reset();
            _destroy = destroy<T>;
            _data = new T(forward<Args>(args)...);
            _id = typeid(T);
        }

        constexpr bool has_value()
        {
            return _data != nullptr;
        }

        void reset()
        {
            if(_is_allocated == true && _destroy != nullptr)
            {
                _destroy(_data);
            }

            _destroy = nullptr;
            _data = nullptr;
            _id = typeid(void);
        }
    };
}
