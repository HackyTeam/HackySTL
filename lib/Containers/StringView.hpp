#pragma once

/// @file StringView.hpp
/// @brief Non-owning views over character sequences.
/// @details Provides length-aware search, comparison, slicing, prefix/suffix
/// checks, Unicode character aliases, and compile-time string-view literals.

#include "../Base/CString.hpp"
#include "../Base/Hash.hpp"
#include "../Serialize/_PrinterImpl.hpp"
#include "../Base/Types.hpp"
#include "../Base/ReverseIterator.hpp"

namespace hsd
{
    /// @brief Error returned for an out-of-range string-view access.
    struct bad_access
    {
        /// @brief Returns the diagnostic used for an invalid element access.
        /// @return A static diagnostic string.
        const char* operator()() const
        {
            return "Tried to access an element out of bounds";
        }
    };
    
    /// @brief Error returned for an invalid string-view substring request.
    struct out_of_range
    {
        /// @brief Returns the diagnostic used for an invalid subview request.
        /// @return A static diagnostic string.
        const char* operator()() const
        {
            return "Out of range";
        }
    };

    /// @brief Non-owning length-aware character sequence view.
    template <typename CharT>
    class basic_string_view
    {
    public:
        /// @brief Character type referenced by the view.
        using value_type = CharT;
        /// @brief Const reference to one character.
        using const_reference = const CharT&;
        /// @brief Pointer type returned by data.
        using const_pointer = const CharT*;
        /// @brief Forward iterator over viewed characters.
        using const_iterator = const_pointer;

    private:
        /// @brief Character algorithms specialized for `CharT`.
        using cstring_utils = basic_cstring<CharT>;
        /// @brief Non-owning start address.
        const_pointer _data = nullptr;
        /// @brief Logical character count, independent of a terminator.
        usize _length = 0;

    public:
        /// @brief Sentinel returned when a search does not find a match.
        static constexpr usize npos = ~static_cast<usize>(0u);
        /// @brief Constructs an empty, non-owning view.
        constexpr basic_string_view() = default;

        /// @brief Views a null-terminated character sequence without copying.
        /// @param cstr Null-terminated sequence whose length is measured.
        constexpr basic_string_view(const_pointer cstr)
            : _data{cstr}, _length{cstring_utils::length(cstr)}
        {}

        /// @brief Views exactly `length` characters at `str`.
        /// @param str Start of the viewed sequence.
        /// @param length Number of characters viewed from `str`.
        constexpr basic_string_view(const_pointer str, usize length)
            : _data{str}, _length{length}
        {}

        /// @brief Views a fixed character array without copying.
        /// @tparam N Array extent, including a source terminator when present.
        /// @param str Character array whose first `N - 1` elements are viewed.
        template <usize N>
        constexpr basic_string_view(const CharT (&str)[N])
            : _data{str}, _length{N - 1}
        {}

        /// @brief Clears the pointer and logical length.
        constexpr void reset()
        {
            _data = nullptr;
            _length = 0;
        }

        // No negative index supported
        /// @brief Returns unchecked access to one viewed character.
        /// @param index Zero-based character index.
        /// @return Reference to the character at `index`; callers must ensure
        ///         `index < length()`.
        constexpr const_reference operator[](usize index) const
        {
            return _data[index];
        }

