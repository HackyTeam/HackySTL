#pragma once

#include "CString.hpp"
#include "Hash.hpp"
#include "_IoDetail.hpp"

namespace hsd
{
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

    template <typename CharT>
    class basic_string_view
    {
    public:
        using value_type = CharT;
        using const_reference = const CharT&;
        using const_pointer = const CharT*;
        using const_iterator = const_pointer;

    private:
        using cstring_utils = basic_cstring<CharT>;
        const_pointer _data = nullptr;
        usize _size = 0;

    public:
        static constexpr usize npos = ~static_cast<usize>(0u);

        constexpr basic_string_view(const_pointer cstr)
            : _data{cstr}, _size{cstring_utils::length(cstr)}
        {}

        constexpr basic_string_view(const_pointer str, usize size)
            : _data{str}, _size{size}
        {}

        template <usize N>
        constexpr basic_string_view(const CharT (&str)[N])
            : _data{str}, _size{N - 1}
        {}

        constexpr void reset()
        {
            _data = nullptr;
            _size = 0;
        }

        // No negative index supported
        constexpr const_reference operator[](usize index) const
        {
            return _data[index];
        }

        constexpr auto at(usize index) const
            -> Result< reference<const CharT>, bad_access >
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        constexpr bool operator==(basic_string_view rhs) const {
            return _size == rhs._size && cstring_utils::compare(_data, rhs._data, _size) == 0;
        }

        constexpr bool operator!=(basic_string_view rhs) const {
            return !(*this == rhs);
        }

        constexpr bool operator<(const basic_string_view& rhs) const
        {
            return cstring_utils::compare(_data, rhs._data) == -1;
        }

        constexpr bool operator<=(const basic_string_view& rhs) const
        {
            auto _comp_rez = cstring_utils::compare(_data, rhs._data);

            return _comp_rez == -1 || _comp_rez == 0;
        }

        constexpr bool operator>(const basic_string_view& rhs) const
        {
            return cstring_utils::compare(_data, rhs._data) == 1;
        }

        constexpr bool operator>=(const basic_string_view& rhs) const
        {
            auto _comp_rez = cstring_utils::compare(_data, rhs._data);

            return _comp_rez == 1 || _comp_rez == 0;
        }
        
        constexpr usize find(char letter) const
        {
            auto* _find_res = cstring_utils::find(_data, letter);

            if (_find_res == nullptr)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_find_res - _data);
            }
        }

        constexpr usize find(const basic_string_view& other) const
        {
            auto* _find_res = cstring_utils::find(_data, other._data);

            if (_find_res == nullptr)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_find_res - _data);
            }
        }

        constexpr usize rfind(char letter) const
        {
            auto* _find_res = cstring_utils::rfind(_data, letter);

            if (_find_res == nullptr)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_find_res - _data);
            }
        }

        constexpr usize rfind(const basic_string_view& other) const
        {
            auto* _find_res = cstring_utils::rfind(_data, other._data);

            if (_find_res == nullptr)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_find_res - _data);
            }
        }

        constexpr bool starts_with(CharT letter) const
        {
            if (_data != nullptr)
                return _data[0] == letter;

            return false;
        }

        constexpr bool starts_with(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        constexpr bool starts_with(const basic_string_view& str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        constexpr bool contains(CharT letter) const
        {
            if (_data != nullptr)
                return find(letter) != npos;

            return false;
        }

        constexpr bool contains(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        constexpr bool contains(const basic_string_view& str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        constexpr bool ends_with(CharT letter) const
        {
            if (_data != nullptr)
                return _data[size() - 1] == letter;

            return false;
        }

        constexpr bool ends_with(const CharT* str) const
        {
            usize _len = cstring::length(str);

            if (_data != nullptr)
                return rfind(str) == size() - _len;

            return false;
        }

        constexpr bool ends_with(const basic_string_view& str) const
        {
            if (_data != nullptr)
                return rfind(str) == size() - str.size();

            return false;
        }

        constexpr usize size() const
        {
            return _size;
        }

        constexpr usize capacity() const
        {
            return _size;
        }

        constexpr const_pointer data() const
        {
            return _data;
        }

        constexpr const_iterator begin() const
        {
            return _data;
        }

        constexpr const_iterator end() const
        {
            return _data + _size;
        }

        constexpr const_iterator rbegin() const
        {
            return _data + _size - 1;
        }

        constexpr const_iterator rend() const
        {
            return _data - 1;
        }

        constexpr auto sub_string(usize start, usize len) const
            -> Result<basic_string_view, out_of_range>
        {
            if (start >= _size)
                return out_of_range{};

            const usize max_length = _size - start;
            return basic_string_view(_data + start, len < max_length ? len : max_length);
        }

        template <format_literal fmt>
        friend void _print_impl(io_detail::printer<fmt>& p, const basic_string_view& str)
        {
            using fmt_char_type = typename decltype(fmt)::char_type;
            constexpr auto _fmt = format_literal<fmt_char_type, 1> {
                .format = {{'\0'}},
                .tag = fmt.tag,
                .foreground = fmt.foreground,
                .background = fmt.background
            };

            io_detail::printer<_fmt> _p = {p.file()};
            _print_impl(p);

            for (auto _ch : str)
            {
                _print_impl(_p, _ch);
            }
        }

        template <format_literal fmt>
        friend auto _write_impl(sstream_detail::stream_writer<fmt>& p, const basic_string_view& str)
        {
            using fmt_char_type = typename decltype(fmt)::char_type;
            constexpr auto _fmt = format_literal<fmt_char_type, 1> {
                .format = "",
                .tag = fmt.tag,
                .foreground = fmt.foreground,
                .background = fmt.background
            };

            sstream_detail::stream_writer<_fmt> _p = {p.data(), p.size(), p.index()};
            _write_impl(p);

            for (auto _ch : str)
            {
                _write_impl(_p, _ch);
            }
        }
    };

    namespace string_view_literals
    {
        constexpr basic_string_view<char> operator""_sv(const char* str, usize size)
        {
            return basic_string_view<char>{str, size};
        }

        constexpr basic_string_view<char8> operator""_sv(const char8* str, usize size)
        {
            return basic_string_view<char8>{str, size};
        }

        constexpr basic_string_view<char16> operator""_sv(const char16* str, usize size)
        {
            return basic_string_view<char16>{str, size};
        }

        constexpr basic_string_view<char32> operator""_sv(const char32* str, usize size)
        {
            return basic_string_view<char32>{str, size};
        }

        constexpr basic_string_view<wchar> operator""_sv(const wchar* str, usize size)
        {
            return basic_string_view<wchar>{str, size};
        }
    } // namespace string_view_literals

    template <typename HashType, typename CharT>
    struct hash<HashType, basic_string_view<CharT>>
    {
        using ResultType = HashType;

        static constexpr ResultType get_hash(basic_string_view<CharT> view) {
            return hash<HashType, const CharT*>::get_hash(view.begin(), view.end());
        }
    };

    using string_view = basic_string_view<char>;
    using wstring_view = basic_string_view<wchar>;
    using u8string_view = basic_string_view<char8>;
    using u16string_view = basic_string_view<char16>;
    using u32string_view = basic_string_view<char32>;
} // namespace hsd
