#pragma once

#include <typeinfo>
#include <type_traits>
#include <exception>
#include "../Utility/Utility.hpp"

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
            int i = 0;

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

        constexpr type_info& operator=(const std::type_info& rhs)
        {
            _name = rhs.name();
            return *this;
        }

        constexpr type_info& operator=(const type_info& rhs)
        {
            _name = rhs.name();
            return *this;
        }

        constexpr bool operator==(const std::type_info& rhs)
        {
            return ((_name == rhs.name()) || (_name[0] != '*' 
                        && strcmp(_name, rhs.name()) == 0));
        }

        constexpr bool operator==(const type_info& rhs)
        {
            return ((_name == rhs._name) || (_name[0] != '*' 
                        && strcmp(_name, rhs._name) == 0));
        }

        constexpr bool operator!=(const std::type_info& rhs)
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
        type_info _id;
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
            _data = &other;
            _id = typeid(T);
        }

        constexpr any(const any& other)
            : _data{other._data}, _id{other._id}
        {}

        constexpr any(any&& other)
            : _data{other._data}, _id{other._id}
        {
            other.reset();
        }

        constexpr ~any()
        {
            reset();
        }

        constexpr any& operator=(const any& rhs)
        {
            _data = rhs._data;
            _id = rhs._id;
            return *this;
        }

        constexpr any& operator=(any&& rhs)
        {
            _data = rhs._data;
            _id = rhs._id;
            rhs.reset();
            return *this;
        }

        template<class T>
        constexpr auto cast()
        {
            if(_id != typeid(T))
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
            }*/
        }

        type_info type()
        {
            return _id;
        }

        constexpr void swap(any& other)
        {
            hsd::swap(_data, other._data);
            hsd::swap(_id, other._id);
            hsd::swap(_is_allocated, other._is_allocated);
        }

        template< typename T, typename... Args >
        constexpr void emplace(Args&&... args)
        {
            if(_is_allocated == true)
            {
                delete _data;
            }

            _is_allocated = true;
            _data = nullptr;
            _data = new T(forward<Args>(args)...);
            _id = typeid(T);
        }

        constexpr bool has_value()
        {
            return _data != nullptr;
        }

        constexpr void reset()
        {
            if(_is_allocated == true)
            {
                delete _data;
            }

            _data = nullptr;
            _id = typeid(void);
        }
    };
}