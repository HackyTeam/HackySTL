#pragma once

/// @file String.hpp
/// @brief Allocator-aware null-terminated basic strings.
/// @details Covers ownership, mutation, search, substrings, Unicode conversion,
/// formatting, and explicit storage-size/terminator invariants.

#include "../Base/Result.hpp"
#include "../Base/CString.hpp"
#include "StringView.hpp"
#include "../Serialize/ToString.hpp"
#include "../Base/Unicode.hpp"
#include "../Base/ReverseIterator.hpp"

namespace hsd
{
    template < typename CharT, typename Allocator = allocator >
    /// @brief Owning null-terminated string with allocator-backed storage.
    class basic_string
    {
    private:
        /// @brief C-string algorithms (length/copy/compare/find) for `CharT`.
        using _str_utils = basic_cstring<CharT>;
        /// @brief Null character returned by `c_str()` when storage is unallocated.
        static constexpr const CharT _s_empty = 0;
        /// @brief Allocator type used to obtain and release character storage.
        using alloc_type = Allocator;
        /// @brief Allocator operations used to allocate/deallocate character storage.
        using alloc_traits = allocator_traits<alloc_type, CharT>;
        
        /// @brief Allocator instance owning the storage referenced by `_data`.
        alloc_type _alloc;
        /// @brief Owning pointer to null-terminated storage, or `nullptr` when unallocated.
        CharT* _data = nullptr;
        /// @brief Logical length plus one for the null terminator, or 0 when unallocated.
        usize _size = 0;
        /// @brief Number of characters currently allocated at `_data`, including the terminator slot.
        usize _capacity = 0;

        /// @brief Deallocates owned storage and clears `_data`, leaving size/capacity untouched.
        constexpr void _reset()
        {
            alloc_traits::deallocate(_alloc, _data, _capacity).unwrap();
            _data = nullptr;
        }

        /// @brief Grants storage access to differently-typed/allocated `basic_string` instantiations.
        template <typename CharT2, typename Allocator2>
        friend class basic_string;

        /// @brief Builds a new string holding `lhs_len` characters of `lhs` followed by `rhs_len` characters of `rhs`.
        /// @param lhs First part; may be null when `lhs_len` is zero.
        /// @param lhs_len Number of characters taken from `lhs`.
        /// @param rhs Second part; may be null when `rhs_len` is zero.
        /// @param rhs_len Number of characters taken from `rhs`.
        /// @return New string holding both parts.
        constexpr static basic_string _join(
            const CharT* lhs, usize lhs_len, const CharT* rhs, usize rhs_len)
        {
            basic_string _buf(lhs_len + rhs_len + 1);
            _buf._size = lhs_len + rhs_len + 1;
            if (lhs_len != 0)
            {
                copy_n(lhs, lhs_len, _buf._data);
            }
            if (rhs_len != 0)
            {
                copy_n(rhs, rhs_len, _buf._data + lhs_len);
            }
            _buf._data[lhs_len + rhs_len] = static_cast<CharT>(0);
            return _buf;
        }

        /// @brief Appends `rhs_len` characters, growing storage when the current capacity is insufficient.
        /// @param rhs Characters to append; may be null when `rhs_len` is zero, and may point into `*this`.
        /// @param rhs_len Number of characters to append.
        /// @return Reference to `*this`.
        constexpr basic_string& _append(const CharT* rhs, usize rhs_len)
        {
            if (rhs_len == 0)
            {
                return *this;
            }

            const usize lhs_len = length();
            if (_capacity >= lhs_len + rhs_len + 1)
            {
                // The source characters lie before the write position or
                // outside the string, so the copy never overlaps the target.
                copy_n(rhs, rhs_len, _data + lhs_len);
                _size = lhs_len + rhs_len + 1;
                _data[lhs_len + rhs_len] = static_cast<CharT>(0);
                return *this;
            }

            // Build the new storage before releasing the old one, so `rhs`
            // can still point into it.
            *this = _join(_data, lhs_len, rhs, rhs_len);
            return *this;
        }
        
    public:
        /// @brief Dynamic string ownership, access, search, and mutation.
        using iterator = CharT*;
        /// @brief Const iterator; a plain pointer to `const CharT`.
        using const_iterator = const CharT*;
        /// @brief Character type stored by the string.
        using value_type = CharT;
        /// @brief Sentinel returned by search operations when no match exists.
        static constexpr usize npos = static_cast<usize>(-1);

        /// @brief Constructs an empty string using a default-constructible allocator.
        constexpr basic_string()
        requires (DefaultConstructible<alloc_type>) = default;

        /// @brief Constructs an empty string with an explicit allocator.
        /// @param alloc Allocator used for future storage.
        constexpr basic_string(const alloc_type& alloc)
            : _alloc{alloc}
        {}

        /// @brief Allocates uninitialized storage for `size` characters.
        /// @param size Number of characters (including terminator slot) to allocate.
        constexpr basic_string(usize size)
        requires (DefaultConstructible<alloc_type>)
        {
            _data = alloc_traits
                ::allocate(_alloc, size)
                .unwrap();
            
            _capacity = size;
        }

        /// @brief Allocates storage for `size` characters with an allocator.
        /// @param size Number of characters (including terminator slot) to allocate.
        /// @param alloc Allocator used to obtain storage.
        constexpr basic_string(usize size, const alloc_type& alloc)
            : _alloc{alloc}
        {
            _data = alloc_traits
                ::allocate(_alloc, size)
                .unwrap();

            _capacity = size;
        }

        /// @brief Constructs from a null-terminated character sequence.
        /// @param cstr Null-terminated source sequence, or `nullptr` for an empty string.
        constexpr basic_string(const CharT* cstr)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) + 1 : 0;
            _capacity = _size;
            
