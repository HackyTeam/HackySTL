#pragma once

#include <stdexcept>
#include <cassert>
#include <initializer_list>

#include "Utility.hpp"
#include "Tuple.hpp"
#include "AlignedStorage.hpp"

#ifndef HSD_DISABLE_VECTOR_CONSTEXPR
#define HSD_CXX20_CONSTEXPR constexpr
#else
#define HSD_CXX20_CONSTEXPR
#endif

namespace hsd
{
    template<typename T>
    class vector
    {
        using storage_type = typename aligned_storage<sizeof(T), alignof(T)>::type;
        storage_type* _data = nullptr;
        usize _size = 0;
        usize _capacity = 0;

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        HSD_CXX20_CONSTEXPR ~vector()
        {
            for (usize _index = _size; _index > 0; --_index)
                at_unchecked(_index - 1).~T();
                
            delete[] _data;
        }

        HSD_CXX20_CONSTEXPR vector(usize size)
        {
            resize(size);
        }

        HSD_CXX20_CONSTEXPR vector() noexcept = default;

        HSD_CXX20_CONSTEXPR vector(const vector& rhs)
            : _data(new storage_type[rhs._capacity]),
              _size(rhs._size), _capacity(rhs._capacity)
        {
            for (usize _index = 0; _index < _size; ++_index)
                new(&_data[_index]) T(rhs[_index]);
        }

        HSD_CXX20_CONSTEXPR vector(vector&& rhs) noexcept
            : _data(exchange(rhs._data, nullptr)),
              _size(exchange(rhs._size, 0)), _capacity(exchange(rhs._capacity, 0))
        {}

        HSD_CXX20_CONSTEXPR vector(std::initializer_list<T> list)
            : _data(new storage_type[list.size()]),
              _size(list.size()), _capacity(list.size())
        {
            auto _arr = list.begin();
            
            for (usize _index = 0; _index < _size; ++_index)
                new(&_data[_index]) T(_arr[_index]);
        }

        HSD_CXX20_CONSTEXPR vector& operator=(const vector& rhs)
        {
            if (_capacity < rhs._size)
            {
                clear();
                reserve(rhs._size);
                
                for (usize _index = 0; _index < rhs._size; ++_index)
                    new(&_data[_index]) T(rhs[_index]);
                
                _size = rhs._size;
            }
            else
            {
                usize _index;
                usize min_size = _size > rhs._size ? _size : rhs._size;
                
                for (_index = 0; _index < min_size; ++_index)
                    _data[_index] = rhs[_index];
                
                if (_size > rhs._size)
                {
                    for (_index = _size; _index > rhs._size; --_index)
                        _data[_index - 1].~T();
                }
                else if (rhs._size > _size)
                {
                    for (; _index < rhs._size; ++_index)
                        new(&_data[_index]) T(rhs[_index]);
                }

                _size = rhs._size;
            }

            return *this;
        }

        HSD_CXX20_CONSTEXPR vector& operator=(std::initializer_list<T> list)
        {
            auto _arr = list.begin();
            if (_capacity < list.size())
            {
                clear();
                reserve(list.size());
                
                for (usize _index = 0; _index < list.size(); ++_index)
                    new(&_data[_index]) T(_arr[_index]);
                
                _size = list.size();
            }
            else
            {
                usize _index;
                usize min_size = _size > list.size() ? _size : list.size();
                
                for (_index = 0; _index < min_size; ++_index)
                {
                    _data[_index] = _arr[_index];
                }
                if (_size > list.size())
                {
                    for (_index = _size; _index > list.size(); --_index)
                        _data[_index - 1].~T();
                }
                else if (list.size() > _size)
                {
                    for (; _index < list.size(); ++_index)
                        new(&_data[_index]) T(_arr[_index]);
                }
                _size = list.size();
            }

            return *this;
        }

        HSD_CXX20_CONSTEXPR vector& operator=(vector&& rhs) noexcept
        {
            clear();
            delete[] _data;

            _data = exchange(rhs._data, nullptr);
            _size = exchange(rhs._size, 0);
            _capacity = exchange(rhs._capacity, 0);
    
            return *this;
        }

