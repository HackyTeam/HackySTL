#pragma once

#include "Result.hpp"
#include "Utility.hpp"
#include "Types.hpp"
#include <new>

namespace hsd
{
    template <typename T> class list;

    namespace list_detail
    {
        struct bad_iterator
        {
            const char* operator()() const
            {
                return "Null pointer access denied";
            }
        };

        template <typename T>
        class iterator
        {
        private:
            struct list_impl
            {
                T _value;
                list_impl* _next = nullptr;
                list_impl* _back = nullptr;

                inline list_impl() = default;

                inline list_impl(const T& val)
                    : _value{val}
                {}

                inline list_impl(T&& val)
                {
                    _value = hsd::move(val);
                }
            }*_iterator = nullptr;

            friend class list<T>;

            inline T* get()
            {
                return &_iterator->_value;
            }

            inline T* get() const
            {
                return &_iterator->_value;
            }

            inline void push_back(const T& value)
            {
                _iterator->_next = new list_impl{value};
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            inline void push_back(T&& value)
            {
                _iterator->_next = new list_impl{move(value)};
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            template <typename... Args>
            inline void emplace_back(Args&&... args)
            {
                _iterator->_next = new list_impl{};
                _iterator->_next->_value.~T();
                new (&_iterator->_next->_value) T{forward<Args>(args)...};
                
                _iterator->_next->_back = _iterator;
                _iterator = _iterator->_next;
            }

            inline void pop_back()
            {
                list_impl* _element = _iterator;
                _iterator = _iterator->_back;
                
                if (_iterator != nullptr)
                    _iterator->_next = nullptr;
                
                delete _element;
            }

            inline void push_front(const T& value)
            {
                _iterator->_back = new list_impl{value};
                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            inline void push_front(T&& value)
            {
                _iterator->_back = new list_impl{move(value)};
                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            template <typename... Args>
            inline void emplace_front(Args&&... args)
            {
                _iterator->_back = new list_impl{};
                _iterator->_back->_value.~T();
                new (&_iterator->_back->_value) T{hsd::forward<Args>(args)...};

                _iterator->_back->_next = _iterator;
                _iterator = _iterator->_back;
            }

            inline void pop_front()
            {
                list_impl* _element = _iterator;
                _iterator = _iterator->_next;
                
                if (_iterator != nullptr)
                    _iterator->_back = nullptr;
                
                delete _element;
            }

        public:
            inline iterator() {}
            inline iterator(hsd::NullType) {}

            inline iterator(const iterator& other)
            {
                _iterator = other._iterator;
            }
        
            inline iterator& operator=(const iterator& rhs)
            {
                _iterator = rhs._iterator;
                return *this;
            }

            inline friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator == rhs._iterator;
            }

            inline friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator != rhs._iterator;
            }

            inline auto& operator++()
            {
                _iterator = _iterator->_next;
                return *this;
            }

            inline auto& operator--()
            {
                _iterator = _iterator->_back;
                return *this;
            }

            inline iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            inline iterator operator--(i32)
            {
                iterator tmp = *this;
                operator--();
                return tmp;
            }

            inline auto& operator*()
            {
                return _iterator->_value;
            }

            inline const auto& operator*() const
            {
                return _iterator->_value;
            }

            inline auto* operator->()
            {
                return get();
            }

            inline const auto* operator->() const
            {
                return get();
            }
        };
    } // namespace list_detail

    template <typename T>
    class list
    {
    private:
        list_detail::iterator<T> _head;
        list_detail::iterator<T> _tail;
        usize _size = 0;

    public:
        using iterator = list_detail::iterator<T>;
        using const_iterator = const iterator;
        inline list() {}

        inline list(const list& other)
        {
            for(const auto& _element : other)
                push_back(_element);
        }

        inline list(list&& other)
        {
            _head = other._head;
            _tail = other._tail;
            other._head = nullptr;
            other._tail = nullptr;
        }

        template <usize N>
        inline list(const T (&arr)[N])
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(arr[_index]);
        }

        template <usize N>
        inline list(T (&&arr)[N])
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(move(arr[_index]));
        }

