#pragma once

#include "CString.hpp"
#include "StringView.hpp"
#include "Algorithm.hpp"
#include "_SStreamDetail.hpp"

namespace hsd
{
    template < typename CharT, template <typename> typename Allocator = allocator >
    class basic_string
    {
    private:
        using _str_utils = basic_cstring<CharT>;
        static inline const CharT _s_empty = 0;
        using alloc_type = Allocator<CharT>;
        
        alloc_type _alloc;
        CharT* _data = nullptr;
        usize _size = 0;
        usize _capacity = 0;

        inline void _reset()
        {
            _alloc.deallocate(_data, _capacity + 1).unwrap();
            _data = nullptr;
        }

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        using value_type = CharT;
        static constexpr usize npos = static_cast<usize>(-1);

        inline basic_string()
        requires (DefaultConstructible<alloc_type>) = default;

        template <typename Alloc = alloc_type>
        inline basic_string(const Alloc& alloc)
        requires (Constructible<alloc_type, const Alloc&>)
            : _alloc{alloc}
        {}

        inline basic_string(usize size)
        requires (DefaultConstructible<alloc_type>)
        {
            _data = _alloc.allocate(size + 1).unwrap();
            _capacity = size;
        }

        template <typename Alloc = alloc_type>
        inline basic_string(usize size, const Alloc& alloc)
        requires (Constructible<alloc_type, Alloc>)
            : _alloc{alloc}
        {
            _data = _alloc.allocate(size + 1).unwrap();
            _capacity = size;
        }

