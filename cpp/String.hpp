#pragma once

#include "CString.hpp"

#include <stdexcept>

namespace hsd
{
    template<typename CharT>
    class string
    {
    private:
        using _str_utils = cstring<CharT>;
        CharT* _data = nullptr;
        usize _size = 0;
        usize _reserved_size = 1;

        HSD_CONSTEXPR void _reset()
        {
            delete[] _data;
        }
    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        static constexpr isize npos = -1;

        HSD_CONSTEXPR string()
        {
            _data = new CharT[1];
            _data[0] = '\0';
        }

        HSD_CONSTEXPR string(usize size)
        {
            _data = new CharT[size + 1];
            _data[size] = '\0';
            _size = size;
            _reserved_size = _size;
        }

        HSD_CONSTEXPR string(const CharT* cstr)
        {
            _size = _str_utils::length(cstr);
            _reserved_size = _size;
            _data = new CharT[_size + 1];
            _str_utils::copy(_data, cstr, _size);
            _data[_size] = '\0';
        }

        HSD_CONSTEXPR string(const CharT* cstr, usize size)
        {
            _size = size;
            _reserved_size = _size;
            _data = new CharT[_size + 1];
            _str_utils::copy(_data, cstr, _size);
            _data[size] = '\0';
        }

        HSD_CONSTEXPR string(const string& other)
        {
            _size = other._size;
            _reserved_size = other._reserved_size;
            _data = new CharT[_reserved_size + 1];
            _str_utils::copy(_data, other._data, _size);
            _data[_size] = '\0';
        }

        constexpr string(string&& other)
        {
            _size = other._size;
            _reserved_size = other._reserved_size;
            _data = other._data;
            other._data = nullptr;
        }

        HSD_CONSTEXPR ~string()
        {
            _reset();
        }

        HSD_CONSTEXPR string& operator=(const CharT* rhs)
        {
            _reset();
            _size = _str_utils::length(rhs);
            _reserved_size = _size;
            _data = new CharT[_size + 1];
            _str_utils::copy(_data, rhs, _size);
            _data[_size] = '\0';
            return *this;
        }

        template<typename RhsCharT>
        HSD_CONSTEXPR string& operator=(const string<RhsCharT>& rhs)
        {
            _reset();
            _size = rhs.size();
            _reserved_size = _size;
            _data = _str_utils::to_string(rhs.c_str(), _size);
            return *this;
        }

        HSD_CONSTEXPR string& operator=(const string& rhs)
        {
            _reset();
            _size = rhs.size();
            _reserved_size = _size;
            _data = _str_utils::to_string(rhs._data, _size);
            return *this;
        }

        HSD_CONSTEXPR string& operator=(string&& rhs)
        {
            _reset();
            _size = rhs._size;
            _reserved_size = rhs._reserved_size;
            _data = rhs._data;
            rhs._data = nullptr;
            return *this;
        }

        template<typename T>
        static HSD_CONSTEXPR string to_string(T val)
        {
            const CharT* _cstr_buf = _str_utils::to_string(val);
            string _str_buf = string(_cstr_buf);
            delete[] _cstr_buf;
            return _str_buf;
        }

        template<typename T>
        static constexpr string<T>& to_string(string<T>& val)
        {
            return val;
        }

        HSD_CONSTEXPR string operator+(const string& rhs)
        {
            string _buf(_size + rhs._size);
            _str_utils::copy(_buf._data, _data, _size);
            _str_utils::add(_buf._data, rhs._data, _size);
            return _buf;
        }

        HSD_CONSTEXPR string operator+(const CharT* rhs)
        {
            usize _rhs_len = _str_utils::length(rhs);
            string _buf(_size + _rhs_len);
            _str_utils::copy(_buf._data, _data, _size);
            _str_utils::add(_buf._data, rhs, _size);
            return _buf;
        }

        HSD_CONSTEXPR friend string operator+(const CharT* lhs, const string& rhs)
        {
            usize _lhs_len = _str_utils::length(lhs);
            string _buf(rhs._size + _lhs_len);
            _str_utils::copy(_buf._data, lhs, _lhs_len);
            _str_utils::add(_buf._data, rhs._data, _lhs_len);
            return _buf;
        }