        inline ~list()
        {
            clear();
        }

        inline list& operator=(const list& rhs)
        {
            clear();
            
            for (const auto& _element : rhs)
                push_back(_element);
            
            return *this;
        }

        inline list& operator=(list&& rhs)
        {
            _head = rhs._head;
            _tail = rhs._tail;
            rhs._head = nullptr;
            rhs._tail = nullptr;
            
            return *this;
        }

        template <usize N>
        inline list& operator=(const T (&arr)[N])
        {
            clear();
            usize _index = 0;

            for (auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = arr[_index];

            for (; _index < N; _index++)
                push_back(arr[_index]);

            return *this;
        }

        template <usize N>
        inline list& operator=(T (&&arr)[N])
        {
            clear();
            usize _index = 0;

            for (auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = move(arr[_index]);

            for (; _index < N; _index++)
                push_back(move(arr[_index]));

            return *this;
        }

        inline auto erase(const_iterator pos)
            -> Result<iterator, runtime_error>
        {
            if (pos._iterator == nullptr)
            {
                // this in the only situation when
                // .erase() will "throw" because
                // there is no fast way to check
                // if it belongs to this list or not
                return runtime_error{"Accessed an null element"};
            }
            else if (pos == _tail)
            {
                pop_back();
                return end();
            }
            else if (pos == _head)
            {
                pop_front();
                return begin();
            }
            else
            {
                iterator _next_iter, _back_iter;
                _next_iter._iterator = pos._iterator->_next;
                _back_iter._iterator = pos._iterator->_back;
                _next_iter._iterator->_back = _back_iter._iterator;
                _back_iter._iterator->_next = _next_iter._iterator;

                _size--;
                delete pos._iterator;
                return _next_iter;
            }

            return runtime_error{"Undefined Behaviour"};
        }

        inline void push_back(const T& value)
        {
            emplace_back(value);
        }

        inline void push_back(T&& value)
        {
            emplace_back(move(value));
        }

        template <typename... Args>
        inline void emplace_back(Args&&... args)
        {
            if (empty())
            {
                _head._iterator = new typename iterator::list_impl{};
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T{forward<Args>(args)...};
                _tail = _head;
            }
            else
                _tail.emplace_back(forward<Args>(args)...);

            _size++;
        }

        inline void pop_back()
        {
            if (!empty())
            {
                _tail.pop_back();
            }
            else
                _head = _tail;

            _size--;
        }

        inline void push_front(const T& value)
        {
            emplace_front(value);
        }

        inline void push_front(T&& value)
        {
            emplace_front(move(value));
        }

        template <typename... Args>
        inline void emplace_front(Args&&... args)
        {
            if (empty())
            {
                _head._iterator = new typename iterator::list_impl{};
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T{forward<Args>(args)...};
                _tail = _head;
            }
            else
                _head.emplace_front(forward<Args>(args)...);

            _size++;
        }

        inline void pop_front()
        {
            if (!empty())
            {
                _head.pop_front();
            }
            else
                _tail = _head;

            _size--;
        }

        inline void clear()
        {
            for (; !empty(); pop_front())
                ;
                
            pop_front();
        }

        inline usize size() const
        {
            return _size;
        }

        inline bool empty()
        {
            return size() == 0;
        }

        inline T& front()
        {
            return *_head.get();
        }

        inline T& back()
        {
            return *_tail.get();
        }

        inline iterator begin()
        {
            return _head;
        }

        inline iterator begin() const
        {
            return _head;
        }

        inline iterator end()
        {
            return {nullptr};
        }

        inline iterator end() const
        {
            return {nullptr};
        }

        inline const_iterator cbegin() const
        {
            return begin();
        }

        inline const_iterator cend() const
        {
            return end();
        }

        inline iterator rbegin()
        {
            return _tail;
        }

        inline iterator rbegin() const
        {
            return _tail;
        }

        inline iterator rend()
        {
            return {nullptr};
        }

        inline iterator rend() const
        {
            return {nullptr};
        }

        inline const_iterator crbegin() const
        {
            return rbegin();
        }

        inline const_iterator crend() const
        {
            return rend();
        }
    };
} // namespace hsd