#pragma once

#include "Types.hpp"
#include "CString.hpp"
#include "Result.hpp"
#include "Hash.hpp"

#include <ostream>

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
            : _data(cstr), _size(cstring_utils::length(cstr))
        {}

        constexpr basic_string_view(const_pointer str, usize size)
            : _data(str), _size(size)
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

        // TODO: at

        constexpr bool operator==(basic_string_view rhs) const {
            return _size == rhs._size and cstring_utils::compare(
                _data, rhs._data, _size
            ) == 0;
        }

        constexpr bool operator!=(basic_string_view rhs) const {
            return !(*this == rhs);
        }

        // TODO: < <= > >=
        // TODO: find
        // TODO: rfind

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

        // TODO: rbegin, rend (requires hsd::reverse_iterator)

        constexpr Result<basic_string_view, out_of_range>
        substring(usize start, usize len) const
        {
            if (start >= _size)
                return out_of_range{};

            const usize max_length = _size - start;
            return basic_string_view(_data + start, math::min(len, max_length));
        }

        // I/O
        friend std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, basic_string_view v) {
            for (CharT ch : v)
                os.put(ch);
            return os;
        }
    };

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