        inline basic_string(const CharT* cstr)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) : 0;
            _capacity = _size;
            _data = _alloc.allocate(_size + 1).unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, _size);
            }

            _data[_size] = static_cast<CharT>(0);
        }

        template <typename Alloc = alloc_type>
        inline basic_string(const CharT* cstr, const Alloc& alloc)
        requires (Constructible<alloc_type, Alloc>)
            : _alloc{alloc}
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) : 0;
            _capacity = _size;
            _data = _alloc.allocate(_size + 1).unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, _size);
            }

            _data[_size] = static_cast<CharT>(0);
        }

        inline basic_string(const CharT* cstr, usize size)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            _size = size;
            _capacity = _size;
            _data = _alloc.allocate(_size + 1).unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, _size);
            }

            _data[_size] = static_cast<CharT>(0);
        }

        template <typename Alloc = alloc_type>
        inline basic_string(const CharT* cstr, usize size, const Alloc& alloc)
        requires (Constructible<alloc_type, Alloc>)
            : _alloc{alloc}
        {
            _size = size;
            _capacity = _size;
            _data = _alloc.allocate(_size + 1).unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, _size);
            }

            _data[_size] = static_cast<CharT>(0);
        }

        inline basic_string(basic_string_view<CharT> view)
        requires (DefaultConstructible<alloc_type>)
            : basic_string{view.data(), view.size()}
        {}

        template <typename Alloc = alloc_type>
        inline basic_string(basic_string_view<CharT> view, const Alloc& alloc)
        requires (Constructible<alloc_type, Alloc>)
            : basic_string{view.data(), view.size(), alloc}
        {}

        inline basic_string(const basic_string& other)
            : _alloc{other._alloc}
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = _alloc.allocate(_capacity + 1).unwrap();
            
            if (other._data != nullptr)
            {
                _str_utils::copy(_data, other._data, _size);
            }

            _data[_size] = static_cast<CharT>(0);
        }

        template <typename CharT2>
        inline basic_string(const basic_string<CharT2, Allocator>& other)
            : _alloc{other._alloc}
        {
            _size = _capacity = unicode::length(other._data);
            _data = _alloc.allocate(_capacity + 1).unwrap();
            
            if constexpr (sizeof(CharT) == 1)
            {
                unicode::to_utf8(_data, other._data);
            }
            else if constexpr (sizeof(CharT) == 2)
            {
                unicode::to_utf16(_data, other._data);
            }
            else if constexpr (sizeof(CharT) == 4)
            {
                unicode::to_utf32(_data, other._data);
            }
            else
            {
                panic("Unsupported character size");
            }

            _data[_size] = static_cast<CharT>(0);
        }

        inline basic_string(basic_string&& other)
            : _alloc{move(other._alloc)}
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
            auto _new_size = _str_utils::length(rhs);
            reserve(_new_size);
            _size = _new_size;
            _str_utils::copy(_data, rhs, _size);
            _data[_size] = static_cast<CharT>(0);
            return *this;
        }

        inline basic_string& operator=(const basic_string_view<CharT>& rhs)
        {
            auto _new_size = rhs.size();
            reserve(_new_size);
            _size = _new_size;
            copy_n(rhs.data(), _size, _data);
            _data[_size] = static_cast<CharT>(0);
            return *this;
        }

        inline basic_string& operator=(const basic_string& rhs)
        {
            _reset();
            _alloc = rhs._alloc;
            _size = rhs._size;
            _capacity = _size;
            _data = _alloc.allocate(_size + 1).unwrap();
            copy_n(rhs.c_str(), _size, _data);
            _data[_size] = static_cast<CharT>(0);
            return *this;
        }

        template <typename CharT2>
        inline basic_string& operator=(const basic_string<CharT2, Allocator>& rhs)
        {
            _reset();
            _alloc = rhs._alloc;
            _size = _capacity = unicode::length(rhs._data);
            _data = _alloc.allocate(_size + 1).unwrap();
            
            if constexpr (sizeof(CharT) == 1)
            {
                unicode::to_utf8(_data, rhs._data);
            }
            else if constexpr (sizeof(CharT) == 2)
            {
                unicode::to_utf16(_data, rhs._data);
            }
            else if constexpr (sizeof(CharT) == 4)
            {
                unicode::to_utf32(_data, rhs._data);
            }
            else
            {
                panic("Unsupported character size");
            }

            _data[_size] = static_cast<CharT>(0);
            return *this;
        }

        inline basic_string& operator=(basic_string&& rhs)
        {
            swap(_alloc, rhs._alloc);
            swap(_size, rhs._size);
            swap(_capacity, rhs._capacity);
            swap(_data, rhs._data);
            return *this;
        }

        inline basic_string operator+(const basic_string& rhs) const
        {
            if (rhs.data() == nullptr || _data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else
            {
                basic_string _buf(_size + rhs._size);
                _buf._size = _size + rhs._size;
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs._data, _size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                return _buf;
            }
        }

        inline basic_string operator+(const basic_string_view<CharT>& rhs) const
        {
            if (rhs.data() == nullptr || _data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else
            {
                basic_string _buf(_size + rhs.size());
                _buf._size = _size + rhs.size();
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs.data(), _size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                return _buf;
            }
        }

        inline basic_string operator+(const CharT* rhs) const
        {
            if (rhs == nullptr || _data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else
            {
                usize _rhs_len = _str_utils::length(rhs);
                basic_string _buf(_size + _rhs_len);
                _buf._size = _size + _rhs_len;
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs, _size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                return _buf;
            }
        }

        inline friend basic_string operator+(
            const basic_string_view<CharT>& lhs, const basic_string& rhs)
        {
            if (lhs.data() == nullptr || rhs._data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else
            {
                basic_string _buf(rhs._size + lhs._size);
                _str_utils::copy(_buf._data, lhs.data(), lhs._size);
                _str_utils::add(_buf._data, rhs._data, lhs._size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                return _buf;
            }
        }

        inline friend basic_string operator+(const CharT* lhs, const basic_string& rhs)
        {
            if (lhs == nullptr || rhs._data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else
            {
                usize _lhs_len = _str_utils::length(lhs);
                basic_string _buf(rhs._size + _lhs_len);
                _str_utils::copy(_buf._data, lhs, _lhs_len);
                _str_utils::add(_buf._data, rhs._data, _lhs_len);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                return _buf;
            }
        }

        inline basic_string& operator+=(const basic_string& rhs)
        {
            if (rhs.data() == nullptr || _data == nullptr)
            {
                panic("Cannot concatenate null strings");
            }
            else if (_capacity <= _size + rhs._size)
            {
                basic_string _buf(_size + rhs._size);
                _buf._size = _size + rhs._size;
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs._data, _size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                operator=(hsd::move(_buf));
                return *this;
            }
            else
            {
                _str_utils::add(_data, rhs._data, _size);
                _size += rhs._size;
                _data[_size] = static_cast<CharT>(0);
                return *this;
            }
        }

        inline basic_string& operator+=(const basic_string_view<CharT>& rhs)
        {
            if (rhs.data() == nullptr || _data == nullptr)
            {
                panic("Error: nullptr argument.");
            }
            else if (_capacity <= _size + rhs.size())
            {
                basic_string _buf(_size + rhs.size());
                _buf._size = _size + rhs.size();
                _str_utils::copy(_buf._data, _data, _size);
                _str_utils::add(_buf._data, rhs.data(), _size);
                _buf._data[_buf._size] = static_cast<CharT>(0);
                operator=(hsd::move(_buf));
                return *this;
            }
            else
            {
                _str_utils::add(_data, rhs.data(), _size);
                _size += rhs.size();
                _data[_size] = static_cast<CharT>(0);
                return *this;
            }
        }

        inline basic_string& operator+=(const CharT* rhs)
        {
            if (rhs == nullptr || _data == nullptr)
            {
                panic("Error: nullptr argument.");
            }
            else 
            {
                usize _rhs_len = _str_utils::length(rhs);

                if (_capacity <= _size + _rhs_len)
                {
                    basic_string _buf(_size + _rhs_len);
                    _buf._size = _size + _rhs_len;
                    _str_utils::copy(_buf._data, _data, _size);
                    _str_utils::add(_buf._data, rhs, _size);
                    _buf._data[_buf._size] = static_cast<CharT>(0);
                    operator=(hsd::move(_buf));
                    return *this;
                }
                else
                {
                    _str_utils::add(_data, rhs, _size);
                    _data[_size] = static_cast<CharT>(0);
                    return *this;
                }
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
            return _size == rhs._size && _str_utils::compare(_data, rhs._data) == 0;
        }

        inline bool operator!=(const basic_string& rhs) const
        {
            return !operator==(rhs);
        }

        inline bool operator<(const basic_string& rhs) const
        {
            return _str_utils::compare(_data, rhs._data) == -1;
        }

        inline bool operator<=(const basic_string& rhs) const
        {
            auto _comp_rez = _str_utils::compare(_data, rhs._data);

            return _comp_rez == -1 || _comp_rez == 0;
        }

        inline bool operator>(const basic_string& rhs) const
        {
            return _str_utils::compare(_data, rhs._data) == 1;
        }

        inline bool operator>=(const basic_string& rhs) const
        {
            auto _comp_rez = _str_utils::compare(_data, rhs._data);

            return _comp_rez == 1 || _comp_rez == 0;
        }

        inline auto at(usize index)
            -> result<reference<CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline auto at(usize index) const
            -> result<reference<const CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        inline usize find(const basic_string& str, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str._data
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline usize find(const CharT* str, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline usize find(CharT letter, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], letter
                );

                if (_find_addr == nullptr)
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
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size
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
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline usize rfind(const CharT* str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline usize rfind(CharT str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        inline bool starts_with(CharT letter) const
        {
            if (_data != nullptr)
                return _data[0] == letter;

            return false;
        }

        inline bool starts_with(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        inline bool starts_with(const basic_string& str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        inline bool contains(CharT letter) const
        {
            if (_data != nullptr)
                return find(letter) != npos;

            return false;
        }

        inline bool contains(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        inline bool contains(const basic_string& str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        inline bool ends_with(CharT letter) const
        {
            if (_data != nullptr)
                return _data[_size - 1] == letter;

            return false;
        }

        inline bool ends_with(const CharT* str) const
        {
            usize _len = cstring::length(str);

            if (_data != nullptr)
                return rfind(str) == (_size - _len);

            return false;
        }

        inline bool ends_with(const basic_string& str) const
        {
            if (_data != nullptr)
                return rfind(str) == (_size - str._size);

            return false;
        }

        inline auto sub_string(usize from, usize count)
            -> result<basic_string, bad_access>
        {
            if (from > _size || (from + count) > _size)
                return bad_access{};

            return basic_string{_data + from, count};
        }

        inline auto sub_string(usize from)
            -> result<basic_string, bad_access>
        {
            return sub_string(from, _size - from);
        }

        inline auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        inline auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            usize _current_pos = static_cast<usize>(from - begin());
            usize _last_pos = static_cast<usize>(to - begin());

            for (usize _index = 0; _index < _capacity - _last_pos + 1; _index++)
            {
                this->_data[_current_pos + _index] = 
                    move(this->_data[_last_pos + _index]);
            }

            _size -= static_cast<usize>(to - from) + 1;
            return begin() + _last_pos;
        }
    
        inline void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;

                while (_new_capacity < new_cap)
                    _new_capacity += (_new_capacity + 1) / 2;

                // Allocate space for NULL byte
                auto* _new_buf = 
                    _alloc.allocate(_new_capacity + 1).unwrap();

                _new_buf[_new_capacity] = 0;
                
                for (usize _index = 0; _index < _size; ++_index)
                {
                    auto& _value = _data[_index];
                    _new_buf[_index] = move(_value);
                }

                _alloc.deallocate(_data, _capacity + 1).unwrap();
                _data = _new_buf;
                _capacity = _new_capacity;
            }
        }

        template <typename... Args>
        inline void emplace_back(Args&&... args)
        {
            reserve(_size + 2);
            _data[_size] = CharT{forward<Args>(args)...};
            _data[++_size] = '\0';
        }

        inline void push_back(const CharT& val)
        {
            emplace_back(val);
        }

        inline void push_back(CharT&& val)
        {
            emplace_back(move(val));
        }

        inline void clear()
        {
            if (_capacity != 0)
            {
                _data[0] = '\0';
                _size = 0;
            }
        }

        inline void pop_back()
        {
            if (_size > 0)
            {
                _data[_size--] = '\0';
            }
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
            return (_data != nullptr) ? _size + 1 : 0;
        }

        inline usize length() const
        {
            return _size;
        }

        inline usize capacity() const
        {
            return _capacity;
        }

        inline iterator data()
        {
            return _data;
        }

        inline const_iterator data() const
        {
            return _data;
        }

        inline const_iterator c_str() const
        {
            return _data != nullptr ? _data : &_s_empty;
        }

        inline iterator begin()
        {
            return data();
        }

        inline const_iterator begin() const
        {
            return data();
        }

        inline iterator end()
        {
            return begin() + size();
        }

        inline const_iterator end() const
        {
            return begin() + size();
        }

        inline const_iterator cbegin() const
        {
            return begin();
        }

        inline const_iterator cend() const
        {
            return end();
        }

        explicit constexpr operator basic_string_view<CharT>() const
        {
            return basic_string_view<CharT>(_data, _size);
        }

        template <typename CharU>
        friend auto _parse_impl(sstream_detail::stream_parser<CharU>& p, basic_string& str)
            -> option_err<runtime_error>
        {
            if (p.index() >= p.data().second)
            {
                return runtime_error {
                    "Invalid format: unexpected end of format string"
                };
            }

            str.clear();

            if (str.size() == 0)
            {
                str.emplace_back('\0');
            }

            for (
                usize _index = 0; p.data().first[p.index()][_index] != '\0' && 
                !basic_cstring<CharT>::iswhitespace(p.data().first[p.index()][_index]); ++_index)
            {
                str.back() = p.data().first[p.index()][_index];
                str.emplace_back('\0');
            }

            str._size--;
            p.index()++;
            return {};
        }

        constexpr auto pretty_args() const
        {
            return make_tuple(size(), data());
        }
    };

    template <typename CharT, usize N>
    class static_basic_string
    {
    private:
        using _str_utils = basic_cstring<CharT>;
        CharT _data[N]{};
        usize _size = 0;
        usize _capacity = N;

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        using value_type = CharT;
        static constexpr usize npos = static_cast<usize>(-1);

        constexpr static_basic_string() = default;

        constexpr static_basic_string(usize size)
        {
            if (size < N)
            {
                _size = size;
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%zu)"
                    ": size is too big\n", size
                );

                abort();
            }
        }

        constexpr static_basic_string(const CharT* cstr)
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) : 0;

            if (_size < N)
            {
                if (cstr != nullptr)
                {
                    _str_utils::copy(_data, cstr, _size);
                }

                _data[_size] = static_cast<CharT>(0);
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%s)"
                    ": cstr is too big to be contained\n", cstr
                );

                abort();
            }
        }

        constexpr static_basic_string(const CharT* cstr, usize size)
        {
            if (size < N)
            {
                _size = size;
                
                if (cstr != nullptr)
                {
                    _str_utils::copy(_data, cstr, _size);
                }

                _data[_size] = static_cast<CharT>(0);
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%s, %zu)"
                    ": size is too big\n", cstr, size
                );

                abort();
            } 
        }

        constexpr static_basic_string(basic_string_view<CharT> view)
            : static_basic_string(view.data(), view.size())
        {}

        constexpr static_basic_string(const static_basic_string& other)
        {
            _size = other._size;
            _str_utils::copy(_data, other._data, _size);
        }

        template <usize N2>
        constexpr static_basic_string(const static_basic_string<CharT, N2>& other)
        {
            if (other._size < N)
            {
                _size = other._size;
                _str_utils::copy(_data, other._data, _size);
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%s)"
                    ": other is too big to be contained\n", other._data
                );

                abort();
            }
        }

        template <typename CharT2, usize N2>
        constexpr static_basic_string(const static_basic_string<CharT2, N2>& other)
        {
            auto _len = unicode::length(other._data);

            if (_len < N2)
            {
                _size = _len;
                
                if constexpr (sizeof(CharT) == 1)
                {
                    unicode::to_utf8(_data, other._data, _size);
                }
                else if constexpr (sizeof(CharT) == 2)
                {
                    unicode::to_utf16(_data, other._data, _size);
                }
                else if constexpr (sizeof(CharT) == 4)
                {
                    unicode::to_utf32(_data, other._data, _size);
                }
                else
                {
                    fprintf(
                        stderr, "Error at static_basic_string(%s, %zu)"
                        ": sizeof(CharT) is not 1, 2 or 4\n", other._data, other._size
                    );

                    abort();
                }
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%s, %zu)"
                    ": cstr is too big to be contained\n", other._data, other._size
                );

                abort();
            }
        }

        constexpr static_basic_string(static_basic_string&& other)
        {
            swap(_size, other._size);
            move(other._data, other._data + _size, _data);
        }

        template <usize N2>
        constexpr static_basic_string(static_basic_string<CharT, N2>&& other)
        {
            if (other._size < N)
            {
                swap(_size, other._size);
                move(other._data, other._data + _size, _data);
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string(%s, %zu)"
                    ": other is too big to be contained\n", other._data
                );
            }
        }

        constexpr static_basic_string& operator=(const CharT* rhs)
        {
            _size = _str_utils::length(rhs);

            if (_size < N)
            {
                _str_utils::copy(_data, rhs, _size);
            }
            else
            {
                fprintf(
                    stderr, "Error at operator=(%s): rhs"
                    " is too big to be contained\n", rhs
                );

                abort();
            }

            return *this;
        }

        constexpr static_basic_string& operator=(const basic_string_view<CharT>& rhs)
        {
            if (rhs.size() < N)
            {
                _size = rhs.size();
                copy_n(rhs.data(), _size, _data);
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string::operator"
                    "=(%s): rhs is too big to be contained\n", rhs
                );

                abort();
            }

            return *this;
        }

        template <usize N2> requires (N2 <= N)
        constexpr static_basic_string& operator=(
            const static_basic_string<CharT, N2>& rhs)
        {
            _size = rhs._size;
            copy_n(rhs.c_str(), _size, _data);
            return *this;
        }

        template <typename CharT2, usize N2>
        constexpr static_basic_string& operator=(
            const static_basic_string<CharT2, N2>& rhs)
        {
            auto _len = unicode::length(rhs);

            if (_len < N2)
            {
                _size = _len;

                if constexpr (sizeof(CharT) == 1)
                {
                    unicode::to_utf8(_data, rhs, _size);
                }
                else if constexpr (sizeof(CharT) == 2)
                {
                    unicode::to_utf16(_data, rhs, _size);
                }
                else if constexpr (sizeof(CharT) == 4)
                {
                    unicode::to_utf32(_data, rhs, _size);
                }
                else
                {
                    fprintf(
                        stderr, "Error at static_basic_string::operator"
                        "=(%s): sizeof(CharT) is not 1, 2 or 4\n", rhs
                    );
                }
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string::operator"
                    "=(%s): rhs is too big to be contained\n", rhs
                );

                abort();
            }

            return *this;
        }

        template <usize N2> requires (N2 <= N)
        constexpr static_basic_string& operator=(static_basic_string<CharT, N2>&& rhs)
        {
            swap(_size, rhs._size);
            swap(_capacity, rhs._capacity);
            move(rhs._data, rhs._data + _size, _data);
            return *this;
        }

        constexpr CharT& operator[](usize index)
        {
            return _data[index];
        }

        constexpr const CharT& operator[](usize index) const
        {
            return _data[index];
        }

        template <usize N2>
        constexpr bool operator==(const static_basic_string<CharT, N2>& rhs) const
        {
            return _size == rhs._size && _str_utils::compare(_data, rhs._data) == 0;
        }

        template <usize N2>
        constexpr bool operator!=(const static_basic_string<CharT, N2>& rhs) const
        {
            return !operator==(rhs);
        }

        template <usize N2>
        constexpr bool operator<(const static_basic_string<CharT, N2>& rhs) const
        {
            return _str_utils::compare(_data, rhs._data) == -1;
        }

        template <usize N2>
        constexpr bool operator<=(const static_basic_string<CharT, N2>& rhs) const
        {
            auto _comp_rez = _str_utils::compare(_data, rhs._data);

            return _comp_rez == -1 || _comp_rez == 0;
        }

        template <usize N2>
        constexpr bool operator>(const static_basic_string<CharT, N2>& rhs) const
        {
            return _str_utils::compare(_data, rhs._data) == 1;
        }

        template <usize N2>
        constexpr bool operator>=(const static_basic_string<CharT, N2>& rhs) const
        {
            auto _comp_rez = _str_utils::compare(_data, rhs._data);

            return _comp_rez == 1 || _comp_rez == 0;
        }

        constexpr auto at(usize index)
            -> result<reference<CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        constexpr auto at(usize index) const
            -> result<reference<const CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        template <usize N2>
        constexpr usize find(
            const static_basic_string<CharT, N2>& str, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str._data
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        constexpr usize find(const CharT* str, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], str
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        constexpr usize find(CharT letter, usize pos = 0) const
        {
            if (pos >= _size)
            {
                return npos;
            }
            else
            {
                const CharT* _find_addr = _str_utils::find(
                    &_data[pos], letter
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        template <usize N2>
        constexpr usize rfind(
            const static_basic_string<CharT, N2>& str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size
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
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str._data, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        constexpr usize rfind(const CharT* str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        constexpr usize rfind(CharT str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
            else
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, _size - pos
                );

                if (_find_addr == nullptr)
                {
                    return npos;
                }
                else
                {
                    return static_cast<usize>(_find_addr - _data);
                }
            }
        }

        constexpr bool starts_with(CharT letter) const
        {
            if (_size != 0)
                return _data[0] == letter;

            return false;
        }

        constexpr bool starts_with(const CharT* str) const
        {
            if (_size != 0)
                return find(str) == 0;

            return false;
        }

        template <usize N2>
        constexpr bool starts_with(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return find(str) == 0;

            return false;
        }

        constexpr bool contains(CharT letter) const
        {
            if (_size != 0)
                return find(letter) != npos;

            return false;
        }

        constexpr bool contains(const CharT* str) const
        {
            if (_size != 0)
                return find(str) != npos;

            return false;
        }

        template <usize N2>
        constexpr bool contains(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return find(str) != npos;

            return false;
        }

        constexpr bool ends_with(CharT letter) const
        {
            if (_size != 0)
                return _data[_size - 1] == letter;

            return false;
        }

        constexpr bool ends_with(const CharT* str) const
        {
            usize _len = cstring::length(str);

            if (_size != 0)
                return rfind(str) == (_size - _len);

            return false;
        }

        template <usize N2>
        constexpr bool ends_with(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return rfind(str) == (_size - str._size);

            return false;
        }

        constexpr auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        constexpr auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            usize _current_pos = static_cast<usize>(from - begin());
            usize _last_pos = static_cast<usize>(to - begin());

            for (usize _index = 0; _index < _capacity - _last_pos + 1; _index++)
            {
                this->_data[_current_pos + _index] = 
                    move(this->_data[_last_pos + _index]);
            }

            _size -= static_cast<usize>(to - from) + 1;
            return begin() + _last_pos;
        }

        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if (_size < _capacity)
            {
                _data[_size] = CharT{forward<Args>(args)...};
                _data[++_size] = '\0';
            }
            else
            {
                fprintf(
                    stderr, "Error at static_basic_string"
                    ":: emplace_back: String is full\n"
                );

                abort();
            }
        }

        constexpr void push_back(const CharT& val)
        {
            emplace_back(val);
        }

        constexpr void push_back(CharT&& val)
        {
            emplace_back(move(val));
        }

        constexpr void clear()
        {
            if (_capacity != 0)
            {
                _data[0] = '\0';
                _size = 0;
            }
        }

        constexpr void resize(usize size)
        {
            if (size > _capacity)
            {
                fprintf(
                    stderr, "Error at static_basic_string::resize"
                    "(%zu): size is bigger than capacity\n", size
                );

                abort();
            }
            else if (size < _capacity)
            {
                _size = size;
            }
        }

        constexpr void pop_back()
        {
            if (_size != 1)
            {
                _data[--_size] = '\0';
            }
            else if (_data[0] != '\0')
            {
                _data[0] = '\0';
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
            return (_size != 0) ? _size + 1 : 0;
        }

        constexpr usize length() const
        {
            return _size;
        }

        constexpr usize capacity() const
        {
            return _capacity;
        }

        constexpr iterator data()
        {
            return _data;
        }

        constexpr const_iterator data() const
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

        constexpr const_iterator begin() const
        {
            return data();
        }

        constexpr iterator end()
        {
            return begin() + size();
        }

        constexpr const_iterator end() const
        {
            return begin() + size();
        }

        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }

        explicit constexpr operator basic_string_view<CharT>() const
        {
            return basic_string_view<CharT>(_data, _size);
        }

        constexpr auto pretty_args() const
        {
            return make_tuple(size(), data());
        }
    };

    template <typename T>
    static inline auto to_string(T val)
    {
        char* _cstr_buf = cstring::to_string(val);
        auto _str_buf = basic_string(_cstr_buf);
        mallocator::deallocate(_cstr_buf);
        return _str_buf;
    }

    static inline auto& to_string(basic_string<char>& val)
    {
        return val;
    }

    template <typename T>
    static inline auto to_wstring(T val)
    {
        wchar* _cstr_buf = wcstring::to_string(val);
        auto _str_buf = basic_string(_cstr_buf);
        mallocator::deallocate(_cstr_buf);
        return _str_buf;
    }

    static inline auto& to_wstring(basic_string<wchar>& val)
    {
        return val;
    }

    template <typename T>
    static inline auto to_u8string(T val)
    {
        char8* _cstr_buf = u8cstring::to_string(val);
        auto _str_buf = basic_string(_cstr_buf);
        mallocator::deallocate(_cstr_buf);
        return _str_buf;
    }

    static inline auto& to_u8string(basic_string<char8>& val)
    {
        return val;
    }

    template <typename T>
    static inline auto to_u16string(T val)
    {
        char16* _cstr_buf = u16cstring::to_string(val);
        auto _str_buf = basic_string(_cstr_buf);
        mallocator::deallocate(_cstr_buf);
        return _str_buf;
    }

    static inline auto& to_u16string(basic_string<char16>& val)
    {
        return val;
    }

    template <typename T>
    static inline auto to_u32string(T val)
    {
        char32* _cstr_buf = u32cstring::to_string(val);
        auto _str_buf = basic_string(_cstr_buf);
        mallocator::deallocate(_cstr_buf);
        return _str_buf;
    }

    static inline auto& to_u32string(basic_string<char32>& val)
    {
        return val;
    }
    
    namespace string_literals
    {
        inline basic_string<char> operator""_s(const char* str, usize size)
        {
            return basic_string<char>{str, size};
        }

        inline basic_string<char8> operator""_s(const char8* str, usize size)
        {
            return basic_string<char8>{str, size};
        }

        inline basic_string<char16> operator""_s(const char16* str, usize size)
        {
            return basic_string<char16>{str, size};
        }

        inline basic_string<char32> operator""_s(const char32* str, usize size)
        {
            return basic_string<char32>{str, size};
        }

        inline basic_string<wchar> operator""_s(const wchar* str, usize size)
        {
            return basic_string<wchar>{str, size};
        }
    } // namespace string_literals

    template <typename HashType, typename CharT>
    struct hash<HashType, basic_string<CharT>>
    {
        using ResultType = HashType;

        static constexpr ResultType get_hash(const basic_string<CharT>& str) {
            return hash<HashType, const CharT*>::get_hash(str.begin(), str.end() - 1);
        }
    };

    template <typename CharT>
    struct count_sorter<basic_string<CharT>>
    {
        static constexpr void sort(
            basic_string<CharT>& arr, usize exp)
        {
            basic_string<CharT> _output = {};
            usize _index, _count[10] = {};
            constexpr CharT val = {};

            for (_index = 0; _index < arr.length(); _index++)
                _output.push_back(val);

            for (_index = 0; _index < arr.length(); _index++)
            {
                _count[(arr[_index] / exp) % 10]++;
            }

            for (_index = 1; _index < 10; _index++)
            {
                _count[_index] += _count[_index - 1];
            }

            for (_index = arr.length() - 1; _index > 0; _index--)
            {
                _output[_count[(arr[_index] / exp) % 10] - 1] = arr[_index];
                _count[(arr[_index] / exp) % 10]--;
            }

            _output[_count[(arr[0] / exp) % 10] - 1] = arr[0];
            _count[(arr[0] / exp) % 10]--;

            for (_index = 0; _index < arr.length(); _index++)
            {
                swap(arr[_index], _output[_index]);
            }
        }

        static constexpr void sort(
            basic_string<CharT>& arr, usize exp, auto&& get_digit)
        {
            basic_string<CharT> _output = {};
            usize _index, _count[10] = {};
            constexpr CharT val = {};

            for (_index = 0; _index < arr.length(); _index++)
                _output.push_back(val);

            for (_index = 0; _index < arr.length(); _index++)
            {
                _count[(get_digit(arr[_index]) / exp) % 10]++;
            }

            for (_index = 1; _index < 10; _index++)
            {
                _count[_index] += _count[_index - 1];
            }

            for (_index = arr.length() - 1; _index > 0; _index--)
            {
                _output[_count[(get_digit(arr[_index]) / exp) % 10] - 1] = get_digit(arr[_index]);
                _count[(get_digit(arr[_index]) / exp) % 10]--;
            }

            _output[_count[(get_digit(arr[0]) / exp) % 10] - 1] = get_digit(arr[0]);
            _count[(get_digit(arr[0]) / exp) % 10]--;

            for (_index = 0; _index < arr.length(); _index++)
            {
                swap(arr[_index], _output[_index]);
            }
        }
    };

    template <typename HashType, typename CharT, usize N>
    struct hash<HashType, static_basic_string<CharT, N>>
    {
        using ResultType = HashType;

        static constexpr ResultType get_hash(const static_basic_string<CharT, N>& str) {
            return hash<HashType, const CharT*>::get_hash(str.begin(), str.end() - 1);
        }
    };

    using string = basic_string<char>;
    using wstring = basic_string<wchar>;
    using u8string = basic_string<char8>;
    using u16string = basic_string<char16>;
    using u32string = basic_string<char32>;
    
    template <usize N>
    using static_string = static_basic_string<char, N>;
    
    template <usize N>
    using static_wstring = static_basic_string<wchar, N>;
    
    template <usize N>
    using static_u8string = static_basic_string<char8, N>;
    
    template <usize N>
    using static_u16string = static_basic_string<char16, N>;
    
    template <usize N>
    using static_u32string = static_basic_string<char32, N>;
} // namespace hsd
