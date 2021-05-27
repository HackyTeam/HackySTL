#pragma once

#include <cassert>
#include "Utility.hpp"
#include "Tuple.hpp"
#include "Allocator.hpp"

namespace hsd
{
    template < typename T, template <typename> typename Allocator = allocator >
    class vector
    {
    private:
        using alloc_type = Allocator<T>;
        alloc_type _alloc;
        T* _data = nullptr;
        usize _size = 0;
        usize _capacity = 0;

        struct bad_access
        {
            const char* operator()() const
            {
                return "Tried to access an element out of bounds";
            }
        };

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        inline ~vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                at_unchecked(_index - 1).~T();
                
            _alloc.deallocate(_data, _capacity).unwrap();
        }

        inline vector() requires (std::is_default_constructible_v<alloc_type>) = default;

        inline vector(usize size)
        {
            resize(size);
        }

        template <typename Alloc = alloc_type>
        inline vector(const Alloc& alloc)
        requires (std::is_constructible_v<alloc_type, Alloc>)
            : _alloc(alloc)
        {}

        template <typename Alloc = alloc_type>
        inline vector(usize size, const Alloc& alloc)
        requires (std::is_constructible_v<alloc_type, Alloc>)
            : _alloc(alloc)
        {
            resize(size);
        }

        inline vector(const vector& other)
        requires (std::is_copy_constructible_v<alloc_type>)
            : _alloc(other._alloc), _size(other._size), _capacity(other._capacity)
        {
            _data = _alloc.allocate(other._capacity).unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                _alloc.construct_at(&_data[_index], other[_index]);
        }

        inline vector(const vector& other)
        requires (!std::is_copy_constructible_v<alloc_type>)
            : _size(other._size), _capacity(other._capacity)
        {
            _data = _alloc.allocate(other._capacity).unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                _alloc.construct_at(&_data[_index], other[_index]);
        }

        inline vector(vector&& other)
        requires (std::is_move_constructible_v<alloc_type>)
            : _alloc(move(other._alloc))
        {
            swap(_data, other._data);
            swap(_size, other._size);
            swap(_capacity, other._capacity);
        }

        inline vector(vector&& other)
        requires (!std::is_move_constructible_v<alloc_type>)
        {
            swap(_data, other._data);
            swap(_size, other._size);
            swap(_capacity, other._capacity);
        }

        template <usize N>
        inline vector(const T (&arr)[N])
            : _size(N), _capacity(N)
        {
            _data = _alloc.allocate(N).unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                _alloc.construct_at(&_data[_index], arr[_index]);
        }

        template <usize N>
        inline vector(T (&&arr)[N])
            : _size(N), _capacity(N)
        {
            _data = _alloc.allocate(N).unwrap();

            for (usize _index = 0; _index < _size; ++_index)
                _alloc.construct_at(&_data[_index], move(arr[_index]));
        }

        inline vector& operator=(const vector& rhs)
        {
            if (_capacity < rhs._size)
            {
                clear();
                reserve(rhs._size);
                
                for (usize _index = 0; _index < rhs._size; ++_index)
                    _alloc.construct_at(&_data[_index], rhs[_index]);
                
                _size = rhs._size;
            }
            else
            {
                usize _index;
                usize _min_size = _size < rhs._size ? _size : rhs._size;
                
                for (_index = 0; _index < _min_size; ++_index)
                    _data[_index] = rhs[_index];
                
                if (_size > rhs._size)
                {
                    for (_index = _size; _index > rhs._size; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (rhs._size > _size)
                {
                    for (; _index < rhs._size; ++_index)
                        _alloc.construct_at(&_data[_index], rhs[_index]);
                }

                _size = rhs._size;
            }

            return *this;
        }

        inline vector& operator=(vector&& rhs)
        {
            clear();
            
            _alloc.deallocate(_data, _capacity).unwrap();
            _data = exchange(rhs._data, nullptr);            
            _size = exchange(rhs._size, 0u);
            _capacity = exchange(rhs._capacity, 0u);
    
            return *this;
        }

        template <usize N>
        inline vector& operator=(const T (&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                    _alloc.construct_at(&_data[_index], arr[_index]);
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    _data[_index] = arr[_index];
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                        _alloc.construct_at(&_data[_index], arr[_index]);
                }
                
                _size = N;
            }

            return *this;
        }

        template <usize N>
        inline vector& operator=(T (&&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                    _alloc.construct_at(&_data[_index], move(arr[_index]));
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    _data[_index] = move(arr[_index]);
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                        _alloc.construct_at(&_data[_index], move(arr[_index]));
                }

                _size = N;
            }

            return *this;
        }