        /// @brief Returns checked access to one viewed character.
        /// @param index Zero-based character index.
        /// @return A reference result, or `bad_access` when `index` is outside
        ///         the logical view.
        constexpr auto at(usize index) const
            -> result<reference<const CharT>, bad_access>
        {
            if(index >= _length)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Lexicographically compares two views.
        /// @details An embedded null is an ordinary character, so only the first
        /// differing character decides the result. When one view is a prefix of
        /// the other, the shorter view sorts first, as in `std::string_view`.
        /// @param rhs View compared with this view.
        /// @return Negative, zero, or positive according to the comparison.
        constexpr i32 operator<=>(const basic_string_view& rhs) const
        {
            auto _cmp_length = (rhs._length > _length) ? _length : rhs._length;
            i32 _order = _compare(_data, rhs._data, _cmp_length);

            if (_order != 0)
            {
                return _order;
            }

            if (_length < rhs._length)
            {
                return -1;
            }

            if (_length > rhs._length)
            {
                return 1;
            }

            return 0;
        }

        /// @brief Tests equal logical length and equal characters.
        /// @param rhs View compared with this view.
        /// @return True when both views contain the same sequence.
        constexpr bool operator==(const basic_string_view& rhs) const
        {
            return _length == rhs._length && _equal_at(_data, rhs._data, _length);
        }

        /// @brief Finds the first character shared with another view.
        /// @param other Character set represented by another view.
        /// @return Index of the first shared character, or `npos`.
        constexpr usize find_first_of(const basic_string_view& other) const
        {
            for (const auto* _str = _data; _str != end(); _str++)
            {
                for (const auto* _other_str = other._data; _other_str != other.end(); ++_other_str)
                {
                    if (*_str == *_other_str)
                    {
                        return static_cast<usize>(_str - _data);
                    }
                }
            }

            return npos;
        }

        /// @brief Finds the first character shared with a null-terminated set.
        /// @param other Null-terminated character set.
        /// @return Index of the first shared character, or `npos`.
        constexpr usize find_first_of(const CharT* other) const
        {
            for (const auto* _str = _data; _str != end(); _str++)
            {
                for (const auto* _other_str = other; *_other_str != '\0'; ++_other_str)
                {
                    if (*_str == *_other_str)
                    {
                        return static_cast<usize>(_str - _data);
                    }
                }
            }

            return npos;
        }
        
        /// @brief Scans `[first, last)` for the first element equal to `letter`.
        /// @param first First element searched.
        /// @param last One past the final element searched.
        /// @param letter Element searched for.
        /// @return Pointer to the first match, or `last` when there is none.
        static constexpr const CharT* _scan(
            const CharT* first, const CharT* last, CharT letter)
        {
            return cstring_utils::find_or_end(first, last, letter);
        }

        /// @brief Orders the first `len` elements at `lhs` and `rhs`.
        /// @details Unlike `cstring_utils::compare`, the scan does not stop at a
        /// null element, so `"a\0b"` and `"a\0c"` differ at index 2. Both ranges
        /// must be readable for `len` elements.
        /// @param lhs First range.
        /// @param rhs Second range.
        /// @param len Number of elements to compare.
        /// @return -1 when `lhs` sorts first, 1 when `rhs` sorts first, 0 when
        /// the ranges are equal.
        static constexpr i32 _compare(const CharT* lhs, const CharT* rhs, usize len)
        {
            if (_equal_at(lhs, rhs, len))
            {
                return 0;
            }

            for (usize _index = 0; _index < len; _index++)
            {
                if (lhs[_index] != rhs[_index])
                {
                    return cstring_utils::order(lhs[_index], rhs[_index]);
                }
            }

            return 0;
        }

        /// @brief Scans `[first, last)` for a `char` the way `find(char)` always has.
        /// @details Only a view of `char` takes the fast path. Other character
        /// types keep the original element-by-element comparison against the
        /// `char`, where integer promotion decides equality.
        /// @param first First element searched.
        /// @param last One past the final element searched.
        /// @param letter Character searched for.
        /// @return Pointer to the first match, or `last` when there is none.
        static constexpr const CharT* _scan_letter(
            const CharT* first, const CharT* last, char letter)
        {
            if constexpr (is_same<CharT, char>::value)
            {
                return _scan(first, last, letter);
            }
            else
            {
                for (; first != last; ++first)
                {
                    if (*first == letter)
                    {
                        break;
                    }
                }

                return first;
            }
        }

        /// @brief Finds the first occurrence of a character.
        /// @param letter Character to find.
        /// @return Index of the first occurrence, or `npos`.
        constexpr usize find(char letter) const
        {
            const auto* _end = _data + _length;
            const auto* _str = _scan_letter(_data, _end, letter);

            if (_str == _end)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_str - _data);
            }
        }

        /// @brief Finds a character at or after `pos`.
        /// @param letter Character to find.
        /// @param pos First position searched.
        /// @return Index of the first occurrence, or `npos`.
        constexpr usize find(char letter, usize pos) const
        {
            if (pos >= length())
            {
                return npos;
            }

            const auto* _end = _data + _length;
            const auto* _str = _scan_letter(_data + pos, _end, letter);

            if (_str == _end)
            {
                return npos;
            }
            else
            {
                return static_cast<usize>(_str - _data);
            }
        }

