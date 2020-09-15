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
        size_t _size = 0;
        size_t _capacity = 0;

    public:
        using value_type = T;
        using iterator = T*;
        using const_iterator = const T*;

        HSD_CXX20_CONSTEXPR ~vector()
        {
            for (size_t i = _size; i > 0; --i)
                at(i-1).~T();
            delete[] _data;
        }

        HSD_CXX20_CONSTEXPR vector(size_t size)
        {
            resize(size);
        }

        constexpr vector() noexcept = default;

        HSD_CXX20_CONSTEXPR vector(const vector& rhs)
            : _data(new storage_type[rhs._capacity]),
              _size(rhs._size), _capacity(rhs._capacity)
        {
            for (size_t i = 0; i < _size; ++i)
                new(&_data[i]) T(rhs[i]);
        }

        HSD_CXX20_CONSTEXPR vector(vector&& rhs) noexcept
            : _data(exchange(rhs._data, nullptr)),
              _size(exchange(rhs._size, 0)), _capacity(exchange(rhs._capacity, 0))
        {}

        HSD_CXX20_CONSTEXPR vector(std::initializer_list<T> list)
            : _data(new storage_type[list.size()]),
              _size(list.size()), _capacity(list.size())
        {
            auto p = list.begin();
            for (size_t i = 0; i < _size; ++i)
                new(&_data[i]) T(p[i]);
        }

        HSD_CXX20_CONSTEXPR vector& operator=(const vector& rhs)
        {
            if (_capacity < rhs._size)
            {
                clear();
                reserve(rhs._size);
                for (size_t i = 0; i < rhs._size; ++i)
                    new(&_data[i]) T(rhs[i]);
                _size = rhs._size;
            }
            else
            {
                size_t i;
                size_t min_size = _size > rhs._size ? _size : rhs._size;
                for (i = 0; i < min_size; ++i)
                    at(i) = rhs[i];
                if (_size > rhs._size)
                {
                    for (i = _size; i > rhs._size; --i)
                        at(i-1).~T();
                }
                else if (rhs._size > _size)
                {
                    for (; i < rhs._size; ++i)
                        new(&_data[i]) T(rhs[i]);
                }
                _size = rhs._size;
            }

            return *this;
        }

        HSD_CXX20_CONSTEXPR vector& operator=(std::initializer_list<T> list)
        {
            auto p = list.begin();
            if (_capacity < list.size())
            {
                clear();
                reserve(list.size());
                for (size_t i = 0; i < list.size(); ++i)
                    new(&_data[i]) T(p[i]);
                _size = list.size();
            }
            else
            {
                size_t i;
                size_t min_size = _size > list.size() ? _size : list.size();
                for (i = 0; i < min_size; ++i)
                    at(i) = p[i];
                if (_size > list.size())
                {
                    for (i = _size; i > list.size(); --i)
                        at(i-1).~T();
                }
                else if (list.size() > _size)
                {
                    for (; i < list.size(); ++i)
                        new(&_data[i]) T(p[i]);
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

        constexpr T& operator[](size_t index) noexcept
        {
            assert(index < _size);
            return reinterpret_cast<T&>(_data[index]);
        }

        constexpr T& at(size_t index) noexcept
        {
            assert(index < _size);
            return reinterpret_cast<T&>(_data[index]);
        }

        constexpr const T& operator[](size_t index) const noexcept
        {
            assert(index < _size);
            return reinterpret_cast<T&>(_data[index]);
        }

        constexpr const T& at(size_t index) const noexcept
        {
            assert(index < _size);
            return reinterpret_cast<T&>(_data[index]);
        }

        constexpr T& at_checked(size_t index)
        {
            if(index >= _size)
            {
                throw std::out_of_range("accessed element out of range");
            }

            return reinterpret_cast<T&>(_data[index]);
        }

        constexpr void clear() noexcept
        {
            for (size_t i = _size; i > 0; --i)
                at(i-1).~T();
            _size = 0;
        }

        HSD_CXX20_CONSTEXPR void reserve(size_t new_cap)
        {
            if (new_cap > _capacity)
            {
                //                    To handle _capacity = 0 case
                size_t new_capacity = _capacity ? _capacity : 1;
                while (new_capacity < new_cap)
                    new_capacity += (new_capacity + 1)/2;

                storage_type* new_buf = new storage_type[new_capacity];
                for (size_t i = 0; i < _size; ++i)
                {
                    auto& v = at(i);
                    new(&new_buf[i]) T(move(v));
                    v.~T();
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
                for (size_t i = 0; i < _size; ++i)
                {
                    auto& v = at(i);
                    new(&new_buf[i]) T(move(v));
                    v.~T();
                }
                storage_type* old_buf = exchange(_data, new_buf);
                _capacity = _size;
                delete[] old_buf;
            }
        }

        HSD_CXX20_CONSTEXPR void resize(size_t new_size)
        {
            if (new_size > _capacity)
            {
                //                    To handle _capacity = 0 case
                size_t new_capacity = _capacity ? _capacity : 1;
                while (new_capacity < new_size)
                    new_capacity += (new_capacity + 1)/2;

                storage_type* new_buf = new storage_type[new_capacity];
                size_t i;
                for (i = 0; i < _size; ++i)
                {
                    auto& v = at(i);
                    new(&new_buf[i]) T(move(v));
                    v.~T();
                }
                for (; i < new_size; ++i)
                {
                    new(&new_buf[i]) T();
                }
                storage_type* old_buf = exchange(_data, new_buf);
                _capacity = new_capacity;
                _size = new_size;
                delete[] old_buf;
            }
            else if (new_size > _size)
            {
                for (size_t i = _size; i < new_size; ++i)
                    new(&_data[i]) T();
                _size = new_size;
            }
            else if (new_size < _size)
            {
                for (size_t i = _size; i > new_size; --i)
                    operator[](i-1).~T();
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
            assert(_size > 0);
            at(--_size).~T();
        }

        constexpr size_t size() const
        {
            return _size;
        }

        constexpr size_t capacity() const
        {
            return _capacity;
        }

        constexpr iterator data()
        {
            return reinterpret_cast<iterator>(_data);
        }

        constexpr iterator begin()
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr const_iterator cbegin() const
        {
            return reinterpret_cast<const_iterator>(_data);
        }

        constexpr const_iterator cend() const
        {
            return cbegin() + size();
        }
    };

    template< typename L, typename... U >
    requires (std::is_constructible_v<L, U> && ...)
    HSD_CXX20_CONSTEXPR vector<L> make_vector(L&& first, U&&... rest)
    {
        constexpr size_t size = 1 + sizeof...(U);
        vector<L> vec;
        vec.reserve(size);
        vec.emplace_back(forward<L>(first));
        [&]<size_t... I>(std::index_sequence<I...>, auto tup)
        {
            (vec.emplace_back(forward<U>(tup.template get<I>())), ...);
        }(std::index_sequence_for<U...>(), tie(rest...));
        return vec;
    }
}
