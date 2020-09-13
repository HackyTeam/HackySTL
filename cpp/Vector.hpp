#pragma once

#include <stdexcept>

#include "Utility.hpp"
#include "InitializerList.hpp"

namespace hsd
{
    template<typename T>
    class vector
    {
    private:
        T *_data = nullptr;
        usize _size = 0;
        usize _reserved_size = 1;
    
        constexpr vector(T* data, usize size)
        {
            _size = size;
            _reserved_size = size * 2;
            _data = new T[_reserved_size];
            hsd::copy(data, data + size, begin());
        }
    public:
        using iterator = T*;
        using const_iterator = const T*;
    
        constexpr ~vector()
        {
            delete[] _data;
        }
    
        constexpr vector(usize size)
        {
            _data = new T[size];
            _size = size;
            _reserved_size = _size;
        }
    
        constexpr vector()
        {
            _data = new T[1];
        }
    
        template< typename L, typename... U >
        constexpr vector(const L& value, const U&... values)
        {
            _size = 1 + sizeof...(U);
            _reserved_size = _size;
            T arr[] = {value, values...};
            _data = new T[_reserved_size];
            hsd::copy(arr, arr + _size, begin());
        }
    
        constexpr vector(const vector& lhs)
        {
            _data = new T[lhs._reserved_size];
            hsd::copy(lhs.begin(), lhs.end(), begin());
            _reserved_size = lhs._reserved_size;
            _size = lhs._size;
        }
    
        constexpr vector(vector&& lhs)
        {
            _data = lhs._data;
            lhs._data = nullptr;
            _reserved_size = lhs._reserved_size;
            _size = lhs._size;
        }
    
        constexpr vector& operator=(const vector& lhs)
        {
            if(_data != nullptr)
            {
                delete[] _data;
            }
    
            _data = new T[lhs._reserved_size];
            hsd::copy(lhs.begin(), lhs.end(), begin());
            _reserved_size = lhs._reserved_size;
            _size = lhs._size;
    
            return *this;
        }
    
        constexpr vector& operator=(vector&& lhs)
        {
            if(_data != nullptr)
            {
                delete[] _data;
            }
    
            _data = lhs._data;
            lhs._data = nullptr;
            _reserved_size = lhs._reserved_size;
            _size = lhs._size;
    
            return *this;
        }
    
        constexpr T& operator[](usize index)
        {
            return _data[index];
        }

        constexpr T& operator[](usize index) const
        {
            return _data[index];
        }
    
        constexpr T& at(usize index)
        {
            if(index >= _size)
                throw std::out_of_range("");
    
            return _data[index];
        }

        constexpr T& at(usize index) const
        {
            if(index >= _size)
                throw std::out_of_range("");

            return _data[index];
        }
        
        constexpr T& front()
        {
            return *begin();
        }

        constexpr T& front() const
        {
            return *begin();
        }

        constexpr T& back()
        {
            return *(begin() + size() - 1);
        }

        constexpr T& back() const
        {
            return *(begin() + size() - 1);
        }
    
        template< usize U, usize L >
        constexpr auto gen_range()
        {
            if(U > _size || L > _size)
            {    
                throw std::out_of_range("");
            }
    
            return vector(&_data[U], L - U);
        }
    
        constexpr void clear()
        {
            if(_data != nullptr)
            {
                delete[] _data;
            }
    
            _data = new T[1];
            _size = 0;
            _reserved_size = 1;
        }
    
        constexpr void reserve(usize size)
        {
            _reserved_size = size;
            T *_buf = new T[size];
            
            if(_data != nullptr)
            {
                hsd::move(begin(), end(), _buf);
                delete[] _data;
            }
    
            _data = _buf;
        }

        constexpr void resize(usize size)
        {
            _reserved_size = size;
            T *_buf = new T[size];
            
            if(_data != nullptr)
            {
                hsd::move(begin(), end(), _buf);
                delete[] _data;
            }
    
            _size = size;
            _data = _buf;
        }

        constexpr void push_back(const T& val)
        {
            if(_reserved_size > _size)
            {
                _data[_size] = val;
                _size++;
                return;
            }
            else
            {
                reserve(_size * 2);
                _data[_size] = val;
                _size++;
            }
        }
    
        constexpr void push_back(T&& val)
        {
            if(_reserved_size > _size)
            {
                _data[_size] = hsd::move(val);
                _size++;
                return;
            }
            else
            {
                reserve(_size * 2);
                _data[_size] = hsd::move(val);
                _size++;
            }
        }
    
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if(_reserved_size > _size)
            {
                _data[_size].~T();
                new (&_data[_size]) T(hsd::forward<Args>(args)...);
                _size++;
                return;
            }
            else
            {
                reserve(_size * 2);
                _data[_size].~T();
                new (&_data[_size]) T(hsd::forward<Args>(args)...);
                _size++;
            }
        }
    
        constexpr void pop_back()
        {
            _data[--_size].~T();
        }
    
        constexpr usize size()
        {
            return _size;
        }

        constexpr usize size() const
        {
            return _size;
        }
    
        constexpr usize capacity()
        {
            return _reserved_size;
        }

        constexpr usize capacity() const
        {
            return _reserved_size;
        }
    
        constexpr iterator data()
        {
            return _data;
        }

        constexpr iterator data() const
        {
            return _data;
        }
    
        constexpr iterator begin()
        {
            return data();
        }

        constexpr iterator begin() const
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr iterator end() const
        {
            return begin() + size();
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
    
    template< typename L, typename... U > vector(const L&, const U&...) -> vector<L>;
}