        /// @brief Finds the first occurrence of another view.
        /// @param other Sequence to find.
        /// @return Starting index of the first occurrence, or `npos`.
        constexpr usize find(const basic_string_view& other) const
        {
            return find(other, 0);
        }

        /// @brief Finds another view at or after `pos`.
        /// @param other Sequence to find.
        /// @param pos First position searched.
        /// @return Starting index of the first occurrence, or `npos`.
        constexpr usize find(const basic_string_view& other, usize pos) const
        {
            if (pos > _length)
            {
                return npos;
            }

            if (other._length == 0)
            {
                return pos;
            }

            if (other._length > _length - pos)
            {
                return npos;
            }

            // Candidate starts are the positions holding the needle's first
            // character; each is verified with the same `compare` as before.
            const CharT _first = other._data[0];
            const CharT* _candidate = _data + pos;
            const CharT* _stop = _data + (_length - other._length) + 1;

            while (_candidate != _stop)
            {
                _candidate = _scan(_candidate, _stop, _first);

                if (_candidate == _stop)
                {
                    break;
                }

                if (_equal_at(_candidate, other._data, other._length))
                {
                    return static_cast<usize>(_candidate - _data);
                }

                ++_candidate;
            }

            return npos;
        }

        /// @brief Finds the last occurrence of a character.
        /// @param letter Character to find.
        /// @return Index of the last occurrence, or `npos`.
        constexpr usize rfind(char letter) const
        {
            // Index-based, so the scan never forms a pointer before `_data`.
            for (usize _index = _length; _index != 0; _index--)
            {
                if (_data[_index - 1] == letter)
                {
                    return _index - 1;
                }
            }

            return npos;
        }

        /// @brief Finds a character at or before the bounded reverse position.
        /// @param letter Character to find.
        /// @param pos Reverse-search bound.
        /// @return Index of the last occurrence in the searched region, or `npos`.
        constexpr usize rfind(char letter, usize pos) const
        {
            if (_length == 0)
            {
                return npos;
            }

            usize _index = pos < _length ? pos : _length - 1;

            while (_data[_index] != letter)
            {
                if (_index == 0)
                {
                    return npos;
                }

                _index--;
            }

            return _index;
        }

        /// @brief Tests whether `len` elements at `first` equal `needle` exactly.
        /// @details Compares elements, so an embedded null is an ordinary
        /// character. At run time on GCC and Clang, equal bytes are confirmed with
        /// one `__builtin_memcmp` call. Both ranges must be readable for `len`
        /// elements.
        /// @param first Start of the candidate range.
        /// @param needle Start of the needle.
        /// @param len Number of elements to compare.
        /// @return Whether every element matches.
        static constexpr bool _equal_at(const CharT* first, const CharT* needle, usize len)
        {
            #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
            if (!is_constant_evaluated() && len != 0)
            {
                return __builtin_memcmp(first, needle, len * sizeof(CharT)) == 0;
            }
            #endif

            for (usize _offset = 0; _offset < len; _offset++)
            {
                if (first[_offset] != needle[_offset])
                {
                    return false;
                }
            }

            return true;
        }

        /// @brief Finds the last occurrence of `other` starting at or before `last_start`.
        /// @details Tries the candidate starts from `last_start` down to zero.
        /// Every candidate lies inside this view, because the caller keeps
        /// `last_start + other.length() <= length()`.
        /// @param other Non-empty sequence to find.
        /// @param last_start Largest starting index tried.
        /// @return Starting index of the last occurrence, or `npos`.
        constexpr usize _rfind_from(const basic_string_view& other, usize last_start) const
        {
            usize _index = last_start;

            while (!_equal_at(_data + _index, other._data, other._length))
            {
                if (_index == 0)
                {
                    return npos;
                }

                _index--;
            }

            return _index;
        }

        /// @brief Finds the last occurrence of another view.
        /// @param other Sequence to find.
        /// @return Starting index of the last occurrence, or `npos`.
        constexpr usize rfind(const basic_string_view& other) const
        {
            // An empty needle matches at the end, as `std::string_view` does.
            if (other._length == 0)
            {
                return _length;
            }

            if (other._length > _length)
            {
                return npos;
            }

            return _rfind_from(other, _length - other._length);
        }

