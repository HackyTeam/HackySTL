#pragma once

#include "Utility.hpp"
#include "Types.hpp"
#include "Result.hpp"
#include <new>

namespace hsd
{
    template <typename T> class forward_list;

    namespace forward_list_detail
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
            struct forward_list_impl
            {
                T _value;
                forward_list_impl* _next = nullptr;

                HSD_CONSTEXPR forward_list_impl() {}

                constexpr forward_list_impl(const T& val)
                    : _value{val}
                {}

                constexpr forward_list_impl(T&& val)
                {
                    _value = hsd::move(val);
                }
            };
            
            forward_list_impl *_iterator = nullptr;
            friend class forward_list<T>;

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

            constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator == rhs._iterator;
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
                _iterator->_next = new forward_list_impl(value);
                _iterator = _iterator->_next;
            }

            HSD_CONSTEXPR void push_back(T&& value)
            {
                _iterator->_next = new forward_list_impl(hsd::move(value));
                _iterator = _iterator->_next;
            }

            template <typename... Args>
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

            template <typename... Args>
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
        
            constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
            {
                return lhs._iterator != rhs._iterator;
            }

            constexpr auto operator++() 
                -> Result<reference<iterator>, bad_iterator>
            {
                if(_iterator == nullptr)
                    return bad_iterator{};

                _iterator = _iterator->_next;
                return {*this};
            }

            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            constexpr auto operator*() 
                -> Result<reference<T>, bad_iterator>
            {
                if(_iterator == nullptr)
                    return bad_iterator{};

                return {_iterator->_value};
            }

            constexpr auto operator*() const 
                -> Result<const reference<T>, bad_iterator>
            {
                if(_iterator == nullptr)
                    return bad_iterator{};

                return {_iterator->_value};
            }

            constexpr auto operator->() 
                -> Result<T*, bad_iterator>
            {
                if(_iterator == nullptr)
                    return bad_iterator{};

                return get();
            }

            constexpr auto operator->() const 
                -> Result<const T*, bad_iterator>
            {
                if(_iterator == nullptr)
                    return bad_iterator{};

                return get();
            }
        };
    } // namespace forward_list_detail

    template <typename T>
    class forward_list
    {
    private:
        forward_list_detail::iterator<T> _head;
        forward_list_detail::iterator<T> _tail;
        usize _size = 0;
        
    public:
        using iterator = forward_list_detail::iterator<T>;
        using const_iterator = const iterator;
        HSD_CONSTEXPR forward_list() {}

        HSD_CONSTEXPR forward_list(const forward_list& other)
        {
            for(const auto& _element : other)
                push_back(_element);
        }

        HSD_CONSTEXPR forward_list(forward_list&& other)
        {
            _head = other._head;
            _tail = other._tail;
            other._head = nullptr;
            other._tail = nullptr;
        }

        template <usize N>
        HSD_CONSTEXPR forward_list(const T (&arr)[N])
        {
            for(usize _index = 0; _index < N; _index++)
                push_back(arr[_index]);
        }

        template <usize N>
        HSD_CONSTEXPR forward_list(T (&&arr)[N])
        {
            for(usize _index = 0; _index < N; _index++)
                push_back(move(arr[_index]));
        }

        HSD_CONSTEXPR ~forward_list()
        {
            clear();
        }

        HSD_CONSTEXPR forward_list& operator=(const forward_list& rhs)
        {
            clear();
            
            for(const auto& _element : rhs)
                push_back(_element);
            
            return *this;
        }

        HSD_CONSTEXPR forward_list& operator=(forward_list&& rhs)
        {
            _head = rhs._head;
            _tail = rhs._tail;
            rhs._head = nullptr;
            rhs._tail = nullptr;
            
            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR forward_list& operator=(const T (&arr)[N])
        {
            clear();
            usize _index = 0;

            for(auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = arr[_index];

            for(; _index < N; _index++)
                push_back(arr[_index]);

            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR forward_list& operator=(T (&&arr)[N])
        {
            clear();
            usize _index = 0;

            for(auto _it = begin(); _it != end() && _index < N; _it++, _index++)
                *_it = move(arr[_index]);

            for(; _index < N; _index++)
                push_back(move(arr[_index]));

            return *this;
        }

        HSD_CONSTEXPR forward_list& operator+=(const forward_list& rhs)
        {
            for(const auto& _element : rhs)
                push_back(_element);

            return *this;
        }

        HSD_CONSTEXPR forward_list& operator+=(forward_list&& rhs)
        {
            for(auto&& _element : rhs)
                push_back(move(_element));

            return *this;
        }

        HSD_CONSTEXPR void push_back(const T& value)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl(value);
                _tail = _head;
            }
            else
                _tail.push_back(value);

            _size++;
        }

        HSD_CONSTEXPR void push_back(T&& value)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl(hsd::move(value));
                _tail = _head;
            }
            else
                _tail.push_back(hsd::move(value));

            _size++;
        }

        template <typename... Args>
        HSD_CONSTEXPR void emplace_back(Args&&... args)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl();
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T(hsd::forward<Args>(args)...);
                _tail = _head;
            }
            else
                _tail.emplace_back(hsd::forward<Args>(args)...);

            _size++;
        }

        HSD_CONSTEXPR void push_front(const T& value)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl(value);
                _tail = _head;
            }
            else
                _head.push_front(value);

            _size++;
        }

        HSD_CONSTEXPR void push_front(T&& value)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl(hsd::move(value));
                _tail = _head;
            }
            else
                _head.push_front(hsd::move(value));

            _size++;
        }

        template <typename... Args>
        HSD_CONSTEXPR void emplace_front(Args&&... args)
        {
            if(empty())
            {
                _head._iterator = new typename iterator::forward_list_impl();
                _head._iterator->_value.~T();
                new (&_head._iterator->_value) T(hsd::forward<Args>(args)...);
                _tail = _head;
            }
            else
                _head.emplace_front(hsd::forward<Args>(args)...);

            _size++;
        }

        HSD_CONSTEXPR void pop_front()
        {
            if(!empty())
            {
                _head.pop_front();
            }
            else
                _tail = _head;
                
            _size--;
        }

        HSD_CONSTEXPR void clear()
        {
            for(; !empty(); pop_front());
            pop_front();
        }

        constexpr bool empty()
        {
            return _size == 0;
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
            return iterator(nullptr);
        }

        constexpr iterator end() const
        {
            return iterator(nullptr);
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