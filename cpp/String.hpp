#pragma once

#include "Result.hpp"
#include "CString.hpp"
#include "Math.hpp"

namespace hsd
{
    struct move_data_pointer {};

    template <typename CharT>
    class basic_string
    {
    private:
        using _str_utils = basic_cstring<CharT>;
        static inline CharT _empty_str[1]{};
        CharT* _data = nullptr;
        usize _size = 0;
        usize _capacity = 0;

        struct bad_access
        {
            const char* operator()() const
            {
                return "Tried to access an element out of bounds";
            }
        };
        
        struct out_of_range
        {
            const char* operator()() const
            {
                return "Out of range";
            }
        };

        inline void _reset()
        {
            delete[] _data;
            _data = nullptr;
        }
    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        static constexpr usize npos = static_cast<usize>(-1);

        inline basic_string() = default;

        inline basic_string(usize size)
        {
            _data = new CharT[size + 1]{};
            _size = size;
            _capacity = _size;
        }

        inline basic_string(const CharT* cstr)
        {
            _size = _str_utils::length(cstr);
            _capacity = _size;
            _data = new CharT[_size + 1]{};
            _str_utils::copy(_data, cstr, _size);
        }

        inline basic_string(const CharT* cstr, usize size)
        {
            _size = size;
            _capacity = _size;
            _data = new CharT[_size + 1]{};
            _str_utils::copy(_data, cstr, _size);
        }

        inline basic_string(CharT* cstr, move_data_pointer)
        {
            _size = _str_utils::length(cstr);
            _capacity = _size;
            _data = cstr;
        }

        inline basic_string(CharT* cstr, 
            usize size, move_data_pointer)
        {
            _size = size;
            _capacity = _size;
            _data = cstr;
        }

        inline basic_string(const basic_string& other)
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = new CharT[_capacity + 1]{};
            _str_utils::copy(_data, other._data, _size);
        }

        inline basic_string(basic_string&& other)
        {
            swap(_size, other._size);
            swap(_capacity, other._capacity);
            swap(_data, other._data);
        }

        inline ~basic_string()
        {
            _reset();
        }

        inline basic_string& operator=(const CharT* rhs)
        {
            _reset();
            _size = _str_utils::length(rhs);
            _capacity = _size;
            _data = new CharT[_size + 1]{};
            _str_utils::copy(_data, rhs, _size);
            return *this;
        }

        template <typename RhsCharT> requires(std::is_convertible_v<RhsCharT, CharT>)
        inline basic_string& operator=(const basic_string<RhsCharT>& rhs)
        {
            _reset();
            _size = rhs.size();
            _capacity = _size;
            _data = new CharT[_size + 1]{};
            copy_n(rhs.c_str(), _size, _data);
            return *this;
        }

        inline basic_string& operator=(const basic_string& rhs)
        {
            _reset();
            _size = rhs.size();
            _capacity = _size;
            _data = _str_utils::to_string(rhs._data, _size);
            return *this;
        }

        inline basic_string& operator=(basic_string&& rhs)
        {
            swap(_size, rhs._size);
            swap(_capacity, rhs._capacity);
            swap(_data, rhs._data);
            return *this;
        }

        inline basic_string operator+(const basic_string& rhs) const
        {
            basic_string _buf(_size + rhs._size);
            _str_utils::copy(_buf._data, _data, _size);
            _str_utils::add(_buf._data, rhs._data, _size);
            return _buf;
        }

        inline basic_string operator+(const CharT* rhs) const
        {
            usize _rhs_len = _str_utils::length(rhs);
            basic_string _buf(_size + _rhs_len);
            _str_utils::copy(_buf._data, _data, _size);
            _str_utils::add(_buf._data, rhs, _size);
            return _buf;
        }

        inline friend basic_string operator+(const CharT* lhs, const basic_string& rhs)
        {
            usize _lhs_len = _str_utils::length(lhs);
            basic_string _buf(rhs._size + _lhs_len);
            _str_utils::copy(_buf._data, lhs, _lhs_len);
            _str_utils::add(_buf._data, rhs._data, _lhs_len);
            return _buf;
        }