        /// @brief Finds another view at or before the bounded reverse position.
        /// @param other Sequence to find.
        /// @param pos Reverse-search bound.
        /// @return Starting index of the last occurrence, or `npos`.
        constexpr usize rfind(const basic_string_view& other, usize pos) const
        {
            if (other._length > _length)
            {
                return npos;
            }

            // A match starts no later than `pos`, and no later than the last start that fits.
            const usize _last_start = _length - other._length;
            return _rfind_from(other, pos < _last_start ? pos : _last_start);
        }

        /// @brief Tests whether the view begins with a character.
        /// @param letter Character tested at the beginning.
        /// @return Whether the view starts with `letter`.
        constexpr bool starts_with(CharT letter) const
        {
            if (_length != 0)
                return _data[0] == letter;

            return false;
        }

        /// @brief Tests whether the view begins with a null-terminated sequence.
        /// @param str Sequence tested at the beginning.
        /// @return Whether the view starts with `str`.
        constexpr bool starts_with(const CharT* str) const
        {
            return find(str) == 0;
        }

        /// @brief Tests whether the view begins with another view.
        /// @details An empty `str` matches any view, as in `std::string_view`.
        /// @param str Sequence tested at the beginning.
        /// @return Whether the view starts with `str`.
        constexpr bool starts_with(const basic_string_view& str) const
        {
            return find(str) == 0;
        }

        /// @brief Tests whether a character occurs in the view.
        /// @param letter Character searched for.
        /// @return Whether `letter` occurs in the view.
        constexpr bool contains(CharT letter) const
        {
            return find(letter) != npos;
        }

        /// @brief Tests whether a null-terminated sequence occurs in the view.
        /// @param str Sequence searched for.
        /// @return Whether `str` occurs in the view.
        constexpr bool contains(const CharT* str) const
        {
            return find(str) != npos;
        }

        /// @brief Tests whether another view occurs in this view.
        /// @details An empty `str` is contained in any view, as in `std::string_view`.
        /// @param str Sequence searched for.
        /// @return Whether `str` occurs in the view.
        constexpr bool contains(const basic_string_view& str) const
        {
            return find(str) != npos;
        }

        /// @brief Tests whether the view ends with a character.
        /// @param letter Character tested at the end.
        /// @return Whether the view ends with `letter`.
        constexpr bool ends_with(CharT letter) const
        {
            if (_length != 0)
                return _data[_length - 1] == letter;

            return false;
        }

        /// @brief Tests whether the view ends with a null-terminated sequence.
        /// @param str Sequence tested at the end.
        /// @return Whether the view ends with `str`.
        constexpr bool ends_with(const CharT* str) const
        {
            return ends_with(basic_string_view{str});
        }

        /// @brief Tests whether the view ends with another view.
        /// @details A needle longer than this view cannot match, so it is rejected
        /// before the offset is computed.
        /// @param str Sequence tested at the end.
        /// @return Whether the view ends with `str`.
        constexpr bool ends_with(const basic_string_view& str) const
        {
            if (str._length > _length)
                return false;

            return _equal_at(_data + (_length - str._length), str._data, str._length);
        }

        /// @brief Returns the number of viewed characters.
        /// @return Logical view length.
        /// @brief Returns the number of characters in the view; same as `length()`.
        constexpr usize size() const
        {
            return length();
        }

        constexpr usize length() const
        {
            return _length;
        }

        /// @brief Returns the non-owning character pointer.
        /// @return Pointer to the first viewed character, or null for an empty reset view.
        constexpr const_pointer data() const
        {
            return _data;
        }

        /// @brief Returns an iterator to the first viewed character.
        /// @return `data()`.
        constexpr const_iterator begin() const
        {
            return _data;
        }

        /// @brief Returns an iterator one past the viewed characters.
        /// @return `data() + length()`.
        constexpr const_iterator end() const
        {
            return _data + length();
        }

        /// @brief Returns a reverse-start pointer at the final viewed character.
        /// @return Pointer to the final viewed character.
        constexpr contiguous_reverse_iterator<const CharT*> rbegin() const
        {
            return contiguous_reverse_iterator<const CharT*>{_data + length()};
        }

        /// @brief Returns the reverse end pointer before the view.
        /// @return Pointer one position before `data()`.
        constexpr contiguous_reverse_iterator<const CharT*> rend() const
        {
            return contiguous_reverse_iterator<const CharT*>{_data};
        }

