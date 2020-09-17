#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "Types.hpp"

namespace hsd
{
    template<typename T> class forward_list;

    namespace forward_list_detail
    {
        template<typename T>
        class iterator
        {
        private:
            struct forward_list_impl
            {
                T _value;
                forward_list_impl* _next = nullptr;

                HSD_CONSTEXPR forward_list_impl() {}

                HSD_CONSTEXPR forward_list_impl(const T& val)
                    : _value{val}
                {}

                HSD_CONSTEXPR forward_list_impl(T&& val)
                {
                    _value = hsd::move(val);
                }
            };
            
            forward_list_impl *_iterator = nullptr;
            friend class forward_list<T>;

            HSD_CONSTEXPR iterator() {}
            HSD_CONSTEXPR iterator(hsd::null) {}

            HSD_CONSTEXPR iterator(const iterator& other)
            {
                _iterator = other._iterator;
            }

            HSD_CONSTEXPR iterator& operator=(const iterator& rhs)
            {
                _iterator = rhs._iterator;
                return *this;
            }

            HSD_CONSTEXPR bool operator==(const iterator& rhs)
            {
                return _iterator == rhs._iterator;
            }

            HSD_CONSTEXPR T* get()
            {
                return &_iterator->_value;
            }

            HSD_CONSTEXPR T* get() const
            {
                return &_iterator->_value;
            }

            HSD_CONSTEXPR void push_back(const T& value)
            {
                _iterator->_next = new forward_list_impl(value);
                _iterator = _iterator->_next;
            }

            HSD_CONSTEXPR void push_back(T&& value)
            {
                _iterator->_next = new forward_list_impl(hsd::move(value));
                _iterator = _iterator->_next;
            }

            template<typename... Args>
            HSD_CONSTEXPR void emplace_back(Args&&... args)
            {
                _iterator->_next = new forward_list_impl();
                _iterator->_next->_value.~T();
                new (&_iterator->_next->_value) T(hsd::forward<Args>(args)...);
                _iterator = _iterator->_next;
            }

            HSD_CONSTEXPR void push_front(const T& value)
            {
                forward_list_impl* _element = new forward_list_impl(value);
                _element->_next = _iterator;
                _iterator = _element;
            }

            HSD_CONSTEXPR void push_front(T&& value)
            {
                forward_list_impl* _element = new forward_list_impl(hsd::move(value));
                _element->_next = _iterator;
                _iterator = _element;
            }

            template<typename... Args>
            HSD_CONSTEXPR void emplace_front(Args&&... args)
            {
                forward_list_impl* _element = new forward_list_impl();
                _element->_next = _iterator;
                _element->_value.~T();
                new (&_element->_value) T(hsd::forward<Args>(args)...);
                _iterator = _element;
            }

            HSD_CONSTEXPR void pop_front()
            {
                forward_list_impl* _element = _iterator;
                _iterator = _iterator->_next;
                delete _element;
            }
        public:
        
            HSD_CONSTEXPR bool operator!=(const iterator& rhs)
            {
                return _iterator != rhs._iterator;
            }

            HSD_CONSTEXPR iterator& operator++()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _iterator = _iterator->_next;
                return *this;
            }

            HSD_CONSTEXPR iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            HSD_CONSTEXPR T& operator*()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
            }

            HSD_CONSTEXPR T& operator*() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
            }

            HSD_CONSTEXPR T* operator->()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }

            HSD_CONSTEXPR T* operator->() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }
        };
    } // namespace forward_list_detail

    template<typename T>
    class forward_list
    {
    private:
        forward_list_detail::iterator<T> _head;
        forward_list_detail::iterator<T> _tail;
        
    public:
        using iterator = forward_list_detail::iterator<T>;
        using const_iterator = const iterator;
        HSD_CONSTEXPR forward_list() {}

        HSD_CONSTEXPR forward_list(const forward_list& other)
        {
            for(auto _it = other.cbegin(); _it != other.cend(); _it++)
                push_back(*_it);
        }

        HSD_CONSTEXPR ~forward_list()
        {
            for(; _head != end(); pop_front());
            pop_front();
        }

        HSD_CONSTEXPR forward_list& operator=(const forward_list& rhs)
        {
            this->~forward_list();
            
            for(auto _it = rhs.cbegin(); _it != rhs.cend(); _it++)
                push_back(*_it);
            
            return *this;
        }

        HSD_CONSTEXPR forward_list& operator+(const forward_list& rhs)
        {
            for(auto _element : rhs)
                push_back(_element);

            return *this;
        }

        HSD_CONSTEXPR void push_back(const T& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl(value);
                _tail = _head;
            }
            else
                _tail.push_back(value);
        }

        HSD_CONSTEXPR void push_back(T&& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl(hsd::move(value));
                _tail = _head;
            }
            else
                _tail.push_back(hsd::move(value));
        }

        template<typename... Args>
        HSD_CONSTEXPR void emplace_back(Args&&... args)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl();
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T(hsd::forward<Args>(args)...);
                _tail = _head;
            }
            else
                _tail.emplace_back(hsd::forward<Args>(args)...);
        }

        HSD_CONSTEXPR void push_front(const T& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl(value);
                _tail = _head;
            }
            else
                _head.push_front(value);
        }

        HSD_CONSTEXPR void push_front(T&& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl(hsd::move(value));
                _tail = _head;
            }
            else
                _head.push_front(hsd::move(value));
        }

        template<typename... Args>
        HSD_CONSTEXPR void emplace_front(Args&&... args)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::forward_list_impl();
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T(hsd::forward<Args>(args)...);
                _tail = _head;
            }
            else
                _head.emplace_front(hsd::forward<Args>(args)...);
        }

        HSD_CONSTEXPR void pop_front()
        {
            if(_head != end())
                _head.pop_front();
            else
                _tail = _head;
        }

        HSD_CONSTEXPR bool empty()
        {
            return _head == end();
        }

        HSD_CONSTEXPR T& front()
        {
            return *_head.get();
        }

        HSD_CONSTEXPR T& back()
        {
            return *_tail.get();
        }

        HSD_CONSTEXPR iterator begin()
        {
            return _head;
        }

        HSD_CONSTEXPR iterator begin() const
        {
            return _head;
        }

        HSD_CONSTEXPR iterator end()
        {
            return iterator(nullptr);
        }

        HSD_CONSTEXPR iterator end() const
        {
            return iterator(nullptr);
        }

        HSD_CONSTEXPR const_iterator cbegin()
        {
            return begin();
        }

        HSD_CONSTEXPR const_iterator cbegin() const
        {
            return begin();
        }

        HSD_CONSTEXPR const_iterator cend()
        {
            return end();
        }

        HSD_CONSTEXPR const_iterator cend() const
        {
            return end();
        }
    };
} // namespace hsd