        HSD_CONSTEXPR string& operator+=(const string& rhs)
        {
            if(_reserved_size <= _size + rhs._size)
            {
                string _buf(_size + rhs._size);
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs._data, _size);
                operator=(hsd::move(_buf));
                return *this;
            }
            else
            {
                _str_utils::add(_data, rhs._data, _size);
                return *this;
            }
        }

        HSD_CONSTEXPR string& operator+=(const CharT* rhs)
        {
            usize _rhs_len = _str_utils::length(rhs);

            if(_reserved_size <= _size + _rhs_len)
            {
                string _buf(_size + _rhs_len);
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs, _size);
                operator=(hsd::move(_buf));
                return *this;
            }
            else
            {
                _str_utils::add(_data, rhs, _size);
                return *this;
            }
        }

        constexpr CharT& operator[](usize index)
        {
            return _data[index];
        }

        constexpr bool operator==(const string& rhs)
        {
            return _str_utils::compare(
                _data, rhs._data, 
                hsd::min(_size, rhs._size)
            ) == 0;
        }

        constexpr bool operator!=(const string& rhs)
        {
            return !operator==(rhs);
        }

        constexpr bool operator<(const string& rhs)
        {
            return _str_utils::compare(
                _data, rhs._data, 
                hsd::min(_size, rhs._size)
            ) == -1;
        }

        constexpr bool operator<=(const string& rhs)
        {
            return operator<(rhs) && operator==(rhs);
        }

        constexpr bool operator>(const string& rhs)
        {
            return _str_utils::compare(_data, rhs._data, 
                hsd::min(_size, rhs._size)) == 1;
        }

        constexpr bool operator>=(const string& rhs)
        {
            return operator>(rhs) && operator==(rhs);
        }

        constexpr CharT& at(usize index)
        {
            if(index >= _size)
                throw std::out_of_range("");

            return _data[index];
        }

        constexpr usize find(const string& str, usize pos = 0)
        {
            if(pos >= _size)
                return npos;
            else
            {
                const CharT* _find_addr = _str_utils::find(&_data[pos], str._data);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr usize find(const CharT* str, usize pos = 0)
        {
            if(pos >= _size)
                return npos;
            else
            {
                const CharT* _find_addr = _str_utils::find(&_data[pos], str);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr usize find(CharT str, usize pos = 0)
        {
            if(pos >= _size)
                return npos;
            else
            {
                const CharT* _find_addr = _str_utils::find(&_data[pos], str);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr usize rfind(const string& str, usize pos = npos)
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str._data, _size);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str._data, _size - pos);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr usize rfind(const CharT* str, usize pos = npos)
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str, _size);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str, _size - pos);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr usize rfind(CharT str, usize pos = npos)
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str, _size);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(&_data[pos], str, _size - pos);

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        constexpr CharT& front()
        {
            return _data[0];
        }

        constexpr CharT& back()
        {
            return _data[_size - 1];
        }

        constexpr usize size() const
        {
            return _size;
        }

        constexpr usize capacity() const
        {
            return _reserved_size;
        }

        template< usize Pos, usize Count >
        HSD_CONSTEXPR string gen_range()
        {
            if(Pos + Count > _size)
            {    
                throw std::out_of_range("");
            }
    
            return string(&_data[Pos], Count);
        }

        HSD_CONSTEXPR void clear()
        {
            if(_data != nullptr)
                _reset();
    
            _data = new CharT[1];
            _data[0] = '\0';
            _size = 0;
            _reserved_size = 1;
        }
    
        HSD_CONSTEXPR void reserve(usize size)
        {
            _reserved_size = size;
            CharT *_buf = new CharT[size + 1];
            
            if(_data != nullptr)
            {
                hsd::move(begin(), end(), _buf);
                _buf[_size] = '\0';
                _reset();
            }
    
            _data = _buf;
        }
        HSD_CONSTEXPR void push_back(const CharT& val)
        {
            if(_reserved_size > _size)
            {
                _data[_size] = val;
                _data[++_size] = '\0';
                return;
            }
            else
            {
                reserve(_size * 2);
                _data[_size] = val;
                _data[++_size] = '\0';
            }
        }

        HSD_CONSTEXPR void pop_back()
        {
            _data[--_size] = '\0';
        }

        constexpr iterator data()
        {
            return _data;
        }

        constexpr const_iterator c_str() const
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
    
    using wstring = hsd::string<wchar>;
    using u8string = hsd::string<char>;
    using u16string = hsd::string<char16>;
    using u32string = hsd::string<char32>;
} // namespace hsd