        /// @brief Returns a bounded subview or an out-of-range error.
        /// @param start Starting index in this view.
        /// @param len Maximum number of characters in the subview.
        /// @return A view clipped to this view's end, or `out_of_range` when
        ///         `start` is outside the logical view.
        constexpr auto sub_string(usize start, usize len) const
            -> result<basic_string_view, out_of_range>
        {
            if (start >= _length)
                return out_of_range{};

            const usize max_length = _length - start;
            return basic_string_view(_data + start, len < max_length ? len : max_length);
        }
    };

    /// @brief Prints a character view into a formatted character buffer.
    /// @param str Destination buffer.
    /// @param sz Destination capacity in characters.
    /// @param val View converted into the destination character type.
    /// @return Number of written characters, or a runtime error for insufficient
    ///         capacity or conversion failure.
    /// @details Only accepts a format with no presentation tag, since text has no numeric form.
    template <format_output fmt, typename CharT, CharacterType T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const basic_string_view<T>& val)
    {
        // The style escape before the value and the reset escape after it need room,
        // and the terminator after the value needs one position more than its characters.
        const usize _prefix = fmt.format.size() + fmt.style.size() - 2;
        const usize _suffix = fmt.reset.size() - 1;

        if (_prefix + _suffix + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);

        // Convert into the room left after the prefix, keeping room for the suffix.
        const usize _room = sz - _prefix - _suffix;
        auto _res = unicode::convert_to<CharT, T>(str, _room, val.data(), val.length());

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        usize _pos = _room - _res.unwrap() - 1;
        str = _print_to_impl(str + _pos, fmt.reset);

        return _prefix + _pos + _suffix;
    }

    namespace string_view_literals
    {
        /// @brief Creates a narrow string view from a literal token.
        /// @param str Literal character pointer.
        /// @param length Literal length excluding a terminator.
        /// @return A non-owning narrow string view.
        constexpr auto operator""_sv(const char* str, usize length)
        {
            return basic_string_view<char>{str, length};
        }

        /// @brief Creates a UTF-8 string view from a literal token.
        /// @param str Literal character pointer.
        /// @param length Literal length excluding a terminator.
        /// @return A non-owning UTF-8 string view.
        constexpr auto operator""_sv(const char8* str, usize length)
        {
            return basic_string_view<char8>{str, length};
        }

        /// @brief Creates a UTF-16 string view from a literal token.
        /// @param str Literal character pointer.
        /// @param length Literal length excluding a terminator.
        /// @return A non-owning UTF-16 string view.
        constexpr auto operator""_sv(const char16* str, usize length)
        {
            return basic_string_view<char16>{str, length};
        }

        /// @brief Creates a UTF-32 string view from a literal token.
        /// @param str Literal character pointer.
        /// @param length Literal length excluding a terminator.
        /// @return A non-owning UTF-32 string view.
        constexpr auto operator""_sv(const char32* str, usize length)
        {
            return basic_string_view<char32>{str, length};
        }

        /// @brief Creates a wide string view from a literal token.
        /// @param str Literal character pointer.
        /// @param length Literal length excluding a terminator.
        /// @return A non-owning wide string view.
        constexpr auto operator""_sv(const wchar* str, usize length)
        {
            return basic_string_view<wchar>{str, length};
        }
    } // namespace string_view_literals

    /// @brief Hash specialization for non-owning character ranges.
    template <typename HashType, typename CharT>
    struct hash<HashType, basic_string_view<CharT>>
    {
        /// @brief Hash result type selected by the hash customization point.
        using ResultType = HashType;

        /// @brief Hashes the viewed character range without requiring a terminator.
        /// @param view View whose character range is hashed.
        /// @return Hash value for the viewed range.
        static constexpr ResultType get_hash(basic_string_view<CharT> view) {
            return hash<HashType, const CharT*>::get_hash(view.begin(), view.end());
        }
    };

    /// @brief Narrow character string-view alias.
    using string_view = basic_string_view<char>;
    /// @brief Wide character string-view alias.
    using wstring_view = basic_string_view<wchar>;
    /// @brief UTF-8 string-view alias.
    using u8string_view = basic_string_view<char8>;
    /// @brief UTF-16 string-view alias.
    using u16string_view = basic_string_view<char16>;
    /// @brief UTF-32 string-view alias.
    using u32string_view = basic_string_view<char32>;
} // namespace hsd