            _data = alloc_traits
                ::allocate(_alloc, _size)
                .unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, length());
            }

            if (_data != nullptr)
            {
                _data[length()] = static_cast<CharT>(0);
            }
        }

        /// @brief Constructs from a null-terminated sequence and allocator.
        /// @param cstr Null-terminated source sequence, or `nullptr` for an empty string.
        /// @param alloc Allocator used to obtain storage.
        constexpr basic_string(
            const CharT* cstr, const alloc_type& alloc)
            : _alloc{alloc}
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) + 1 : 0;
            _capacity = _size;
            _data = alloc_traits::allocate(_alloc, _size).unwrap();
            
            if (cstr != nullptr)
            {
                _str_utils::copy(_data, cstr, length());
            }

            _data[length()] = static_cast<CharT>(0);
        }

        /// @brief Constructs from a character sequence and storage size.
        /// @param cstr Source sequence, or `nullptr` to leave the storage default-initialized.
        /// @param size Number of characters (including terminator slot) to allocate.
        constexpr basic_string(const CharT* cstr, usize size)
        requires (DefaultConstructible<alloc_type>)
            : _alloc{}
        {
            _size = size;
            _capacity = _size;
            _data = alloc_traits
                ::allocate(_alloc, _size)
                .unwrap();
            
            if (cstr != nullptr)
            {
                // Copies exactly `length()` characters, so an embedded null is kept.
                for (usize _index = 0; _index < length(); ++_index)
                {
                    _data[_index] = cstr[_index];
                }
            }

            if (_data != nullptr)
            {
                _data[length()] = static_cast<CharT>(0);
            }
        }

        /// @brief Constructs from a bounded sequence with an explicit allocator.
        /// @param cstr Source sequence, or `nullptr` to leave the storage default-initialized.
        /// @param size Number of characters (including terminator slot) to allocate.
        /// @param alloc Allocator used to obtain storage.
        constexpr basic_string(
            const CharT* cstr, usize size, const alloc_type& alloc)
            : _alloc{alloc}
        {
            _size = size;
            _capacity = _size;
            _data = alloc_traits::allocate(_alloc, _size).unwrap();
            
            if (cstr != nullptr)
            {
                // Copies exactly `length()` characters, so an embedded null is kept.
                for (usize _index = 0; _index < length(); ++_index)
                {
                    _data[_index] = cstr[_index];
                }
            }

            if (_data != nullptr)
            {
                _data[length()] = static_cast<CharT>(0);
            }
        }

        /// @brief Copies a non-owning character view into owned storage.
        /// @param view Source view whose contents are copied.
        constexpr basic_string(basic_string_view<CharT> view)
        requires (DefaultConstructible<alloc_type>)
            : basic_string{view.data(), view.length() + 1}
        {}

        /// @brief Copies a character view using an explicit allocator.
        /// @param view Source view whose contents are copied.
        /// @param alloc Allocator used to obtain storage.
        constexpr basic_string(
            basic_string_view<CharT> view, const alloc_type& alloc)
            : basic_string{view.data(), view.length() + 1, alloc}
        {}

        /// @brief Copies string contents and selects the copy allocator.
        /// @param other String whose contents and allocator selection are copied.
        constexpr basic_string(const basic_string& other)
            : _alloc{
                alloc_traits::select_on_container_copy_construction(
                    other._alloc
                )
            }
        {
            _size = other._size;
            _capacity = other._capacity;
            _data = alloc_traits
                ::allocate(_alloc, _capacity)
                .unwrap();
            
            if (other._data != nullptr)
            {
                copy_n(other._data, length(), _data);
            }

            if (_data != nullptr)
            {
                _data[length()] = static_cast<CharT>(0);
            }
        }

        /// @brief Converts and copies from a differently-encoded string, re-encoding characters.
        /// @tparam CharT2 Source character type, re-encoded into `CharT`.
        /// @param other Source string whose contents are converted.
        template <typename CharT2>
        constexpr basic_string(const basic_string<CharT2, Allocator>& other)
            : _alloc{other._alloc}
        {
            _size = _capacity = unicode::length<CharT, CharT2>(other._data) + 1;
                
            _data = alloc_traits
                ::allocate(_alloc, _capacity)
                .unwrap();
            
            unicode::convert_to(_data, _size, other._data).unwrap();

            _data[length()] = static_cast<CharT>(0);
        }

        /// @brief Transfers string storage and allocator state.
        /// @param other String whose storage and allocator are moved from; left unallocated.
        constexpr basic_string(basic_string&& other)
            : _alloc{hsd::move(other._alloc)}
        {
            hsd::swap(_size, other._size);
            hsd::swap(_capacity, other._capacity);
            hsd::swap(_data, other._data);
        }

        /// @brief Releases owned storage, if any.
        constexpr ~basic_string()
        {
            _reset();
        }

        /// @brief Replaces the contents with a copy of a null-terminated sequence.
        /// @param rhs Null-terminated source sequence.
        /// @return Reference to `*this`.
        constexpr basic_string& operator=(const CharT* rhs)
        {
            auto _new_size = _str_utils::length(rhs);
            
            _reset();
            _size = _new_size + 1;
            _capacity = _size;
            
            _data = alloc_traits
                ::allocate(_alloc, _size)
                .unwrap();
            
            copy_n(rhs, length(), _data);
            _data[length()] = static_cast<CharT>(0);

            return *this;
        }

        /// @brief Replaces the contents with a copy of a non-owning character view.
        /// @param rhs Source view whose contents are copied.
        /// @return Reference to `*this`.
        constexpr basic_string& operator=(const basic_string_view<CharT>& rhs)
        {
            _reset();
            _size = rhs.length() + 1;
            _capacity = _size;
            
            _data = alloc_traits
                ::allocate(_alloc, _size)
                .unwrap();
            
            copy_n(rhs.data(), length(), _data);
            _data[length()] = static_cast<CharT>(0);
            
            return *this;
        }

        /// @brief Copy-assigns via copy-and-swap, reusing `rhs`'s allocator selection rules.
        /// @param rhs String whose contents and allocator selection are copied.
        /// @return Reference to `*this`.
        constexpr basic_string& operator=(const basic_string& rhs)
        {
            if (this != &rhs)
            {
                basic_string _copy{rhs};
                hsd::swap(_alloc, _copy._alloc);
                hsd::swap(_data, _copy._data);
                hsd::swap(_size, _copy._size);
                hsd::swap(_capacity, _copy._capacity);
            }

            return *this;
        }

        /// @brief Replaces the contents by re-encoding characters from a differently-encoded string.
        /// @tparam CharT2 Source character type, re-encoded into `CharT`.
        /// @param rhs Source string whose contents are converted.
        /// @return Reference to `*this`.
        template <typename CharT2>
        constexpr basic_string& operator=(const basic_string<CharT2, Allocator>& rhs)
        {
            _reset();
            _alloc = rhs._alloc;
            _size = _capacity = unicode::length<CharT, CharT2>(rhs._data) + 1;
            _data = alloc_traits::allocate(_alloc, _size).unwrap();
            
            unicode::convert_to(_data, _size, rhs._data).unwrap();
            _data[length()] = static_cast<CharT>(0);
            return *this;
        }

        /// @brief Move-assigns by swapping allocator and storage with `rhs`.
        /// @param rhs String whose storage and allocator are swapped into `*this`.
        /// @return Reference to `*this`.
        constexpr basic_string& operator=(basic_string&& rhs)
        {
            if (this != &rhs)
            {
                hsd::swap(_alloc, rhs._alloc);
                hsd::swap(_size, rhs._size);
                hsd::swap(_capacity, rhs._capacity);
                hsd::swap(_data, rhs._data);
            }

            return *this;
        }

        /// @brief Returns a new string holding the concatenation of `*this` and `rhs`.
        /// @param rhs String appended after `*this`'s contents.
        /// @return New string holding the concatenation.
        constexpr basic_string operator+(const basic_string& rhs) const
        {
            return _join(_data, length(), rhs._data, rhs.length());
        }

        /// @brief Returns a new string holding the concatenation of `*this` and a character view.
        /// @param rhs View appended after `*this`'s contents.
        /// @return New string holding the concatenation.
        constexpr basic_string operator+(const basic_string_view<CharT>& rhs) const
        {
            return _join(_data, length(), rhs.data(), rhs.length());
        }

        /// @brief Returns a new string holding the concatenation of `*this` and a null-terminated sequence.
        /// @param rhs Null-terminated sequence appended after `*this`'s contents; `nullptr` counts as empty.
        /// @return New string holding the concatenation.
        constexpr basic_string operator+(const CharT* rhs) const
        {
            return _join(_data, length(), rhs, (rhs != nullptr) ? _str_utils::length(rhs) : 0);
        }

        /// @brief Returns a new string holding the concatenation of a character view and `rhs`.
        /// @param lhs View placed before `rhs`'s contents.
        /// @param rhs String appended after `lhs`'s contents.
        /// @return New string holding the concatenation.
        constexpr friend basic_string operator+(
            const basic_string_view<CharT>& lhs, const basic_string& rhs)
        {
            return _join(lhs.data(), lhs.length(), rhs._data, rhs.length());
        }

        /// @brief Returns a new string holding the concatenation of a null-terminated sequence and `rhs`.
        /// @param lhs Null-terminated sequence placed before `rhs`'s contents; `nullptr` counts as empty.
        /// @param rhs String appended after `lhs`'s contents.
        /// @return New string holding the concatenation.
        constexpr friend basic_string operator+(const CharT* lhs, const basic_string& rhs)
        {
            return _join(lhs, (lhs != nullptr) ? _str_utils::length(lhs) : 0, rhs._data, rhs.length());
        }

        /// @brief Appends `rhs`, growing storage first when the current capacity is insufficient.
        /// @param rhs String appended to `*this`.
        /// @return Reference to `*this`.
        constexpr basic_string& operator+=(const basic_string& rhs)
        {
            return _append(rhs._data, rhs.length());
        }

        /// @brief Appends a character view, growing storage first when the current capacity is insufficient.
        /// @param rhs View appended to `*this`.
        /// @return Reference to `*this`.
        constexpr basic_string& operator+=(const basic_string_view<CharT>& rhs)
        {
            return _append(rhs.data(), rhs.length());
        }

        /// @brief Appends a null-terminated sequence, growing storage first when the current capacity is insufficient.
        /// @param rhs Null-terminated sequence appended to `*this`; `nullptr` appends nothing.
        /// @return Reference to `*this`.
        constexpr basic_string& operator+=(const CharT* rhs)
        {
            return _append(rhs, (rhs != nullptr) ? _str_utils::length(rhs) : 0);
        }

        /// @brief Returns an unchecked mutable character reference.
        /// @param index Zero-based character position; must be within storage bounds.
        /// @return Mutable reference to the character at `index`.
        constexpr CharT& operator[](usize index)
        {
            return _data[index];
        }

        /// @brief Returns an unchecked const character reference.
        /// @param index Zero-based character position; must be within storage bounds.
        /// @return Const reference to the character at `index`.
        constexpr const CharT& operator[](usize index) const
        {
            return _data[index];
        }

        /// @brief Lexicographically compares two strings' contents.
        /// @details Compares by logical length, so an embedded null is an ordinary
        /// character and a shorter prefix sorts first.
        /// @param rhs String compared against `*this`.
        /// @return Negative, zero, or positive when `*this` sorts before, equal to, or after `rhs`.
        constexpr i32 operator<=>(const basic_string& rhs) const
        {
            return basic_string_view<CharT>{_data, length()}
                <=> basic_string_view<CharT>{rhs._data, rhs.length()};
        }

        /// @brief Tests whether two strings have equal size and contents.
        /// @param rhs String compared against `*this`.
        /// @return `true` when both strings have equal size and contents.
        constexpr bool operator==(const basic_string& rhs) const
        {
            return _size == rhs._size && operator<=>(rhs) == 0;
        }

        /// @brief Lexicographically compares this string's contents against a character view.
        /// @details Compares by logical length, the same rule as `basic_string_view`.
        /// @param rhs View compared against `*this`.
        /// @return Negative, zero, or positive when `*this` sorts before, equal to, or after `rhs`.
        constexpr i32 operator<=>(const basic_string_view<CharT>& rhs) const
        {
            return basic_string_view<CharT>{_data, length()} <=> rhs;
        }

        /// @brief Tests whether this string and a character view have equal length and contents.
        /// @param rhs View compared against `*this`.
        /// @return `true` when both have equal length and contents.
        constexpr bool operator==(const basic_string_view<CharT>& rhs) const
        {
            return length() == rhs.length() && operator<=>(rhs) == 0;
        }

        /// @brief Returns a checked mutable character reference.
        /// @param index Zero-based character position.
        /// @return Reference wrapper for the character, or `bad_access` when `index` is out of bounds.
        constexpr auto at(usize index)
            -> result<reference<CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Returns a checked const character reference.
        /// @param index Zero-based character position.
        /// @return Reference wrapper for the character, or `bad_access` when `index` is out of bounds.
        constexpr auto at(usize index) const
            -> result<reference<const CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Finds a string at or after a logical character position.
        /// @param str String to search for.
        /// @param pos Logical position to begin searching from.
        /// @return Index of the first match, or `npos` when not found.
        constexpr usize find(const basic_string& str, usize pos = 0) const
        {
            return basic_string_view<CharT>{_data, length()}.find(
                basic_string_view<CharT>{str._data, str.length()}, pos
            );
        }

        /// @brief Finds a null-terminated sequence at or after `pos`.
        /// @param str Null-terminated sequence searched for.
        /// @param pos Logical position from which searching begins.
        /// @return First matching index, or `npos`.
        constexpr usize find(const CharT* str, usize pos = 0) const
        {
            if (pos >= length())
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

        /// @brief Finds a character at or after `pos`.
        /// @param letter Character searched for.
        /// @param pos Logical position from which searching begins.
        /// @return First matching index, or `npos`.
        constexpr usize find(CharT letter, usize pos = 0) const
        {
            if (pos >= length())
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

        /// @brief Finds the last occurrence of a string up to `pos`.
        /// @param str String to search for.
        /// @param pos Logical position to search up to, or `npos` to search the whole string.
        /// @return Index of the last match, or `npos` when not found.
        constexpr usize rfind(const basic_string& str, usize pos = npos) const
        {
            return basic_string_view<CharT>{_data, length()}.rfind(
                basic_string_view<CharT>{str._data, str.length()}, pos
            );
        }

        /// @brief Finds the last occurrence of a null-terminated sequence.
        /// @param str Null-terminated sequence to search for.
        /// @param pos Logical position to search up to, or `npos` to search the whole string.
        /// @return Index of the last match, or `npos` when not found.
        constexpr usize rfind(const CharT* str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    _data, str, length()
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
                    &_data[pos], str, length() - pos
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

        /// @brief Finds the last occurrence of a character.
        /// @param str Character to search for.
        /// @param pos Logical position to search up to, or `npos` to search the whole string.
        /// @return Index of the last match, or `npos` when not found.
        constexpr usize rfind(CharT str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    _data, str, length()
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
                    &_data[pos], str, length() - pos
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

        /// @brief Tests whether the logical string begins with a character.
        /// @param letter Character tested at the beginning.
        /// @return True when the string begins with `letter`.
        constexpr bool starts_with(CharT letter) const
        {
            if (_data != nullptr)
                return _data[0] == letter;

            return false;
        }

        /// @brief Tests whether the logical string begins with a sequence.
        /// @param str Sequence tested at the beginning.
        /// @return True when the string begins with `str`.
        constexpr bool starts_with(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        /// @brief Tests whether the logical string begins with another string's contents.
        /// @param str String to test for.
        /// @return `true` when the string begins with `str`'s contents.
        constexpr bool starts_with(const basic_string& str) const
        {
            if (_data != nullptr)
                return find(str) == 0;

            return false;
        }

        /// @brief Tests whether a character occurs in the logical string.
        /// @param letter Character searched for.
        /// @return True when `letter` occurs.
        constexpr bool contains(CharT letter) const
        {
            if (_data != nullptr)
                return find(letter) != npos;

            return false;
        }

        /// @brief Tests whether a sequence occurs in the logical string.
        /// @param str Sequence searched for.
        /// @return True when `str` occurs.
        constexpr bool contains(const CharT* str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        /// @brief Tests whether another string's contents occur in the logical string.
        /// @param str String to search for.
        /// @return `true` when `str`'s contents occur anywhere in the string.
        constexpr bool contains(const basic_string& str) const
        {
            if (_data != nullptr)
                return find(str) != npos;

            return false;
        }

        /// @brief Tests whether the logical string ends with a character.
        /// @param letter Character to test for.
        /// @return `true` when the string is non-empty and its last character equals `letter`.
        constexpr bool ends_with(CharT letter) const
        {
            // a
            if (_data != nullptr)
                return _data[length() - 1] == letter;

            return false;
        }

        /// @brief Tests whether the logical string ends with a sequence.
        /// @param str Null-terminated sequence to test for.
        /// @return `true` when the string ends with `str`.
        constexpr bool ends_with(const CharT* str) const
        {
            usize _len = cstring::length(str);

            if (_data != nullptr)
                return rfind(str) == (length() - _len);

            return false;
        }

        /// @brief Tests whether the logical string ends with another string's contents.
        /// @param str String to test for.
        /// @return `true` when the string ends with `str`'s contents.
        constexpr bool ends_with(const basic_string& str) const
        {
            if (_data != nullptr)
                return rfind(str) == (length() - str.length());

            return false;
        }

        /// @brief Copies a bounded logical substring into a new string.
        /// @param from Starting logical character position.
        /// @param length Number of characters to copy.
        /// @return New string with the requested substring, or `bad_access` when the range exceeds the string.
        constexpr auto sub_string(usize from, usize length)
            -> result<basic_string, bad_access>
        {
            if (from > _size || (from + length) > _size)
                return bad_access{};

            return basic_string{_data + from, length + 1};
        }

        /// @brief Copies the logical suffix beginning at `from`.
        /// @param from Starting logical character position.
        /// @return New string with the requested suffix, or `bad_access` when `from` exceeds the string.
        constexpr auto sub_string(usize from)
            -> result<basic_string, bad_access>
        {
            return sub_string(from, length() - from);
        }

        /// @brief Erases one character and returns the following iterator.
        /// @param pos Iterator to the character to erase.
        /// @return Iterator following the erased character, or `bad_access` when `pos` is out of range.
        constexpr auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        /// @brief Erases the half-open character range `[from, to)`.
        /// @param from First iterator in the erased range.
        /// @param to Iterator one past the erased range.
        /// @return Iterator after the erased range, or `bad_access`.
        constexpr auto erase_for(const_iterator from, const_iterator to)
            -> result<iterator, bad_access>
        {
            if (from < begin() || from > end() || to < begin() || to > end() || from > to)
                return bad_access{};

            usize _current_pos = static_cast<usize>(from - begin());
            usize _last_pos = static_cast<usize>(to - begin());

            for (usize _index = 0; _index < _capacity - _last_pos; _index++)
            {
                this->_data[_current_pos + _index] = 
                    hsd::move(this->_data[_last_pos + _index]);
            }

            _size -= static_cast<usize>(to - from);
            _data[length()] = static_cast<CharT>(0);
            return begin() + _last_pos;
        }
    
        /// @brief Ensures storage capacity for at least `new_cap` characters.
        /// @param new_cap Minimum requested storage capacity.
        constexpr void reserve(usize new_cap)
        {
            if (new_cap > _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;

                while (_new_capacity < new_cap)
                    _new_capacity += (_new_capacity + 1) / 2;

                // Allocate space for NULL byte
                auto* _new_buf = alloc_traits
                    ::allocate(_alloc, _new_capacity)
                    .unwrap();

                _new_buf[_new_capacity - 1] = 0;
                
                for (usize _index = 0; _index < _size; ++_index)
                {
                    auto& _value = _data[_index];
                    _new_buf[_index] = hsd::move(_value);
                }

                alloc_traits::deallocate(
                    _alloc, _data, _capacity
                ).unwrap();
                _data = _new_buf;
                _capacity = _new_capacity;
            }
        }

        /// @brief Changes logical length while preserving a null terminator.
        /// @param new_length New logical character length.
        constexpr void resize(usize new_length)
        {
            if (new_length >= _capacity)
            {
                // To handle _capacity = 0 case
                usize _new_capacity = _capacity ? _capacity : 1;
                
                while (_new_capacity < new_length)
                    _new_capacity += (_new_capacity + 1) / 2;

                if (_new_capacity == new_length)
                    ++_new_capacity;

                CharT* _new_buf = alloc_traits
                    ::allocate(_alloc, _new_capacity)
                    .unwrap();
                
                usize _index = 0;

                for (; _index < _size; ++_index)
                {
                    auto& _value = _data[_index];
                    alloc_traits::construct(
                        _alloc, &_new_buf[_index], hsd::move(_value)
                    );
                    alloc_traits::destroy(_alloc, &_value);
                }
                for (; _index <= new_length; ++_index)
                {
                    alloc_traits::construct(_alloc, &_new_buf[_index]);
                }

                alloc_traits::deallocate(
                    _alloc, _data, _capacity
                ).unwrap();
                _data = _new_buf;
                _capacity = _new_capacity;
                _size = new_length;
            }
            else if (new_length > _size)
            {
                for (usize _index = _size; _index <= new_length; ++_index)
                {
                    alloc_traits::construct(_alloc, &_data[_index]);
                }
                
                _size = new_length;
            }
            else if (new_length < _size)
            {
                for (usize _index = _size; _index > new_length; --_index)
                    alloc_traits::destroy(_alloc, &_data[_index - 1]);
                
                _size = new_length;
                _data[_size] = 0;
            }
        }

        /// @brief Constructs and appends one character in place.
        /// @param args Arguments forwarded to the character constructor.
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if (_size == 0)
            {
                reserve(2);
                _size = 2;
                _data[0] = CharT{hsd::forward<Args>(args)...};
                _data[1] = '\0';
            }
            else
            {
                reserve(_size + 1);
                _data[_size - 1] = CharT{hsd::forward<Args>(args)...};
                _data[_size++] = '\0';
            }
        }

        /// @brief Appends one character by copy.
        /// @param val Character copied to the end.
        constexpr void push_back(const CharT& val)
        {
            emplace_back(val);
        }

        /// @brief Appends one character by move.
        /// @param val Character moved to the end.
        constexpr void push_back(CharT&& val)
        {
            emplace_back(hsd::move(val));
        }

        /// @brief Truncates the logical length to 0, retaining allocated capacity.
        constexpr void clear()
        {
            if (_capacity != 0)
            {
                _data[0] = '\0';
                _size = 0;
            }
        }

        /// @brief Removes the last logical character when one exists.
        constexpr void pop_back()
        {
            if (_size > 1)
            {
                --_size;
                _data[_size - 1] = '\0';
            }
        }

        /// @brief Returns an unchecked mutable reference to the first character. Undefined if empty.
        /// @return Mutable reference to the first character.
        constexpr CharT& front()
        {
            return _data[0];
        }

        /// @brief Returns an unchecked mutable reference to the last logical character. Undefined if empty.
        /// @return Mutable reference to the last character.
        constexpr CharT& back()
        {
            return _data[_size - 2];
        }

        /// @brief Returns the raw storage count, i.e. logical length plus one for the terminator, or 0 when unallocated.
        /// @return Storage count including the terminator slot.
        constexpr usize size() const
        {
            return _size;
        }

        /// @brief Returns the logical character count, excluding the null terminator.
        /// @return Logical character length.
        constexpr usize length() const
        {
            return (_size != 0) ? _size - 1 : 0;
        }

        /// @brief Returns the number of characters currently allocated, including the terminator slot.
        /// @return Allocated character capacity.
        constexpr usize capacity() const
        {
            return _capacity;
        }

        ///
        /// @brief Returns the allocator associated with this string's storage.
        ///
        /// @return Copy of the allocator associated with storage.
        constexpr Allocator get_allocator() const
        {
            return _alloc;
        }

        /// @brief Returns a mutable pointer to the underlying null-terminated storage.
        /// @return Mutable storage pointer, possibly null.
        constexpr iterator data()
        {
            return _data;
        }

        /// @brief Returns a const pointer to the underlying null-terminated storage.
        /// @return Const storage pointer, possibly null.
        constexpr const_iterator data() const
        {
            return _data;
        }

        /// @brief Returns a null-terminated pointer, using a shared empty string when unallocated.
        /// @return Null-terminated string pointer.
        constexpr const_iterator c_str() const
        {
            return _data != nullptr ? _data : &_s_empty;
        }

        /// @brief Returns a mutable iterator to the first character.
        /// @return Mutable begin iterator.
        constexpr iterator begin()
        {
            return data();
        }

        /// @brief Returns a const iterator to the first character.
        /// @return Const begin iterator.
        constexpr const_iterator begin() const
        {
            return data();
        }

        /// @brief Returns a mutable iterator one past the last logical character.
        /// @return Mutable end iterator.
        /// @brief Returns a reverse iterator to the last character.
        constexpr contiguous_reverse_iterator<CharT*> rbegin()
        {
            return contiguous_reverse_iterator<CharT*>{end()};
        }

        /// @brief Returns a const reverse iterator to the last character.
        constexpr contiguous_reverse_iterator<const CharT*> rbegin() const
        {
            return contiguous_reverse_iterator<const CharT*>{end()};
        }

        /// @brief Returns the reverse end sentinel, the position before the first character.
        constexpr contiguous_reverse_iterator<CharT*> rend()
        {
            return contiguous_reverse_iterator<CharT*>{begin()};
        }

        /// @brief Returns the const reverse end sentinel.
        constexpr contiguous_reverse_iterator<const CharT*> rend() const
        {
            return contiguous_reverse_iterator<const CharT*>{begin()};
        }

        constexpr iterator end()
        {
            return begin() + length();
        }

        /// @brief Returns a const iterator one past the last logical character.
        /// @return Const end iterator.
        constexpr const_iterator end() const
        {
            return begin() + length();
        }

        /// @brief Returns a const iterator to the first character.
        /// @return Const begin iterator.
        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        /// @brief Returns a const iterator one past the last logical character.
        /// @return Const end iterator.
        constexpr const_iterator cend() const
        {
            return end();
        }

        /// @brief Returns a non-owning view over the logical contents.
        /// @return Non-owning view over the logical characters.
        explicit constexpr operator basic_string_view<CharT>() const
        {
            return basic_string_view<CharT>{_data, length()};
        }
    };

    /// @brief Parses a string by copying a view when the character types already match.
    /// @brief Parses a same-character view by copying its logical contents.
    /// @param from Source view without an owned terminator.
    /// @param to Destination owning string.
    /// @return Empty success.
    template <typename CharT>
    static constexpr option_err<runtime_error> _parse_from(
        const basic_string_view<CharT>& from, basic_string<CharT>& to)
    {
        to = from;
        return {};
    }

    /// @brief Parses a string by re-encoding a view of a differing character type.
    /// @brief Parses a view by converting its character encoding into `T`.
    /// @param from Source view.
    /// @param to Destination owning string.
    /// @return Empty success or the Unicode conversion error.
    template <typename CharT, CharacterType T>
    requires (!IsSame<CharT, T>)
    static constexpr option_err<runtime_error> _parse_from(
        const basic_string_view<CharT>& from, basic_string<T>& to)
    {
        auto _len = unicode::length<T, CharT>(from.data(), from.size()) + 1;
        to.resize(_len);
        
        auto _res = unicode::convert_to(to.data(), _len, from.data(), from.size());
        
        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }
        
        return {};
    }

    /// @brief Formats a `basic_string` into `str`, applying `fmt`'s style/reset markers and re-encoding as needed.
    /// @param str Destination character buffer.
    /// @param sz Destination capacity.
    /// @param val String value to format.
    /// @return Number of characters written, or a `runtime_error` if `str` is too small or transcoding fails.
    /// @details Only accepts a format with no presentation tag, since text has no numeric form.
    template <format_output fmt, typename CharT, CharacterType T>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const basic_string<T>& val)
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
        auto _res = unicode::convert_to<CharT, T>(str, _room, val.c_str(), val.length());

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        usize _pos = _room - _res.unwrap() - 1;
        str = _print_to_impl(str + _pos, fmt.reset);

        return _prefix + _pos + _suffix;
    }

    template <typename CharT, usize N>
    /// @brief Fixed-capacity null-terminated string stored inline.
    class static_basic_string
    {
    private:
        /// @brief C-string algorithms (length/copy/compare/find) for `CharT`.
        using _str_utils = basic_cstring<CharT>;
        /// @brief Inline null-terminated character storage; no heap allocation.
        CharT _data[N]{};
        /// @brief Logical length plus one for the null terminator, or 0 when empty.
        usize _size = 0;
        /// @brief Fixed storage capacity, equal to `N`.
        usize _capacity = N;

    public:
        /// @brief Fixed-capacity string storage, access, and mutation.
        using iterator = CharT*;
        /// @brief Const iterator; a plain pointer to `const CharT`.
        using const_iterator = const CharT*;
        /// @brief Character type stored by the string.
        using value_type = CharT;
        /// @brief Sentinel returned by search operations when no match exists.
        static constexpr usize npos = static_cast<usize>(-1);

        /// @brief Constructs an empty string.
        constexpr static_basic_string() = default;

        /// @brief Sets the logical length to `size`, leaving characters uninitialized. Aborts if `size >= N`.
        /// @param size Storage/logical size requested, excluding the terminator.
        constexpr static_basic_string(usize size)
        {
            if (size < N)
            {
                _size = size;
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", size, "): size is too big\n"
                );

                abort();
            }
        }

        /// @brief Constructs from a null-terminated sequence. Aborts if it does not fit in `N`.
        /// @param cstr Null-terminated source sequence.
        constexpr static_basic_string(const CharT* cstr)
        {
            _size = (cstr != nullptr) ? _str_utils::length(cstr) + 1 : 0;

            if (_size < N)
            {
                if (cstr != nullptr)
                {
                    // Copies exactly `length()` characters, so an embedded null is kept.
                    for (usize _index = 0; _index < length(); ++_index)
                    {
                        _data[_index] = cstr[_index];
                    }
                }

                _data[length()] = static_cast<CharT>(0);
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", cstr, "): cstr is too big to be contained\n"
                );

                abort();
            }
        }

        /// @brief Constructs from a bounded character sequence. Aborts if `size >= N`.
        /// @param cstr Source sequence, possibly null.
        /// @param size Source/storage size including the terminator slot.
        constexpr static_basic_string(const CharT* cstr, usize size)
        {
            if (size < N)
            {
                _size = size;
                
                if (cstr != nullptr)
                {
                    // Copies exactly `length()` characters, so an embedded null is kept.
                    for (usize _index = 0; _index < length(); ++_index)
                    {
                        _data[_index] = cstr[_index];
                    }
                }

                _data[length()] = static_cast<CharT>(0);
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", cstr, ", ", size, "): size is too big\n"
                );

                abort();
            } 
        }

        /// @brief Copies a non-owning character view into inline storage. Aborts if it does not fit in `N`.
        /// @param view Non-owning source view.
        constexpr static_basic_string(basic_string_view<CharT> view)
            : static_basic_string(view.data(), view.length() + 1)
        {}

        /// @brief Copies another string's contents of the same capacity.
        /// @param other Source string copied into inline storage.
        constexpr static_basic_string(const static_basic_string& other)
        {
            _size = other._size;
            copy_n(other._data, length(), _data);
        }

        /// @brief Copies from a differently-sized instantiation. Aborts if it does not fit in `N`.
        template <usize N2>
        /// @param other Source string copied when it fits this capacity.
        constexpr static_basic_string(const static_basic_string<CharT, N2>& other)
        {
            if (other._size < N)
            {
                _size = other._size;
                copy_n(other._data, length(), _data);
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", other._data, "): other is too big to be contained\n"
                );

                abort();
            }
        }

        /// @brief Re-encodes from a differently-typed instantiation. Aborts if it does not fit in `N`.
        template <typename CharT2, usize N2>
        /// @param other Differently encoded source string.
        constexpr static_basic_string(const static_basic_string<CharT2, N2>& other)
        {
            auto _len = unicode::length(other._data);

            if (_len + 1 < N2)
            {
                _size = _len + 1;
                
                unicode::convert_to(_data, _size, other._data).unwrap();
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", other._data, ", ", other._size, "): cstr is too big to be contained\n"
                );

                abort();
            }
        }

        /// @brief Transfers characters from another string of the same capacity.
        /// @param other Source string whose characters are moved.
        constexpr static_basic_string(static_basic_string&& other)
        {
            hsd::swap(_size, other._size);
            hsd::move(other._data, other._data + _size, _data);
        }

        /// @brief Transfers characters from a differently-sized instantiation. Ignored (no-op) if `other` does not fit in `N`.
        template <usize N2>
        /// @param other Source string moved when it fits this capacity.
        constexpr static_basic_string(static_basic_string<CharT, N2>&& other)
        {
            if (other._size < N)
            {
                hsd::swap(_size, other._size);
                hsd::move(other._data, other._data + _size, _data);
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string(", other._data, ", ", other._size, "): other is too big to be contained\n"
                );
            }
        }

        /// @brief Replaces the contents with a copy of a null-terminated sequence. Aborts if it does not fit in `N`.
        /// @param rhs Null-terminated source sequence.
        /// @return `*this` after assignment.
        constexpr static_basic_string& operator=(const CharT* rhs)
        {
            _size = _str_utils::length(rhs) + 1;

            if (_size < N)
            {
                _str_utils::copy(_data, rhs, _size);
            }
            else
            {
                result_detail::write_error(
                    "Error at operator=(", rhs, "): rhs is too big to be contained\n"
                );

                abort();
            }

            return *this;
        }

        /// @brief Replaces the contents with a copy of a non-owning character view. Aborts if it does not fit in `N`.
        /// @param rhs Source view copied into inline storage.
        /// @return `*this` after assignment.
        constexpr static_basic_string& operator=(const basic_string_view<CharT>& rhs)
        {
            if (rhs.size() < N)
            {
                _size = rhs.size();
                copy_n(rhs.data(), _size, _data);
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string::operator=(", rhs, "): rhs is too big to be contained\n"
                );

                abort();
            }

            return *this;
        }

        /// @brief Copy-assigns from a differently-sized instantiation whose capacity fits within `N`.
        /// @param rhs Source static string.
        /// @return `*this` after assignment.
        template <usize N2> requires (N2 <= N)
        constexpr static_basic_string& operator=(
            const static_basic_string<CharT, N2>& rhs)
        {
            _size = rhs._size;
            copy_n(rhs.c_str(), _size, _data);
            return *this;
        }

        /// @brief Re-encodes and assigns from a differently-typed instantiation. Aborts if it does not fit.
        /// @param rhs Differently encoded source string.
        /// @return `*this` after assignment.
        template <typename CharT2, usize N2>
        constexpr static_basic_string& operator=(
            const static_basic_string<CharT2, N2>& rhs)
        {
            auto _len = unicode::length(rhs);

            if (_len + 1 < N2)
            {
                _size = _len + 1;

                unicode::convert_to(_data, _size, rhs, _size).unwrap();
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string::operator=(", rhs, "): rhs is too big to be contained\n"
                );

                abort();
            }

            return *this;
        }

        /// @brief Move-assigns by swapping contents with a differently-sized instantiation whose capacity fits within `N`.
        /// @param rhs Source static string moved into inline storage.
        /// @return `*this` after assignment.
        template <usize N2> requires (N2 <= N)
        constexpr static_basic_string& operator=(static_basic_string<CharT, N2>&& rhs)
        {
            hsd::swap(_size, rhs._size);
            hsd::swap(_capacity, rhs._capacity);
            hsd::move(rhs._data, rhs._data + _size, _data);
            return *this;
        }

        /// @brief Returns an unchecked mutable character reference.
        /// @param index Zero-based character index.
        /// @return Mutable reference to the selected character.
        constexpr CharT& operator[](usize index)
        {
            return _data[index];
        }

        /// @brief Returns an unchecked const character reference.
        /// @param index Zero-based character index.
        /// @return Const reference to the selected character.
        constexpr const CharT& operator[](usize index) const
        {
            return _data[index];
        }

        /// @brief Lexicographically compares against a differently-sized instantiation's contents.
        template <usize N2>
        /// @param rhs String compared with this string.
        /// @return Negative, zero, or positive lexical comparison result.
        constexpr i32 operator<=>(const static_basic_string<CharT, N2>& rhs) const
        {
            return basic_string_view<CharT>{_data, length()}
                <=> basic_string_view<CharT>{rhs._data, rhs.length()};
        }

        /// @brief Tests whether two strings have equal size and contents.
        template <usize N2>
        /// @param rhs String compared with this string.
        /// @return True when size and contents match.
        constexpr bool operator==(const static_basic_string<CharT, N2>& rhs) const
        {
            return _size == rhs._size && operator<=>(rhs) == 0;
        }

        /// @brief Lexicographically compares this string's contents against a character view.
        /// @param rhs View compared with this string.
        /// @return Negative, zero, or positive lexical comparison result.
        constexpr i32 operator<=>(const basic_string_view<CharT>& rhs) const
        {
            return basic_string_view<CharT>{_data, length()} <=> rhs;
        }

        /// @brief Tests whether this string and a character view have equal length and contents.
        /// @param rhs View compared with this string.
        /// @return True when size and contents match.
        constexpr bool operator==(const basic_string_view<CharT>& rhs) const
        {
            return length() == rhs.length() && operator<=>(rhs) == 0;
        }

        /// @brief Returns a checked mutable character reference.
        /// @param index Zero-based character index.
        /// @return Mutable reference result or `bad_access`.
        constexpr auto at(usize index)
            -> result<reference<CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Returns a checked const character reference.
        /// @param index Zero-based character index.
        /// @return Const reference result or `bad_access`.
        constexpr auto at(usize index) const
            -> result<reference<const CharT>, bad_access>
        {
            if(index >= _size)
                return bad_access{};

            return {_data[index]};
        }

        /// @brief Finds another string's contents at or after a logical character position.
        /// @param str String searched for.
        /// @param pos Logical position from which searching begins.
        /// @return First matching index, or `npos`.
        template <usize N2>
        constexpr usize find(
            const static_basic_string<CharT, N2>& str, usize pos = 0) const
        {
            return basic_string_view<CharT>{_data, length()}.find(
                basic_string_view<CharT>{str._data, str.length()}, pos
            );
        }

        /// @brief Finds a null-terminated sequence at or after `pos`.
        /// @param str Null-terminated sequence searched for.
        /// @param pos Logical position from which searching begins.
        /// @return First matching index, or `npos`.
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

        /// @brief Finds a character at or after `pos`.
        /// @param letter Character searched for.
        /// @param pos Logical position from which searching begins.
        /// @return First matching index, or `npos`.
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

        /// @brief Finds the last occurrence of another string's contents up to `pos`.
        /// @param str String searched for.
        /// @param pos Reverse-search bound, or `npos` for the full string.
        /// @return Last matching index, or `npos`.
        template <usize N2>
        constexpr usize rfind(
            const static_basic_string<CharT, N2>& str, usize pos = npos) const
        {
            return basic_string_view<CharT>{_data, length()}.rfind(
                basic_string_view<CharT>{str._data, str.length()}, pos
            );
        }

        /// @brief Finds the last occurrence of a null-terminated sequence up to `pos`.
        /// @param str Null-terminated sequence searched for.
        /// @param pos Reverse-search bound, or `npos` for the full string.
        /// @return Last matching index, or `npos`.
        constexpr usize rfind(const CharT* str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, length()
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
                    &_data[pos], str, length() - pos
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

        /// @brief Finds the last occurrence of a character up to `pos`.
        /// @param str Character searched for.
        /// @param pos Reverse-search bound, or `npos` for the full string.
        /// @return Last matching index, or `npos`.
        constexpr usize rfind(CharT str, usize pos = npos) const
        {
            if (pos >= _size && pos != npos)
            {
                return npos;
            }
            else if (pos == npos)
            {
                const CharT* _find_addr = _str_utils::find_rev(
                    &_data[pos], str, length()
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
                    &_data[pos], str, length() - pos
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

        /// @brief Tests whether the logical string begins with a character.
        /// @param letter Character tested at the beginning.
        /// @return True when the string begins with `letter`.
        constexpr bool starts_with(CharT letter) const
        {
            if (_size != 0)
                return _data[0] == letter;

            return false;
        }

        /// @brief Tests whether the logical string begins with a sequence.
        /// @param str Sequence tested at the beginning.
        /// @return True when the string begins with `str`.
        constexpr bool starts_with(const CharT* str) const
        {
            if (_size != 0)
                return find(str) == 0;

            return false;
        }

        /// @brief Tests whether the logical string begins with another string's contents.
        template <usize N2>
        /// @param str String tested at the beginning.
        /// @return True when the string begins with `str`.
        constexpr bool starts_with(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return find(str) == 0;

            return false;
        }

        /// @brief Tests whether a character occurs in the logical string.
        /// @param letter Character searched for.
        /// @return True when `letter` occurs.
        constexpr bool contains(CharT letter) const
        {
            if (_size != 0)
                return find(letter) != npos;

            return false;
        }

        /// @brief Tests whether a sequence occurs in the logical string.
        /// @param str Sequence searched for.
        /// @return True when `str` occurs.
        constexpr bool contains(const CharT* str) const
        {
            if (_size != 0)
                return find(str) != npos;

            return false;
        }

        /// @brief Tests whether another string's contents occur in the logical string.
        template <usize N2>
        /// @param str String searched for.
        /// @return True when `str` occurs.
        constexpr bool contains(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return find(str) != npos;

            return false;
        }

        /// @brief Tests whether the logical string ends with a character.
        /// @param letter Character tested at the end.
        /// @return True when the string ends with `letter`.
        constexpr bool ends_with(CharT letter) const
        {
            if (_size != 0)
                return _data[length() - 1] == letter;

            return false;
        }

        /// @brief Tests whether the logical string ends with a sequence.
        /// @param str Sequence tested at the end.
        /// @return True when the string ends with `str`.
        constexpr bool ends_with(const CharT* str) const
        {
            usize _len = cstring::length(str);

            if (_size != 0)
                return rfind(str) == (_size - _len);

            return false;
        }

        /// @brief Tests whether the logical string ends with another string's contents.
        template <usize N2>
        /// @param str String tested at the end.
        /// @return True when the string ends with `str`.
        constexpr bool ends_with(
            const static_basic_string<CharT, N2>& str) const
        {
            if (_size != 0)
                return rfind(str) == (_size - str._size);

            return false;
        }

        /// @brief Erases one character and returns the following iterator.
        /// @param pos Iterator identifying the erased character.
        /// @return Iterator after the erased character, or `bad_access`.
        constexpr auto erase(const_iterator pos)
            -> result<iterator, bad_access>
        {
            return erase_for(pos, pos + 1);
        }

        /// @brief Erases the half-open character range `[from, to)`, shifting trailing characters left.
        /// @param from First iterator in the erased range.
        /// @param to Iterator one past the erased range.
        /// @return Iterator after the erased range, or `bad_access`.
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
                    hsd::move(this->_data[_last_pos + _index]);
            }

            _size -= static_cast<usize>(to - from) + 1;
            return begin() + _last_pos;
        }

        /// @param args Arguments forwarded to the character constructor.
        template <typename... Args>
        constexpr void emplace_back(Args&&... args)
        {
            if (_size < _capacity)
            {
                _data[_size] = CharT{hsd::forward<Args>(args)...};
                _data[++_size] = '\0';
            }
            else
            {
                result_detail::write_error(
                    "Error at static_basic_string:: emplace_back: String is full\n"
                );

                abort();
            }
        }

        /// @param val Character copied to the end.
        constexpr void push_back(const CharT& val)
        {
            emplace_back(val);
        }

        /// @param val Character moved to the end.
        constexpr void push_back(CharT&& val)
        {
            emplace_back(hsd::move(val));
        }

        /// @brief Removes all logical characters while retaining capacity.
        constexpr void clear()
        {
            if (_capacity != 0)
            {
                _data[0] = '\0';
                _size = 0;
            }
        }

        /// @param size New logical size, including the terminator slot.
        constexpr void resize(usize size)
        {
            if (size > _capacity)
            {
                result_detail::write_error(
                    "Error at static_basic_string::resize(", size, "): size is bigger than capacity\n"
                );

                abort();
            }
            else if (size < _capacity)
            {
                _size = size;
            }
        }

        /// @brief Removes the final logical character when non-empty.
        constexpr void pop_back()
        {
            if (_size > 1)
            {
                --_size;
                _data[_size - 1] = '\0';
            }
        }

        /// @return Mutable reference to the first logical character.
        constexpr CharT& front()
        {
            return _data[0];
        }

        /// @return Mutable reference to the final logical character.
        constexpr CharT& back()
        {
            return _data[length() - 1];
        }

        /// @return Storage size including the terminator slot.
        constexpr usize size() const
        {
            return (_size != 0) ? _size : 0;
        }

        /// @return Logical character length excluding the terminator.
        constexpr usize length() const
        {
            return (_size != 0) ? _size - 1 : 0;
        }

        /// @return Fixed inline storage capacity.
        constexpr usize capacity() const
        {
            return _capacity;
        }

        /// @return Mutable inline storage pointer.
        constexpr iterator data()
        {
            return _data;
        }

        /// @return Const inline storage pointer.
        constexpr const_iterator data() const
        {
            return _data;
        }

        /// @return Null-terminated character pointer.
        constexpr const_iterator c_str() const
        {
            return _data;
        }

        /// @return Mutable iterator to the first logical character.
        constexpr iterator begin()
        {
            return data();
        }

        /// @return Const iterator to the first logical character.
        constexpr const_iterator begin() const
        {
            return data();
        }

        /// @return Mutable iterator one past the logical characters.
        constexpr iterator end()
        {
            return begin() + length();
        }

        /// @return Const iterator one past the logical characters.
        constexpr const_iterator end() const
        {
            return begin() + length();
        }

        /// @return Const iterator to the first logical character.
        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        /// @return Const iterator one past the logical characters.
        constexpr const_iterator cend() const
        {
            return end();
        }

        /// @return Non-owning view over the logical characters.
        explicit constexpr operator basic_string_view<CharT>() const
        {
            return basic_string_view<CharT>(_data, _size);
        }
    };

    /// @brief Formats an inline static string into a character buffer.
    /// @param str Destination character buffer.
    /// @param sz Destination capacity.
    /// @param val Static string value to format.
    /// @return Number of characters written, or a formatting/conversion error.
    template <format_output fmt, typename CharT, CharacterType T, usize N>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && fmt.tag == decltype(fmt)::none)
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const static_basic_string<T, N>& val)
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
        auto _res = unicode::convert_to<CharT, T>(str, _room, val.data(), val.size() - 1);

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        usize _pos = _room - _res.unwrap() - 1;
        str = _print_to_impl(str + _pos, fmt.reset);

        return _prefix + _pos + _suffix;
    }

    /// @brief Converts a value to an owning narrow string.
    /// @param val Value formatted by the project pseudo-string converter.
    /// @return Owning narrow string.
    template <typename T>
    static constexpr auto to_string(T val)
    {
        const auto _res = to_pseudo_string<char>(val);
        return basic_string<char>{_res.data(), _res.size() + 1};
    }

    /// @brief Returns an existing narrow string without copying.
    /// @param val Existing string.
    /// @return Reference to `val`.
    static constexpr auto& to_string(basic_string<char>& val)
    {
        return val;
    }

    /// @brief Converts a value to an owning wide string.
    /// @param val Value formatted by the project pseudo-string converter.
    /// @return Owning wide string.
    template <typename T>
    static constexpr auto to_wstring(T val)
    {
        const auto _res = to_pseudo_string<wchar>(val);
        return basic_string<wchar>{_res.data(), _res.size() + 1};
    }

    /// @brief Returns an existing wide string without copying.
    /// @param val Existing string.
    /// @return Reference to `val`.
    static constexpr auto& to_wstring(basic_string<wchar>& val)
    {
        return val;
    }

    /// @brief Converts a value to an owning UTF-8 string.
    /// @param val Value formatted by the project pseudo-string converter.
    /// @return Owning UTF-8 string.
    template <typename T>
    static constexpr auto to_u8string(T val)
    {
        const auto _res = to_pseudo_string<char8>(val);
        return basic_string<char8>{_res.data(), _res.size() + 1};
    }

    /// @brief Returns an existing UTF-8 string without copying.
    /// @param val Existing string.
    /// @return Reference to `val`.
    static constexpr auto& to_u8string(basic_string<char8>& val)
    {
        return val;
    }

    /// @brief Converts a value to an owning UTF-16 string.
    /// @param val Value formatted by the project pseudo-string converter.
    /// @return Owning UTF-16 string.
    template <typename T>
    static constexpr auto to_u16string(T val)
    {
        const auto _res = to_pseudo_string<char16>(val);
        return basic_string<char16>{_res.data(), _res.size() + 1};
    }

    /// @brief Returns an existing UTF-16 string without copying.
    /// @param val Existing string.
    /// @return Reference to `val`.
    static constexpr auto& to_u16string(basic_string<char16>& val)
    {
        return val;
    }

    /// @brief Converts a value to an owning UTF-32 string.
    /// @param val Value formatted by the project pseudo-string converter.
    /// @return Owning UTF-32 string.
    template <typename T>
    static constexpr auto to_u32string(T val)
    {
        const auto _res = to_pseudo_string<char32>(val);
        return basic_string<char32>{_res.data(), _res.size() + 1};
    }

    /// @brief Returns an existing UTF-32 string without copying.
    /// @param val Existing string.
    /// @return Reference to `val`.
    static constexpr auto& to_u32string(basic_string<char32>& val)
    {
        return val;
    }

    /// @brief Hash specialization for owning null-terminated strings.
    template <typename HashType, typename CharT>
    struct hash<HashType, basic_string<CharT>>
    {
        /// @brief Hash result type.
        using ResultType = HashType;

        /// @brief Hashes logical characters without the terminator.
        /// @param str String value to hash.
        /// @return Hash value for the logical character range.
        static constexpr ResultType get_hash(const basic_string<CharT>& str) {
            return hash<HashType, const CharT*>::get_hash(str.begin(), str.end() - 1);
        }
    };

    /// @brief Hash specialization for fixed-capacity strings.
    template <typename HashType, typename CharT, usize N>
    struct hash<HashType, static_basic_string<CharT, N>>
    {
        /// @brief Hash result type.
        using ResultType = HashType;

        /// @brief Hashes logical characters without the terminator.
        /// @param str Static string value to hash.
        /// @return Hash value for the logical character range.
        static constexpr ResultType get_hash(const static_basic_string<CharT, N>& str) {
            return hash<HashType, const CharT*>::get_hash(str.begin(), str.end() - 1);
        }
    };

    /// @brief Narrow owning string alias.
    using string = basic_string<char>;
    /// @brief Wide owning string alias.
    using wstring = basic_string<wchar>;
    /// @brief UTF-8 owning string alias.
    using u8string = basic_string<char8>;
    /// @brief UTF-16 owning string alias.
    using u16string = basic_string<char16>;
    /// @brief UTF-32 owning string alias.
    using u32string = basic_string<char32>;
    
    /// @brief Narrow inline string alias.
    template <usize N>
    using static_string = static_basic_string<char, N>;
    
    /// @brief Wide inline string alias.
    template <usize N>
    using static_wstring = static_basic_string<wchar, N>;
    
    /// @brief UTF-8 inline string alias.
    template <usize N>
    using static_u8string = static_basic_string<char8, N>;
    
    /// @brief UTF-16 inline string alias.
    template <usize N>
    using static_u16string = static_basic_string<char16, N>;
    
    /// @brief UTF-32 inline string alias.
    template <usize N>
    using static_u32string = static_basic_string<char32, N>;

    namespace string_literals
    {
        /// @brief Constructs a narrow owning string from a literal token.
        /// @param str Literal character sequence.
        /// @param length Literal length excluding its terminator.
        /// @return Owning narrow string.
        static constexpr auto operator""_s(const char* str, usize length)
        {
            return string{str, length + 1};
        }

        /// @brief Constructs a UTF-8 owning string from a literal token.
        /// @param str Literal character sequence.
        /// @param length Literal length excluding its terminator.
        /// @return Owning UTF-8 string.
        static constexpr auto operator""_s(const char8* str, usize length)
        {
            return u8string{str, length + 1};
        }

        /// @brief Constructs a UTF-16 owning string from a literal token.
        /// @param str Literal character sequence.
        /// @param length Literal length excluding its terminator.
        /// @return Owning UTF-16 string.
        static constexpr auto operator""_s(const char16* str, usize length)
        {
            return u16string{str, length + 1};
        }

        /// @brief Constructs a UTF-32 owning string from a literal token.
        /// @param str Literal character sequence.
        /// @param length Literal length excluding its terminator.
        /// @return Owning UTF-32 string.
        static constexpr auto operator""_s(const char32* str, usize length)
        {
            return u32string{str, length + 1};
        }

        /// @brief Constructs a wide owning string from a literal token.
        /// @param str Literal character sequence.
        /// @param length Literal length excluding its terminator.
        /// @return Owning wide string.
        static constexpr auto operator""_s(const wchar* str, usize length)
        {
            return wstring{str, length + 1};
        }
    } // namespace string_literals

    namespace static_string_literals
    {
        /// @brief Returns a narrow structural static string literal.
          /// @return The compile-time literal value.
        template <static_string str>
        static constexpr auto operator""_s()
        {
            return str;
        }

        /// @brief Returns a UTF-8 structural static string literal.
          /// @return The compile-time literal value.
        template <static_u8string str>
        static constexpr auto operator""_s()
        {
            return str;
        }

        /// @brief Returns a UTF-16 structural static string literal.
          /// @return The compile-time literal value.
        template <static_u16string str>
        static constexpr auto operator""_s()
        {
            return str;
        }

        /// @brief Returns a UTF-32 structural static string literal.
          /// @return The compile-time literal value.
        template <static_u32string str>
        static constexpr auto operator""_s()
        {
            return str;
        }

        /// @brief Returns a wide structural static string literal.
          /// @return The compile-time literal value.
        template <static_wstring str>
        static constexpr auto operator""_s()
        {
            return str;
        }
    } // namespace static_string_literals
} // namespace hsd