        inline auto& operator[](usize index)
        {
            return at_unchecked(index);
        }

        inline auto& operator[](usize index) const
        {
            return at_unchecked(index);
        }

        inline auto& front()
        {
            return *begin();
        }

        inline auto& front() const
        {
            return *begin();
        }

        inline auto& back() noexcept
        {
            return *(begin() + size() - 1);
        }

        inline auto& back() const
        {
            return *(begin() + size() - 1);
        }

        inline auto erase(const_iterator pos)
            -> Result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        inline auto erase_for(const_iterator from, const_iterator to)
            -> Result<iterator, bad_access>
        {
            if(from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            if(to == end())
            {
                for(; from != end(); from++)
                    from->~T();
    
                return end();
            }
            else
            {
                usize _current_pos = static_cast<usize>(from - begin());
                usize _last_pos = static_cast<usize>(to - begin());

                for(usize _index = 0; _index < _capacity - _last_pos; _index++)
                {
                    _data[_current_pos + _index] = 
                        move(_data[_last_pos + _index]);
                }

                _size -= static_cast<usize>(to - from) + 1;
                return begin() + _last_pos;
            }
        }

        inline auto at(usize index)
            -> Result<reference<T>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline auto at(usize index) const
            -> Result< const reference<T>, bad_access >
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline auto& at_unchecked(usize index)
        {
            return _data[index];
        }

        inline const auto& at_unchecked(usize index) const
        {
            return _data[index];
        }

        inline void clear() noexcept
        {
            if constexpr(is_same<decltype(_data), T*>::value)
            {
                for (usize _index = _size; _index > 0; --_index)
                    at_unchecked(_index - 1).~T();
            }
                
            _size = 0;
        }

        inline void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;

                while (_new_capacity < new_cap)
                    _new_capacity += (_new_capacity + 1) / 2;

                if constexpr(is_same<decltype(_data), T*>::value)
                {
                    T* _new_buf = _alloc.allocate(_new_capacity).unwrap();
                
                    for (usize _index = 0; _index < _size; ++_index)
                    {
                        auto& _value = at_unchecked(_index);
                        _alloc.construct_at(&_new_buf[_index], move(_value));
                        _value.~T();
                    }

                    _alloc.deallocate(_data, _capacity).unwrap();
                    _data = _new_buf;
                }
                
                _capacity = _new_capacity;
            }
        }

        inline void shrink_to_fit()
        {
            if (_size == 0)
            {
                T* _old_buf = exchange(_data, nullptr);
                deallocate(_old_buf, _capacity).unwrap();
                _capacity = 0;
            }
            else if (_size < _capacity)
            {
                T* _new_buf = _alloc.allocate(_size).unwrap();
                move<T>(_data, _data + _size, _new_buf);
                deallocate(_data, _capacity).unwrap();
                _capacity = _size;
                _data = _new_buf;
            }
        }

        inline void resize(usize new_size)
        {
            if (new_size > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;
                
                while (_new_capacity < new_size)
                    _new_capacity += (_new_capacity + 1) / 2;

                if constexpr(is_same<decltype(_data), T*>::value)
                {
                    T* _new_buf = _alloc.allocate(_new_capacity).unwrap();
                    usize _index = 0;

                    for (; _index < _size; ++_index)
                    {
                        auto& _value = at_unchecked(_index);
                        _alloc.construct_at(&_new_buf[_index], move(_value));
                        _value.~T();
                    }
                    for (; _index < new_size; ++_index)
                    {
                        if constexpr(
                            std::is_constructible_v<T, alloc_type> && 
                            !std::is_default_constructible_v<T>)
                        {
                            _alloc.construct_at(&_new_buf[_index], _alloc);
                        }
                        else
                        {
                            _alloc.construct_at(&_new_buf[_index]);
                        }
                    }

                    _alloc.deallocate(_data, _capacity).unwrap();
                    _data = _new_buf;
                }

                _capacity = _new_capacity;
                _size = new_size;
            }
            else if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                {
                    if constexpr(
                        std::is_copy_constructible_v<alloc_type> &&
                        !std::is_default_constructible_v<T>)
                    {
                        _alloc.construct_at(&_data[_index], _alloc);
                    }
                    else
                    {
                        _alloc.construct_at(&_data[_index]);
                    }
                }
                
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    at_unchecked(_index - 1).~T();
                
                _size = new_size;
            }
        }

        inline void push_back(const T& val)
        {
            emplace_back(val);
        }

        inline void push_back(T&& val)
        {
            emplace_back(move(val));
        }

        template <typename... Args>
        inline void emplace_back(Args&&... args)
        {
            reserve(_size + 1);
            _alloc.construct_at(&_data[_size], forward<Args>(args)...);
            ++_size;
        }

        inline void pop_back() noexcept
        {
            if(_size > 0)
            {
                at_unchecked(_size - 1).~T();
                _size--;
            }
        }

        inline usize size() const
        {
            return _size;
        }

        inline usize capacity() const
        {
            return _capacity;
        }

        inline iterator data()
        {
            if constexpr(is_same<decltype(_data), T*>::value)
            {
                return _data;
            }
            else
            {
                return &_data[0];
            }
        }

        inline iterator begin()
        {
            return data();
        }

        inline iterator end()
        {
            return begin() + size();
        }

        inline const_iterator begin() const
        {
            return cbegin();
        }

        inline const_iterator end() const
        {
            return cend();
        }

        inline const_iterator cbegin() const
        {
            if constexpr(is_same<decltype(_data), T*>::value)
            {
                return _data;
            }
            else
            {
                return &_data[0];
            }
        }

        inline const_iterator cend() const
        {
            return cbegin() + size();
        }

        inline iterator rbegin()
        {
            return end() - 1;
        }

        inline iterator rend()
        {
            return begin() - 1;
        }

        inline const_iterator rbegin() const
        {
            return crbegin();
        }

        inline const_iterator rend() const
        {
            return crend();
        }

        inline const_iterator crbegin() const
        {
            return cend() - 1;
        }

        inline const_iterator crend() const
        {
            return cbegin() - 1;
        }
    };

    template < typename T, usize N >
    class static_vector
    {
    private:
        stack_array<T, N> _data;
        usize _size = 0;
        static constexpr usize _capacity = N;

        struct bad_access
        {
            const char* operator()() const
            {
                return "Tried to access an element out of bounds";
            }
        };

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        constexpr ~static_vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                at_unchecked(_index - 1).~T();
        }

        constexpr static_vector() = default;

        constexpr static_vector(usize size)
        {
            resize(size);
        }

        constexpr static_vector(const static_vector& other)
            : _size(other._size)
        {
            for (usize _index = 0; _index < _size; ++_index)
                _data[_index] = other[_index];
        }

        constexpr static_vector(static_vector&& other)
        {
            swap(_data, other._data);
            swap(_size, other._size);
        }

        constexpr static_vector(const T (&arr)[N])
            : _size(N)
        {
            for (usize _index = 0; _index < _size; ++_index)
                _data[_index] = arr[_index];
        }

        constexpr static_vector(T (&&arr)[N])
            : _size(N)
        {
            for (usize _index = 0; _index < _size; ++_index)
                _data[_index] = move(arr[_index]);
        }

        constexpr static_vector& operator=(const static_vector& rhs)
        {
            usize _index;
            usize _min_size = _size < rhs._size ? _size : rhs._size;
            
            for (_index = 0; _index < _min_size; ++_index)
                _data[_index] = rhs[_index];
            
            if (_size > rhs._size)
            {
                for (_index = _size; _index > rhs._size; --_index)
                    at_unchecked(_index - 1).~T();
            }
            else if (rhs._size > _size)
            {
                for (; _index < rhs._size; ++_index)
                    _data[_index] = rhs[_index];
            }

            _size = rhs._size;

            return *this;
        }

        constexpr static_vector& operator=(static_vector&& rhs)
        {
            clear();
            swap(_data, rhs._data);
            _size = exchange(rhs._size, 0u);
    
            return *this;
        }

        constexpr static_vector& operator=(const T (&arr)[N])
        {
            usize _index;
            usize min_size = _size > N ? _size : N;
            
            for (_index = 0; _index < min_size; ++_index)
            {
                _data[_index] = arr[_index];
            }
            if (_size > N)
            {
                for (_index = _size; _index > N; --_index)
                    at_unchecked(_index - 1).~T();
            }
            else if (N > _size)
            {
                for (; _index < N; ++_index)
                    _data[_index] = arr[_index];
            }
                
            _size = N;
            return *this;
        }

        constexpr static_vector& operator=(T (&&arr)[N])
        {
            usize _index;
            usize min_size = _size > N ? _size : N;
            
            for (_index = 0; _index < min_size; ++_index)
            {
                _data[_index] = move(arr[_index]);
            }
            if (_size > N)
            {
                for (_index = _size; _index > N; --_index)
                    at_unchecked(_index - 1).~T();
            }
            else if (N > _size)
            {
                for (; _index < N; ++_index)
                    _data[_index] = move(arr[_index]);
            }

            _size = N;
            return *this;
        }

        constexpr auto& operator[](usize index)
        {
            return at_unchecked(index);
        }

        constexpr auto& operator[](usize index) const
        {
            return at_unchecked(index);
        }

        constexpr auto& front()
        {
            return *begin();
        }

        constexpr auto& front() const
        {
            return *begin();
        }

        constexpr auto& back() noexcept
        {
            return *(begin() + size() - 1);
        }

        constexpr auto& back() const
        {
            return *(begin() + size() - 1);
        }

        inline auto erase(const_iterator pos)
            -> Result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        inline auto erase_for(const_iterator from, const_iterator to)
            -> Result<iterator, bad_access>
        {
            if(from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            if(to == end())
            {
                for(; from != end(); from++)
                    from->~T();
    
                return end();
            }
            else
            {
                usize _current_pos = static_cast<usize>(from - begin());
                usize _last_pos = static_cast<usize>(to - begin());

                for(usize _index = 0; _index < _capacity - _last_pos; _index++)
                {
                    _data[_current_pos + _index] = 
                        move(_data[_last_pos + _index]);
                }

                _size -= static_cast<usize>(to - from) + 1;
                return begin() + _last_pos;
            }
        }

        constexpr auto at(usize index)
            -> Result<reference<T>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        constexpr auto at(usize index) const
            -> Result< const reference<T>, bad_access >
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        constexpr auto& at_unchecked(usize index)
        {
            return _data[index];
        }

        constexpr const auto& at_unchecked(usize index) const
        {
            return _data[index];
        }

        constexpr void clear() noexcept
        {  
            _size = 0;
        }

        constexpr void resize(usize new_size)
        {
            if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                    _data[_index] = T{};

                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    at_unchecked(_index - 1).~T();
                
                _size = new_size;
            }
        }

        constexpr void push_back(const T& val)
        {
            emplace_back(val);
        }

        constexpr void push_back(T&& val)
        {
            emplace_back(move(val));
        }

        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            _data[_size] = T{forward<Args>(args)...};
            ++_size;
        }

        constexpr void pop_back() noexcept
        {
            if(_size > 0)
            {
                at_unchecked(_size - 1).~T();
                _size--;
            }
        }

        constexpr usize size() const
        {
            return _size;
        }

        constexpr usize capacity() const
        {
            return _capacity;
        }

        constexpr iterator data()
        {
            return &_data[0];
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr const_iterator begin() const
        {
            return cbegin();
        }

        constexpr const_iterator end() const
        {
            return cend();
        }

        constexpr const_iterator cbegin() const
        {
            return &_data[0];
        }

        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }

        constexpr iterator rbegin()
        {
            return end() - 1;
        }

        constexpr iterator rend()
        {
            return begin() - 1;
        }

        constexpr const_iterator rbegin() const
        {
            return crbegin();
        }

        constexpr const_iterator rend() const
        {
            return crend();
        }

        constexpr const_iterator crbegin() const
        {
            return cend() - 1;
        }

        constexpr const_iterator crend() const
        {
            return cbegin() - 1;
        }
    };

    template < typename T, usize N > vector(const T (&)[N]) -> vector<T>;
    template < typename T, usize N > vector(T (&&)[N]) -> vector<T>;
    template < typename T > using buffered_vector = vector< T, buffered_allocator >;

    template< typename L, typename... U >
    requires (std::is_constructible_v<L, U> && ...)
    constexpr vector<L> make_vector(L&& first, U&&... rest)
    {
        constexpr usize size = 1 + sizeof...(U);
        vector<L> vec;
        vec.reserve(size);
        
        [&vec]<usize... _index>(hsd::index_sequence<_index...>, auto&... args)
        {
            (vec.emplace_back(forward<L>(args)), ...);
        }(hsd::make_index_sequence<size>{}, first, rest...);
        
        return vec;
    }
}
