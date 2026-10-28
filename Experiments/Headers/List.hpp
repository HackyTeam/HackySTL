#pragma once

#include "../lib/Base/Allocator.hpp"

namespace hsd_test
{
    using namespace hsd;

    template < typename T, typename Allocator > class list;

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
        struct list_impl
        {
            T _value;
            list_impl* _next = nullptr;
            list_impl* _back = nullptr;

            template <typename... Args>
            inline list_impl(Args&&... args)
                : _value{hsd::forward<Args>(args)...}
            {}
        };
    
        template < typename T, typename Allocator >
        class iterator
        {
        private:
            using impl_type = list_impl<T>;
            impl_type* _iterator = nullptr;

            friend class list<T, Allocator>;
        
        public:
            inline iterator() {}
            inline iterator(hsd::null_type) {}

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
                return addressof(_iterator->_value);
            }

            inline const auto* operator->() const
            {
                return addressof(_iterator->_value);
            }
        };
    }

    template < typename T, typename Allocator = allocator >
    class list
    {
    public:
        using iterator = list_detail::iterator<T, Allocator>;
        using const_iterator = const iterator;
        using alloc_type = Allocator;

    private:
        iterator _head;
        iterator _tail;
        alloc_type _alloc;
        usize _size = 0;

    public:
        inline list()
        requires (DefaultConstructible<alloc_type>) = default;

        inline list(const list& other)
            : _alloc{hsd::move(other._alloc)}
        {
            for(const auto& _element : other)
                push_back(_element);
        }

        inline list(list&& other)
            : _head{exchange(other._head, nullptr)},
            _tail{exchange(other._tail, nullptr)},
            _alloc{hsd::move(other._alloc)}, _size{other._size}
        {}

        template <usize N>
        inline list(const T (&arr)[N])
        requires (DefaultConstructible<alloc_type>)
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(arr[_index]);
        }

        template <usize N>
        inline list(T (&&arr)[N])
        requires (DefaultConstructible<alloc_type>)
        {
            for (usize _index = 0; _index < N; _index++)
                push_back(hsd::move(arr[_index]));
        }

        inline list& operator=(const list& rhs)
        {
            clear();
            _alloc = rhs._alloc;

            for (const auto& _element : rhs)
                push_back(_element);
            
            return *this;
        }

        inline list& operator=(list&& rhs)
        {
            swap(_alloc, rhs._alloc);
            swap(_head, rhs._head);
            swap(_tail, rhs._tail);
            
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
                *_it = hsd::move(arr[_index]);

            for (; _index < N; _index++)
                push_back(hsd::move(arr[_index]));

            return *this;
        }

        inline auto erase(const_iterator pos)
            -> result<iterator, runtime_error>
        {
            if (pos._iterator == nullptr)
            {
                // this in the only situation when
                // .erase() will "throw" because
                // there is no fast way to check
                // if it belongs to this list or not
                return runtime_error{"Accessed an null element"};
            }
            
            if (pos == _tail)
            {
                pop_back();
                return end();
            }
            
            if (pos == _head)
            {
                pop_front();
                return begin();
            }

            iterator _next_iter, _back_iter;
            _next_iter._iterator = pos._iterator->_next;
            _back_iter._iterator = pos._iterator->_back;
            _next_iter._iterator->_back = _back_iter._iterator;
            _back_iter._iterator->_next = _next_iter._iterator;

            _size--;
            
            (*pos).~T();

            _alloc
            .deallocate(pos._iterator, 1)
            .unwrap();
            
            return _next_iter;
        }

        inline void push_back(const T& value)
        {
            emplace_back(value);
        }

        inline void push_back(T&& value)
        {
            emplace_back(hsd::move(value));
        }

        template <typename... Args>
        inline void emplace_back(Args&&... args)
        {
            using impl_type = typename iterator::impl_type;

            if (empty())
            {
                _head._iterator = _alloc
                .template allocate<impl_type>(1)
                .unwrap();

                construct_at(
                    _head._iterator, hsd::forward<Args>(args)...
                );
                
                _tail = _head;
            }
            else
            {
                iterator _new_tail;
                
                _new_tail._iterator = _alloc
                .template allocate<impl_type>(1)
                .unwrap();

                construct_at(
                    _new_tail._iterator, hsd::forward<Args>(args)...
                );
                
                _new_tail._iterator->_back = _tail._iterator;
                _tail._iterator->_next = _new_tail._iterator;
                _tail = _new_tail;
            }

            _size++;
        }

        inline void pop_back()
        {
            auto _old_tail = _tail--;
            (*_old_tail).~T();

            _alloc
            .deallocate(_old_tail._iterator, 1)
            .unwrap();

            _size--;
                
            if (_tail != end()) _tail._iterator->_next = nullptr;
            
            if (_size == 0)
            {
                _head._iterator = nullptr;
            }
        }

        inline void push_front(const T& value)
        {
            emplace_front(value);
        }

        inline void push_front(T&& value)
        {
            emplace_front(hsd::move(value));
        }

        template <typename... Args>
        inline void emplace_front(Args&&... args)
        {
            using impl_type = typename iterator::impl_type;

            if (empty())
            {
                _head._iterator = _alloc
                .template allocate<impl_type>(1)
                .unwrap();

                construct_at(
                    _head._iterator, hsd::forward<Args>(args)...
                );
                
                _tail = _head;
            }
            else
            {
                iterator _new_head;
                
                _new_head._iterator = _alloc
                .template allocate<impl_type>(1)
                .unwrap();

                construct_at(
                    _new_head._iterator, hsd::forward<Args>(args)...
                );
                
                _new_head._iterator->_next = _head._iterator;
                _head._iterator->_back = _new_head._iterator;
                _head = _new_head;
            }

            _size++;
        }

        inline void pop_front()
        {
            auto _old_head = _head++;
            (*_old_head).~T();

            _alloc
            .deallocate(_old_head._iterator, 1)
            .unwrap();
                
            _size--;

            if (_head != end()) _head._iterator->_back = nullptr;

            if (_size == 0)
            {
                _tail._iterator = nullptr;
            }
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

        inline const_iterator begin() const
        {
            return cbegin();
        }

        inline iterator end()
        {
            return {nullptr};
        }

        inline const_iterator end() const
        {
            return cend();
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

        inline const_iterator rbegin() const
        {
            return cbegin();
        }

        inline iterator rend()
        {
            return {nullptr};
        }

        inline const_iterator rend() const
        {
            return crend();
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
}