        HSD_CXX20_CONSTEXPR T& operator[](usize index) noexcept
        {
            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR const T& operator[](usize index) const noexcept
        {
            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR T& front() noexcept
        {
            return *begin();
        }

        HSD_CXX20_CONSTEXPR T& front() const noexcept
        {
            return *begin();
        }

        HSD_CXX20_CONSTEXPR T& back() noexcept
        {
            return *(begin() + size() - 1);
        }

        HSD_CXX20_CONSTEXPR T& back() const noexcept
        {
            return *(begin() + size() - 1);
        }

        HSD_CXX20_CONSTEXPR T& at(usize index)
        {
            if(index >= _size)
                throw std::out_of_range("Accessed element out of range");

            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR const T& at(usize index) const
        {
            if(index >= _size)
                throw std::out_of_range("Accessed element out of range");

            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR T& at_unchecked(usize index) noexcept
        {
            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR T& at_unchecked(usize index) const noexcept
        {
            return reinterpret_cast<T&>(_data[index]);
        }

        HSD_CXX20_CONSTEXPR void clear() noexcept
        {
            for (usize _index = _size; _index > 0; --_index)
                _data[_index - 1].~T();
                
            _size = 0;
        }

        HSD_CXX20_CONSTEXPR void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize new_capacity = _capacity ? _capacity : 1;
                while (new_capacity < new_cap)
                    new_capacity += (new_capacity + 1)/2;

                storage_type* new_buf = new storage_type[new_capacity];
                
                for (usize _index = 0; _index < _size; ++_index)
                {
                    auto& _value = at_unchecked(_index);
                    new(&new_buf[_index]) T(move(_value));
                    _value.~T();
                }
                
                storage_type* old_buf = exchange(_data, new_buf);
                _capacity = new_capacity;
                delete[] old_buf;
            }
        }

        HSD_CXX20_CONSTEXPR void shrink_to_fit()
        {
            if (_size == 0)
            {
                storage_type* old_buf = exchange(_data, nullptr);
                _capacity = 0;
                delete[] old_buf;
            }
            else if (_size < _capacity)
            {
                storage_type* new_buf = new storage_type[_size];
                
                for (usize _index = 0; _index < _size; ++_index)
                {
                    auto& _value = _data[_index];
                    new(&new_buf[_index]) T(move(_value));
                    _value.~T();
                }
                
                storage_type* old_buf = exchange(_data, new_buf);
                _capacity = _size;
                delete[] old_buf;
            }
        }

        HSD_CXX20_CONSTEXPR void resize(usize new_size)
        {
            if (new_size > _capacity)
            {
                //                    To handle _capacity = 0 case
                usize new_capacity = _capacity ? _capacity : 1;
                while (new_capacity < new_size)
                    new_capacity += (new_capacity + 1) / 2;

                storage_type* new_buf = new storage_type[new_capacity];
                usize _index;
                
                for (_index = 0; _index < _size; ++_index)
                {
                    auto& _value = at_unchecked(_index);
                    new(&new_buf[_index]) T(move(_value));
                    _value.~T();
                }
                for (; _index < new_size; ++_index)
                {
                    new(&new_buf[_index]) T();
                }
                
                storage_type* old_buf = exchange(_data, new_buf);
                _capacity = new_capacity;
                _size = new_size;
                delete[] old_buf;
            }
            else if (new_size > _size)
            {
                for (usize _index = _size; _index < new_size; ++_index)
                    new(&_data[_index]) T();
                
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (usize _index = _size; _index > new_size; --_index)
                    at_unchecked(_index - 1).~T();
                
                _size = new_size;
            }
        }

        HSD_CXX20_CONSTEXPR void push_back(const T& val)
        {
            emplace_back(val);
        }

        HSD_CXX20_CONSTEXPR void push_back(T&& val)
        {
            emplace_back(move(val));
        }

        template <typename... Args>
        HSD_CXX20_CONSTEXPR void emplace_back(Args&&... args)
        {
            reserve(_size + 1);
            new(&_data[_size]) T(forward<Args>(args)...);
            ++_size;
        }

        HSD_CXX20_CONSTEXPR void pop_back() noexcept
        {
            if(_size < 0)
            {
                at_unchecked(_size - 1).~T();
                _size--;
            }
        }

        HSD_CXX20_CONSTEXPR usize size() const
        {
            return _size;
        }

        HSD_CXX20_CONSTEXPR usize capacity() const
        {
            return _capacity;
        }

        HSD_CXX20_CONSTEXPR iterator data()
        {
            return reinterpret_cast<iterator>(_data);
        }

        HSD_CXX20_CONSTEXPR iterator begin()
        {
            return data();
        }

        HSD_CXX20_CONSTEXPR iterator end()
        {
            return begin() + size();
        }

        HSD_CXX20_CONSTEXPR const_iterator cbegin() const
        {
            return reinterpret_cast<const_iterator>(_data);
        }

        HSD_CXX20_CONSTEXPR const_iterator cend() const
        {
            return end();
        }

    };

    template< typename L, typename... U >
    requires (std::is_constructible_v<L, U> && ...)
    HSD_CXX20_CONSTEXPR vector<L> make_vector(L&& first, U&&... rest)
    {
        constexpr usize size = 1 + sizeof...(U);
        vector<L> vec;
        vec.reserve(size);
        vec.emplace_back(forward<L>(first));
        [&]<usize... _index>(hsd::index_sequence<_index...>, auto tup)
        {
            (vec.emplace_back(forward<U>(tup.template get<_index>())), ...);
        }(hsd::index_sequence_for<U...>(), tie(rest...));
        return vec;
    }
}