        inline basic_string& operator+=(const basic_string& rhs)
        {
            if(_capacity <= _size + rhs._size)
            {
                basic_string _buf(_size + rhs._size);
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

        inline basic_string& operator+=(const CharT* rhs)
        {
            usize _rhs_len = _str_utils::length(rhs);

            if(_capacity <= _size + _rhs_len)
            {
                basic_string _buf(_size + _rhs_len);
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

        inline CharT& operator[](usize index)
        {
            return _data[index];
        }

        inline const CharT& operator[](usize index) const
        {
            return _data[index];
        }

        inline bool operator==(const basic_string& rhs) const
        {
            return _str_utils::compare(
                _data, rhs._data, 
                hsd::math::min(_size, rhs._size)
            ) == 0;
        }

        inline bool operator!=(const basic_string& rhs) const
        {
            return !operator==(rhs);
        }

        inline bool operator<(const basic_string& rhs) const
        {
            return _str_utils::compare(
                _data, rhs._data, 
                hsd::math::min(_size, rhs._size)
            ) == -1;
        }

        inline bool operator<=(const basic_string& rhs) const
        {
            return operator<(rhs) && operator==(rhs);
        }

        inline bool operator>(const basic_string& rhs) const
        {
            return _str_utils::compare(_data, rhs._data, 
                hsd::math::min(_size, rhs._size)) == 1;
        }

        inline bool operator>=(const basic_string& rhs) const
        {
            return operator>(rhs) && operator==(rhs);
        }

        inline auto at(usize index)
            -> Result< reference<CharT>, bad_access >
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline auto at(usize index) const
            -> Result< reference<const CharT>, bad_access >
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline usize find(const basic_string& str, usize pos = 0) const
        {
            if(pos >= _size)
                return npos;
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str._data
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        inline usize find(const CharT* str, usize pos = 0) const
        {
            if(pos >= _size)
                return npos;
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        inline usize find(CharT str, usize pos = 0) const
        {
            if(pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str
                );

                if(_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline usize rfind(const basic_string& str, usize pos = npos) const
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size - pos
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        inline usize rfind(const CharT* str, usize pos = npos) const
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        inline usize rfind(CharT str, usize pos = npos) const
        {
            if(pos >= _size && pos != npos)
            {
                return npos;
            }
            else if(pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if(_find_addr == nullptr)
                    return npos;
                else
                    return _find_addr - _data;
            }
        }

        inline auto erase(const_iterator pos)
            -> Result<iterator, bad_access>
        {
            return erase_for(pos, pos);
        }

        inline auto erase_for(const_iterator from, const_iterator to)
            -> Result<iterator, bad_access>
        {
            if(from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            usize _current_pos = static_cast<usize>(from - begin());
            usize _last_pos = static_cast<usize>(to - begin());

            for(usize _index = 0; _index < _size - _last_pos + 1; _index++)
            {
                this->_data[_current_pos + _index] = 
                    move(this->_data[_last_pos + _index + 1]);
            }

            _size -= static_cast<usize>(to - from) + 1;
            return begin() + _current_pos;
        }

        inline CharT& front()
        {
            return _data[0];
        }

        inline CharT& back()
        {
            return _data[_size - 1];
        }

        inline usize size() const
        {
            return _size;
        }

        inline usize capacity() const
        {
            return _capacity;
        }

        template < usize Pos, usize Count >
        inline auto gen_range()
            -> Result<basic_string<CharT>, out_of_range>
        {
            if(Pos + Count > _size)
                return out_of_range{};
    
            return basic_string(&_data[Pos], Count);
        }

        inline void clear()
        {
            _data[0] = '\0';
            _size = 0;
        }
    
        inline void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;
                allocator<CharT> _alloc;

                while (_new_capacity < new_cap)
                    _new_capacity += (_new_capacity + 1) / 2;

                auto* _new_buf = _alloc.allocate(_new_capacity).unwrap();
                
                for (usize _index = 0; _index < _size; ++_index)
                {
                    auto& _value = _data[_index];
                    construct_at(&_new_buf[_index], move(_value));
                    _value.~CharT();
                }

                _alloc.deallocate(_data, _capacity).unwrap();
                _data = _new_buf;
                _capacity = _new_capacity;
            }
        }

        inline void push_back(const CharT& val)
        {
            if(_capacity > _size)
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

        inline void pop_back()
        {
            _data[--_size] = '\0';
        }

        inline iterator data()
        {
            return _data != nullptr ? _data : _empty_str;
        }

        inline const_iterator c_str() const
        {
            return _data != nullptr ? _data : _empty_str;
        }

        inline iterator begin()
        {
            return data();
        }

        inline const_iterator begin() const
        {
            return c_str();
        }

        inline iterator end()
        {
            return begin() + size();
        }

        inline const_iterator end() const
        {
            return begin() + size();
        }

        inline const_iterator cbegin()
        {
            return begin();
        }

        inline const_iterator cbegin() const
        {
            return begin();
        }

        inline const_iterator cend()
        {
            return end();
        }

        inline const_iterator cend() const
        {
            return end();
        }
    };

    template <typename T, typename CharT>
    static inline basic_string<CharT> to_string(T val)
    {
        const CharT* _cstr_buf = basic_cstring<CharT>::to_string(val);
        basic_string<CharT> _str_buf = basic_string<CharT>(_cstr_buf, move_data_pointer{});
        return _str_buf;
    }

    template <typename T>
    static inline basic_string<T>& to_string(basic_string<T>& val)
    {
        return val;
    }
    
    using string = basic_string<char>;
    using wstring = basic_string<wchar>;
    using u8string = basic_string<char8>;
    using u16string = basic_string<char16>;
    using u32string = basic_string<char32>;
} // namespace hsd
