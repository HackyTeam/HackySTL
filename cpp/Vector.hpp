#pragma once

#include <stdexcept>
#include <cassert>

#include "Utility.hpp"
#include "Tuple.hpp"
#include "Allocator.hpp"

namespace hsd
{
    template < typename T, typename Allocator = allocator<T> >
    class vector : private Allocator
    {
    private:
        usize _size = 0;
        usize _capacity = 0;

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        HSD_CONSTEXPR ~vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                at_unchecked(_index - 1).~T();
                
            this->deallocate(this->_data, _capacity);
        }

        HSD_CONSTEXPR vector(usize size)
        {
            resize(size);
        }

        HSD_CONSTEXPR vector() noexcept = default;

        HSD_CONSTEXPR vector(const Allocator& alloc)
        requires (std::is_copy_constructible_v<Allocator>)
            : Allocator(alloc)
        {}

        // used only for initializing once
        HSD_CONSTEXPR vector(Allocator&& alloc)
            : Allocator(move(alloc))
        {}

        HSD_CONSTEXPR vector(const vector& rhs)
            : Allocator(rhs), _size(rhs._size), _capacity(rhs._capacity)
        {
            if constexpr(is_same<decltype(this->_data), T*>::value)
                this->_data = this->allocate(rhs._capacity);

            for (usize _index = 0; _index < _size; ++_index)
                std::construct_at(&this->_data[_index], rhs[_index]);
        }

        constexpr vector(vector&& rhs) noexcept
        {
            swap(this->_data, rhs._data);
            swap(_size, rhs._size);
            swap(_capacity, rhs._capacity);
        }

        template <usize N>
        HSD_CONSTEXPR vector(const T (&arr)[N])
            : _size(N), _capacity(N)
        {
            if constexpr(is_same<decltype(this->_data), T*>::value)
                this->_data = this->allocate(N);

            for (usize _index = 0; _index < _size; ++_index)
                std::construct_at(&this->_data[_index], arr[_index]);
        }

        template <usize N>
        HSD_CONSTEXPR vector(T (&&arr)[N])
            : _size(N), _capacity(N)
        {
            if constexpr(is_same<decltype(this->_data), T*>::value)
                this->_data = this->allocate(N);

            for (usize _index = 0; _index < _size; ++_index)
                std::construct_at(&this->_data[_index], move(arr[_index]));
        }

