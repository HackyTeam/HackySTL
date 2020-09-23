#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "Types.hpp"

namespace hsd
{
    template<typename T> class list;

    namespace list_detail
    {
        template<typename T>
        class iterator
        {
        private:
            struct list_impl
            {
                T _value;
                list_impl* _next = nullptr;
                list_impl* _back = nullptr;

                HSD_CONSTEXPR list_impl() {}

                constexpr list_impl(const T& val)
                    : _value{val}
                {}

                constexpr list_impl(T&& val)
                {
                    _value = hsd::move(val);
                }
            }*_iterator = nullptr;

            friend class list<T>;
            HSD_CONSTEXPR iterator() {}
            constexpr iterator(hsd::NullType) {}

            constexpr iterator(const iterator& other)
            {
                _iterator = other._iterator;
            }

            constexpr iterator& operator=(const iterator& rhs)
            {
                _iterator = rhs._iterator;
                return *this;
            }

            constexpr bool operator==(const iterator& rhs)
            {
                return _iterator == rhs._iterator;
            }

            constexpr T* get()
            {
                return &_iterator->_value;
            }

            constexpr T* get() const
            {
                return &_iterator->_value;
            }

            HSD_CONSTEXPR void push_back(const T& value)
            {
                _iterator->_next = new list_impl(value);
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            HSD_CONSTEXPR void push_back(T&& value)
            {
                _iterator->_next = new list_impl(hsd::move(value));
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            template<typename... Args>
            HSD_CONSTEXPR void emplace_back(Args&&... args)
            {
                _iterator->_next = new list_impl();
                _iterator->_next->_value.~T();
                new (&_iterator->_next->_value) T(hsd::forward<Args>(args)...);
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            HSD_CONSTEXPR void pop_back()
            {
                list_impl* _element = _iterator;
                _iterator = _iterator->_back;
                
                if(_iterator != nullptr)
                    _iterator->_next = nullptr;
                
                delete _element;
            }

            HSD_CONSTEXPR void push_front(const T& value)
            {
                _iterator->_back = new list_impl(value);
                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            HSD_CONSTEXPR void push_front(T&& value)
            {
                _iterator->_back = new list_impl(hsd::move(value));
                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            template<typename... Args>
            HSD_CONSTEXPR void emplace_front(Args&&... args)
            {
                _iterator->_back = new list_impl();
                _iterator->_back->_value.~T();
                new (&_iterator->_back->_value) T(hsd::forward<Args>(args)...);
                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            HSD_CONSTEXPR void pop_front()
            {
                list_impl* _element = _iterator;
                _iterator = _iterator->_next;
                
                if(_iterator != nullptr)
                    _iterator->_back = nullptr;
                
                delete _element;
            }
        public:
        
            constexpr bool operator!=(const iterator& rhs)
            {
                return _iterator != rhs._iterator;
            }

            constexpr iterator& operator++()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _iterator = _iterator->_next;
                return *this;
            }

            constexpr iterator& operator--()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                _iterator = _iterator->_back;
                return *this;
            }

            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr iterator operator--(i32)
            {
                iterator tmp = *this;
                operator--();
                return tmp;
            }

            constexpr T& operator*()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
            }

            constexpr T& operator*() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return _iterator->_value;
            }

            constexpr T* operator->()
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }

            constexpr T* operator->() const
            {
                if(_iterator == nullptr)
                    throw std::runtime_error("Null pointer access denied");

                return get();
            }
        };
    } // namespace list_detail

    template<typename T>
    class list
    {
    private:
        list_detail::iterator<T> _head;
        list_detail::iterator<T> _tail;

    public:
        using iterator = list_detail::iterator<T>;
        using const_iterator = const iterator;
        HSD_CONSTEXPR list() {}

        HSD_CONSTEXPR list(const list& other)
        {
            for(auto _element : other)
                push_back(_element);
        }

        HSD_CONSTEXPR ~list()
        {
            for(; _head != end(); pop_front());
            pop_front();
        }

        HSD_CONSTEXPR list& operator=(const list& rhs)
        {
            ~list();
            
            for(auto _element : rhs)
                push_back(_element);
            
            return *this;
        }

        HSD_CONSTEXPR list& operator+(const list& rhs)
        {
            for(auto _element : rhs)
                push_back(_element);

            return *this;
        }

        HSD_CONSTEXPR void push_back(const T& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::list_impl(value);
                _tail = _head;
            }
            else
                _tail.push_back(value);
        }

        HSD_CONSTEXPR void push_back(T&& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::list_impl(hsd::move(value));
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
                _head._iterator = new typename iterator::list_impl();
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T(hsd::forward<Args>(args)...);
                _tail = _head;
            }
            else
                _tail.emplace_back(hsd::forward<Args>(args)...);
        }

        HSD_CONSTEXPR void pop_back()
        {
            if(_tail != end())
                _tail.pop_back();
            else
                _head = _tail;
        }

        HSD_CONSTEXPR void push_front(const T& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::list_impl(value);
                _tail = _head;
            }
            else
                _head.push_front(value);
        }

        HSD_CONSTEXPR void push_front(T&& value)
        {
            if(_head == end())
            {
                _head._iterator = new typename iterator::list_impl(hsd::move(value));
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
                _head._iterator = new typename iterator::list_impl();
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

        constexpr bool empty()
        {
            return _head == end();
        }

        constexpr T& front()
        {
            return *_head.get();
        }

        constexpr T& back()
        {
            return *_tail.get();
        }

        constexpr iterator begin()
        {
            return _head;
        }

        constexpr iterator begin() const
        {
            return _head;
        }

        constexpr iterator end()
        {
            return {nullptr};
        }

        constexpr iterator end() const
        {
            return {nullptr};
        }

        constexpr const_iterator cbegin()
        {
            return begin();
        }

        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend()
        {
            return end();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }
    };
} // namespace hsd