        HSD_CONSTEXPR vector& operator=(const vector& rhs)
        {
            if (_capacity < rhs._size)
            {
                clear();
                reserve(rhs._size);
                
                for (usize _index = 0; _index < rhs._size; ++_index)
                    std::construct_at(&this->_data[_index], rhs[_index]);
                
                _size = rhs._size;
            }
            else
            {
                usize _index;
                usize _min_size = _size < rhs._size ? _size : rhs._size;
                
                for (_index = 0; _index < _min_size; ++_index)
                    this->_data[_index] = rhs[_index];
                
                if (_size > rhs._size)
                {
                    for (_index = _size; _index > rhs._size; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (rhs._size > _size)
                {
                    for (; _index < rhs._size; ++_index)
                        std::construct_at(&this->_data[_index], rhs[_index]);
                }

                _size = rhs._size;
            }

            return *this;
        }

        HSD_CONSTEXPR vector& operator=(vector&& rhs) noexcept
        {
            clear();
            this->deallocate(this->_data, _capacity);
            
            if constexpr(is_same<decltype(this->_data), T*>::value)
            {
                this->_data = exchange(rhs._data, nullptr);
            }
            else
            {
                swap(this->_data, rhs._data);
            }
            
            _size = exchange(rhs._size, 0u);
            _capacity = exchange(rhs._capacity, 0u);
    
            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR vector& operator=(const T (&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                    std::construct_at(&this->_data[_index], arr[_index]);
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    this->_data[_index] = arr[_index];
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                        std::construct_at(&this->_data[_index], arr[_index]);
                }
                
                _size = N;
            }

            return *this;
        }

        template <usize N>
        HSD_CONSTEXPR vector& operator=(T (&&arr)[N])
        {
            if (_capacity < N)
            {
                clear();
                reserve(N);
                
                for (usize _index = 0; _index < N; ++_index)
                    std::construct_at(&this->_data[_index], move(arr[_index]));
                
                _size = N;
            }
            else
            {
                usize _index;
                usize min_size = _size > N ? _size : N;
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    this->_data[_index] = move(arr[_index]);
                }
                if (_size > N)
                {
                    for (_index = _size; _index > N; --_index)
                        at_unchecked(_index - 1).~T();
                }
                else if (N > _size)
                {
                    for (; _index < N; ++_index)
                        std::construct_at(&this->_data[_index], move(arr[_index]));
                }

                _size = N;
            }

            return *this;
        }

        constexpr auto& operator[](usize index) noexcept
        {
            return at(index);
        }

        constexpr auto& operator[](usize index) const noexcept
        {
            return at(index);
        }

        constexpr auto& front() noexcept
        {
            return *begin();
        }

        constexpr auto& front() const noexcept
        {
            return *begin();
        }

        constexpr auto& back() noexcept
        {
            return *(begin() + size() - 1);
        }

        constexpr auto& back() const noexcept
        {
            return *(begin() + size() - 1);
        }

        constexpr auto& at(usize index)
        {
            if(index >= _size)
                throw std::out_of_range("Accessed element out of range");

            return this->_data[index];
        }

        constexpr const auto& at(usize index) const
        {
            if(index >= _size)
                throw std::out_of_range("Accessed element out of range");

            return this->_data[index];
        }

        constexpr auto& at_unchecked(usize index) noexcept
        {
            return this->_data[index];
        }

        constexpr const auto& at_unchecked(usize index) const noexcept
        {
            return this->_data[index];
        }

        constexpr void clear() noexcept
        {
            for (usize _index = _size; _index > 0; --_index)
                at_unchecked(_index - 1).~T();
                
            _size = 0;
        }

        HSD_CONSTEXPR void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;

                while (_new_capacity < new_cap)
                    _new_capacity += (_new_capacity + 1) / 2;

                if constexpr(is_same<decltype(this->_data), T*>::value)
                {
                    T* _new_buf = this->allocate(_new_capacity);
                
                    for (usize _index = 0; _index < _size; ++_index)
                    {
                        auto& _value = at_unchecked(_index);
                        std::construct_at(&_new_buf[_index], move(_value));
                        _value.~T();
                    }

                    this->deallocate(this->_data, _capacity);
                    this->_data = _new_buf;
                }
                
                _capacity = _new_capacity;
            }
        }

        HSD_CONSTEXPR void shrink_to_fit()
        {
            if (_size == 0)
            {
                T* _old_buf = exchange(this->_data, nullptr);
                this->deallocate(_old_buf, _capacity);
                _capacity = 0;
            }
            else if (_size < _capacity)
            {
                T* _new_buf = this->allocate(_size);
                move<T>(this->_data, this->_data + _size, _new_buf);
                this->deallocate(this->_data, _capacity);
                _capacity = _size;
                this->_data = _new_buf;
            }
        }

        HSD_CONSTEXPR void resize(usize new_size)
        {
            if (new_size > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;
                
                while (_new_capacity < new_size)
                    _new_capacity += (_new_capacity + 1) / 2;

                if constexpr(is_same<decltype(this->_data), T*>::value)
                {
                    T* _new_buf = this->allocate(_new_capacity);
                    usize _index = 0;

                    for (; _index < _size; ++_index)
                    {
                        auto& _value = at_unchecked(_index);
                        std::construct_at(&_new_buf[_index], move(_value));
                        _value.~T();
                    }
                    for (; _index < new_size; ++_index)
                    {
                        std::construct_at(&_new_buf[_index]);
                    }

                    this->deallocate(this->_data, _capacity);
                    this->_data = _new_buf;
                }

                _capacity = _new_capacity;
                _size = new_size;
            }
            else if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                    std::construct_at(&this->_data[_index]);
                
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    at_unchecked(_index - 1).~T();
                
                _size = new_size;
            }
        }

        HSD_CONSTEXPR void push_back(const T& val)
        {
            emplace_back(val);
        }

        HSD_CONSTEXPR void push_back(T&& val)
        {
            emplace_back(move(val));
        }

        template <typename... Args>
        HSD_CONSTEXPR void emplace_back(Args&&... args)
        {
            reserve(_size + 1);
            std::construct_at(&this->_data[_size], forward<Args>(args)...);
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
            if constexpr(is_same<decltype(this->_data), T*>::value)
            {
                return this->_data;
            }
            else
            {
                return &this->_data[0];
            }
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
            if constexpr(is_same<decltype(this->_data), T*>::value)
            {
                return this->_data;
            }
            else
            {
                return &this->_data[0];
            }
        }

        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }

    };

    template < typename T, usize N > vector(const T (&)[N]) -> vector<T>;
    template < typename T, usize N > vector(T (&&)[N]) -> vector<T>;
    template <typename T> using buffered_vector = vector< T, buffered_allocator<T> >;

    template< typename L, typename... U >
    requires (std::is_constructible_v<L, U> && ...)
    HSD_CONSTEXPR vector<L> make_vector(L&& first, U&&... rest)
    {
        constexpr usize size = 1 + sizeof...(U);
        vector<L> vec;
        vec.reserve(size);
        
        [&vec]<usize... _index>(hsd::index_sequence<_index...>, auto&... args)
        {
            (vec.emplace_back(forward<decltype(args)>(args)), ...);
        }(hsd::make_index_sequence<size>{}, first, rest...);
        
        return vec;
    }
}
