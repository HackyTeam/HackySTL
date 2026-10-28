#pragma once

/// @file CString.hpp
/// @brief Character-type-independent string primitives and parsers.
/// @details Supplies bounded copying, comparison, search, whitespace, numeric
/// parsing, and character conversion operations used by strings and parsers.

#include "Allocator.hpp"
#include "../Containers/StackArray.hpp"

namespace hsd
{	
    template <typename CharT>
    /// @brief Static operations for null-terminated character sequences.
    /// @tparam CharT Character type processed by the operation.
    class basic_cstring
    {
    private:
        ///
        /// @brief Tests whether `substr` fully matches starting at `str`.
        ///
        /// @param str - Position in the haystack to test.
        /// @param substr - Null-terminated needle compared at `str`.
        /// @return `true` when every character of `substr` matches, `false`
        /// otherwise (including when `str` ends before `substr` does).
        ///
        static constexpr bool _compare(const CharT* str, const CharT* substr)
        {
            for (; *str && *substr; str++, substr++)
            {
                if (*str != *substr)
                    return false;
            }

            return (*substr == '\0');
        }

        ///
        /// @brief Gets the absolute value of a signed numeric type.
        ///
        /// @tparam T - Numeric type of `num`.
        /// @param num - Value whose magnitude is returned.
        /// @return `num` when non-negative, `-num` otherwise.
        ///
        template <typename T>
        static constexpr T _modulus(T num)
        {
            if (num < 0)
            {
                return -num;
            }

            return num;
        }

        ///
        /// @brief Counts the base-10 digits needed to print `num`.
        ///
        /// @tparam T - Numeric type of `num`.
        /// @param num - Value whose decimal digit count is measured.
        /// @return The digit count, at least 1 (for `num == 0`).
        ///
        template <typename T>
        static constexpr usize _num_len(T num)
        {
            usize _len = 0;
            
            if (num == 0)
            {
                return 1;
            }
            while (num)
            {
                _len++;
                num /= 10;
            }

            return _len;
        }

        ///
        /// @brief Tests whether `letter` is an ASCII decimal digit.
        ///
        /// @param letter - Character to test.
        /// @return `true` if `letter` is in `['0', '9']`, `false` otherwise.
        ///
        static constexpr bool _is_number(CharT letter)
        {
            return letter >= '0' && letter <= '9';
        }

        ///
        /// @brief Builds the null-terminated table backing `whitespace_chars`.
        ///
        /// Wide character types (16 bits or larger) get the full Unicode
        /// whitespace set; narrower types get only the ASCII/Latin-1 subset.
        ///
        /// @return A `stack_array` of whitespace code points ending in `0`.
        ///
        static constexpr auto _get_whitespace_chars()
        {
            if constexpr (sizeof(CharT) >= sizeof(char16))
            {
                return stack_array{
                    static_cast<CharT>(0x09),
                    static_cast<CharT>(0x0A),
                    static_cast<CharT>(0x0B),
                    static_cast<CharT>(0x0C),
                    static_cast<CharT>(0x0D),
                    static_cast<CharT>(0x20),
                    static_cast<CharT>(0x85),
                    static_cast<CharT>(0xA0),
                    static_cast<CharT>(0x1680),
                    static_cast<CharT>(0x180E),
                    static_cast<CharT>(0x2000),
                    static_cast<CharT>(0x2001),
                    static_cast<CharT>(0x2002),
                    static_cast<CharT>(0x2003),
                    static_cast<CharT>(0x2004),
                    static_cast<CharT>(0x2005),
                    static_cast<CharT>(0x2006),
                    static_cast<CharT>(0x2007),
                    static_cast<CharT>(0x2008),
                    static_cast<CharT>(0x2009),
                    static_cast<CharT>(0x200A),
                    static_cast<CharT>(0x2028),
                    static_cast<CharT>(0x2029),
                    static_cast<CharT>(0x202F),
                    static_cast<CharT>(0x205F),
                    static_cast<CharT>(0x3000),
                    static_cast<CharT>(0) // Terminator
                };
            } 
            else 
            {
                return stack_array{
                    static_cast<CharT>(0x09),
                    static_cast<CharT>(0x0A),
                    static_cast<CharT>(0x0B),
                    static_cast<CharT>(0x0C),
                    static_cast<CharT>(0x0D),
                    static_cast<CharT>(0x20),
                    static_cast<CharT>(0x85),
                    static_cast<CharT>(0xA0),
                    static_cast<CharT>(0) // Terminator
                };
            }
        }
    public:
        /// @brief Storage backing `whitespace_chars`, built once per `CharT`.
        static constexpr const auto whitespace_chars_arr = _get_whitespace_chars();
        /// @brief Null-terminated table of whitespace code points for `CharT`.
        static constexpr const CharT* whitespace_chars = whitespace_chars_arr.data();

        ///
        /// @brief Tests whether `ch` is one of the recognized whitespace code points.
        ///
        /// @param ch - Character to test.
        /// @return `true` if `ch` is whitespace, `false` otherwise.
        ///
        static constexpr bool iswhitespace(CharT ch)
        {
            return find(whitespace_chars, ch) != nullptr;
        }

        ///
        /// @brief Finds the first occurrence of a null-terminated substring.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param substr - Null-terminated needle to find.
        /// @return A pointer into `str` at the first match, or `nullptr` when
        /// `substr` does not occur.
        ///
        static constexpr const CharT* find(const CharT* str, const CharT* substr)
        {
            for (; *str != '\0'; str++)
            {
                if ((*str == *substr) && _compare(str, substr))
                    return str;
            }

            return nullptr;
        }

        ///
        /// @brief Finds the first occurrence of a single character.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param letter - Character to find.
        /// @return A pointer into `str` at the first match, or `nullptr` when
        /// `letter` does not occur.
        ///
        static constexpr const CharT* find(const CharT* str, CharT letter)
        {
            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    return str;
            }

            return nullptr;
        }

        ///
        /// @brief Finds the first occurrence of a null-terminated substring.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param substr - Null-terminated needle to find.
        /// @return A pointer into `str` at the first match, or a pointer to
        /// the terminating `'\0'` of `str` when `substr` does not occur.
        ///
        static constexpr const CharT* find_or_end(const CharT* str, const CharT* substr)
        {
            for (; *str != '\0'; str++)
            {
                if ((*str == *substr) && _compare(str, substr))
                    return str;
            }

            return str;
        }

        ///
        /// @brief Finds the first occurrence of a single character.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param letter - Character to find.
        /// @return A pointer into `str` at the first match, or a pointer to
        /// the terminating `'\0'` of `str` when `letter` does not occur.
        ///
        static constexpr const CharT* find_or_end(const CharT* str, CharT letter)
        {
            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    return str;
            }

            return str;
        }

        ///
        /// @brief Finds the first occurrence of a character in a bounded range.
        ///
        /// @details At run time on GCC and Clang, one-byte character types are
        /// searched with a single `__builtin_memchr` call. Otherwise blocks
        /// without the character are skipped in one pass. The inner loop has no
        /// early exit, so the compiler can vectorize it.
        ///
        /// @param first - Start of the range to search.
        /// @param last - One past the final character searched.
        /// @param letter - Character to find.
        /// @return A pointer to the first match, or `last` when `letter` does
        /// not occur in `[first, last)`.
        ///
        static constexpr const CharT* find_or_end(
            const CharT* first, const CharT* last, CharT letter)
        {
            #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
            if constexpr (sizeof(CharT) == 1)
            {
                if (!is_constant_evaluated() && first != last)
                {
                    const void* _hit = __builtin_memchr(
                        first, static_cast<int>(letter),
                        static_cast<usize>(last - first)
                    );

                    return _hit == nullptr
                        ? last : static_cast<const CharT*>(_hit);
                }
            }
            #endif

            constexpr usize _block = 32;

            for (; static_cast<usize>(last - first) >= _block; first += _block)
            {
                bool _hit = false;

                for (usize _offset = 0; _offset < _block; _offset++)
                {
                    _hit = _hit | (first[_offset] == letter);
                }

                if (_hit)
                {
                    break;
                }
            }

            for (; first != last; ++first)
            {
                if (*first == letter)
                {
                    break;
                }
            }

            return first;
        }

        ///
        /// @brief Finds the last occurrence of a null-terminated substring.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param substr - Null-terminated needle to find.
        /// @return A pointer into `str` at the last match, or `nullptr` when
        /// `substr` does not occur.
        ///
        static constexpr const CharT* find_rev(const CharT* str, const CharT* substr)
        {
            const CharT* rez = nullptr;

            for (; *str != '\0'; str++)
            {
                if ((*str == *substr) && _compare(str, substr))
                    rez = str;
            }

            return rez;
        }

        ///
        /// @brief Finds the last occurrence of a single character.
        ///
        /// @param str - Null-terminated haystack to search.
        /// @param letter - Character to find.
        /// @return A pointer into `str` at the last match, or `nullptr` when
        /// `letter` does not occur.
        ///
        static constexpr const CharT* find_rev(const CharT* str, CharT letter)
        {
            const CharT* rez = nullptr;

            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    rez = str;
            }

            return rez;
        }

        ///
        /// @brief Searches backward for a substring starting below index `pos`.
        ///
        /// The scan visits indices `pos - 1` down to `1`; index `0` is never
        /// inspected, so a match only at `str[0]` is not found.
        ///
        /// @param str - Haystack indexed by `pos`.
        /// @param substr - Null-terminated needle to find.
        /// @param pos - One past the highest index to inspect.
        /// @return A pointer into `str` at the first match found scanning
        /// backward, or `nullptr` when none occurs in range.
        ///
        static constexpr const CharT* find_rev(const CharT* str, const CharT* substr, usize pos)
        {
            for (; pos != 0; pos--)
            {
                if ((str[pos] == *substr) && _compare(&str[pos], substr))
                    return &str[pos];
            }

            return nullptr;
        }

        ///
        /// @brief Searches backward for a character starting below index `pos`.
        ///
        /// The scan visits indices `pos - 1` down to `1`; index `0` is never
        /// inspected, so a match only at `str[0]` is not found.
        ///
        /// @param str - Haystack indexed by `pos`.
        /// @param letter - Character to find.
        /// @param pos - One past the highest index to inspect.
        /// @return A pointer into `str` at the first match found scanning
        /// backward, or `nullptr` when none occurs in range.
        ///
        static constexpr const CharT* find_rev(const CharT* str, CharT letter, usize pos)
        {
            for (; pos != 0; pos--)
            {
                if (str[pos] == letter)
                    return &str[pos];
            }

            return nullptr;
        }

        ///
        /// @brief Counts the characters before the terminating `'\0'`.
        ///
        /// @param str - Null-terminated string to measure.
        /// @return The number of characters before the terminator.
        ///
        static constexpr usize length(const CharT* str)
        {
            usize _iter;

            for (_iter = 0; str[_iter] != '\0'; _iter++)
                ;

            return _iter;
        }

        ///
        /// @brief Converts ASCII lowercase letters in `str` to uppercase, in place.
        ///
        /// @param str - Null-terminated string modified in place.
        /// @return `str`, for chaining.
        ///
        static constexpr const CharT* upper(CharT* str)
        requires (is_same<CharT, char>::value)
        {
            usize _iter;

            for (_iter = 0; str[_iter] != '\0'; _iter++)
            {
                if (str[_iter] >= 'a' && str[_iter] <= 'z')
                    str[_iter] -= 32;
            }

            return str;
        }

        ///
        /// @brief Converts ASCII uppercase letters in `str` to lowercase, in place.
        ///
        /// @param str - Null-terminated string modified in place.
        /// @return `str`, for chaining.
        ///
        static constexpr const CharT* lower(CharT* str)
        requires (is_same<CharT, char>::value)
        {
            usize _iter;

            for (_iter = 0; str[_iter] != '\0'; _iter++)
            {
                if (str[_iter] >= 'A' && str[_iter] <= 'Z')
                    str[_iter] += 32;
            }

            return str;
        }

        ///
        /// @brief Tests whether `ch` is an ASCII letter.
        ///
        /// Only covers `[a-z]`/`[A-Z]`; other Unicode letters are not detected.
        ///
        /// @param ch - Character to test.
        /// @return `true` if `ch` is an ASCII letter, `false` otherwise.
        ///
        static constexpr bool is_letter(CharT ch)
        {
            // Incomplete, there are a lot of letters that this case doesn't cover
            return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
        }

        ///
        /// @brief Builds a new, reversed copy of `str` in freshly allocated storage.
        ///
        /// @param str - Source characters to reverse; need not be null-terminated
        /// when `size` is supplied.
        /// @param size - Number of characters to reverse, or `0` to reverse up
        /// to the terminating `'\0'` of `str`.
        /// @return A newly allocated, null-terminated buffer holding the
        /// reversed characters. Allocated through `mallocator`; the caller owns
        /// the result and must release it with `mallocator::deallocate` (or
        /// `free`). Aborts via `.unwrap()` if allocation fails.
        ///
        static inline CharT* reverse(const CharT* str, usize size = 0)
        {
            usize _begin = 0;
            usize _end = size - 1;
            CharT* _buf = nullptr;

            if (size == 0)
            {
                _end = length(str);
                
                if(_end != 0)
                    _end--;
            }
            
            _buf = mallocator::allocate<CharT>(_end + 2).unwrap();
            copy(str, str + _end + 1, _buf);

            for (; _begin <= _end; _begin++, _end--)
                swap(_buf[_begin], _buf[_end]);

            return _buf;
        }

        ///
        /// @brief Reverses `str` in place without allocating.
        ///
        /// @param str - Buffer whose characters are reversed in place.
        /// @param size - Number of characters to reverse, or `0` to reverse up
        /// to the terminating `'\0'` of `str`.
        ///
        static constexpr void reverse(CharT*& str, usize size = 0)
        {
            usize _begin = 0;
            usize _end = size - 1;

            if (size == 0)
            {
                _end = length(str);
                if(_end != 0){ _end--; }
            }
            for (; _begin <= _end; _begin++, _end--)
            {
                hsd::swap(str[_begin], str[_end]);
            }
        }

        ///
        /// @brief Parses a signed integer from a null-terminated string.
        ///
        /// Skips leading non-digit characters, tracking the last-seen `-` as
        /// the sign, then accumulates consecutive digits. Malformed or empty
        /// input yields `0`; no overflow checking is performed.
        ///
        /// @tparam T - Signed integral destination type.
        /// @param str - Null-terminated string to parse.
        /// @return The parsed value.
        ///
        template <typename T>
        static constexpr T parse(const CharT* str)
        requires (is_integral<T>::value && !is_unsigned<T>::value)
        {
            T _num = 0;
            bool _negative = false;

            for (; *str != '\0' && !_is_number(*str); str++)
            {
                _negative = (*str == static_cast<CharT>('-'));
            }

            for (; *str != '\0' && _is_number(*str); str++)
            {
                _num *= 10;
                _num += *str - '0';
            }
            
            return _negative ? -_num : _num;
        }

        ///
        /// @brief Parses an unsigned integer from a null-terminated string.
        ///
        /// Skips leading non-digit characters, then accumulates consecutive
        /// digits. Malformed or empty input yields `0`; no overflow checking
        /// is performed, and a leading `-` is ignored rather than rejected.
        ///
        /// @tparam T - Unsigned integral destination type.
        /// @param str - Null-terminated string to parse.
        /// @return The parsed value.
        ///
        template <typename T> requires (is_unsigned<T>::value)
        static constexpr T parse(const CharT* str)
        {
            T _num = 0;
            
            for (; *str != '\0' && !_is_number(*str); str++)
                ;

            for (; *str != '\0' && _is_number(*str); str++)
            {
                _num *= 10;
                _num += static_cast<T>(*str - '0');
            }

            return _num;
        }

        ///
        /// @brief Parses a floating-point number from a null-terminated string.
        ///
        /// Skips leading non-digit characters, tracking the last-seen `-` as
        /// the sign, accumulates the integral part up to an optional `.`, then
        /// accumulates the fractional part. Malformed or empty input yields
        /// `0`; no overflow or precision-loss checking is performed.
        ///
        /// @tparam T - Floating-point destination type.
        /// @param str - Null-terminated string to parse.
        /// @return The parsed value.
        ///
        template <typename T> requires (is_floating_point<T>::value)
        static constexpr T parse(const CharT* str)
        {
            T _round_num = 0;
            T _point_num = 0;
            
            bool _negative = false;
            usize _num_len = 0;

            for (; *str != '\0' && !_is_number(*str); str++)
            {
                _negative = (*str == static_cast<CharT>('-'));
            }
            
            if (*str == '\0')
            {
                return static_cast<T>(0);
            }

            for (; *str != '\0' && *str != static_cast<CharT>('.') && _is_number(*str); str++)
            {
                _round_num *= 10;
                _round_num += *str - static_cast<CharT>('0');
            }
            
            if (*str == '\0' || (*str != static_cast<CharT>('.') && !_is_number(*str)))
            {
                return _negative ? -_round_num : _round_num;
            }

            for (str += 1; *str != '\0' && _is_number(*str); str++, _num_len++)
            {
                _point_num *= 10;
                _point_num += *str - '0';
            }
            
            for (; _num_len != 0; _num_len--, _point_num *= static_cast<T>(0.1))
                ;
            
            return _negative ? -(_round_num + _point_num) : _round_num + _point_num;
        }

        ///
        /// @brief Parses a signed integer from a bounded character range.
        ///
        /// Same digit/sign handling as `parse(const CharT*)`, but stops at
        /// `end` instead of a null terminator.
        ///
        /// @tparam T - Signed integral destination type.
        /// @param str - Start of the range to parse.
        /// @param end - One past the last character to inspect.
        /// @return The parsed value.
        ///
        template <typename T>
        static constexpr T parse(const CharT* str, const CharT* end)
        requires (is_integral<T>::value && !is_unsigned<T>::value)
        {
            T _num = 0;
            bool _negative = false;

            for (; str != end && !_is_number(*str); str++)
            {
                _negative = (*str == static_cast<CharT>('-'));
            }

            for (; str != end && _is_number(*str); str++)
            {
                _num *= 10;
                _num += *str - '0';
            }
            
            return _negative ? -_num : _num;
        }

        ///
        /// @brief Parses an unsigned integer from a bounded character range.
        ///
        /// Same digit handling as `parse(const CharT*)`, but stops at `end`
        /// instead of a null terminator.
        ///
        /// @tparam T - Unsigned integral destination type.
        /// @param str - Start of the range to parse.
        /// @param end - One past the last character to inspect.
        /// @return The parsed value.
        ///
        template <typename T> requires (is_unsigned<T>::value)
        static constexpr T parse(const CharT* str, const CharT* end)
        {
            T _num = 0;
            
            for (; str != end && !_is_number(*str); str++)
                ;

            for (; str != end && _is_number(*str); str++)
            {
                _num *= 10;
                _num += *str - '0';
            }

            return _num;
        }

        ///
        /// @brief Parses a floating-point number from a bounded character range.
        ///
        /// Same integral/fractional handling as `parse(const CharT*)`, but
        /// stops at `end` instead of a null terminator.
        ///
        /// @tparam T - Floating-point destination type.
        /// @param str - Start of the range to parse.
        /// @param end - One past the last character to inspect.
        /// @return The parsed value.
        ///
        template <typename T> requires (is_floating_point<T>::value)
        static constexpr T parse(const CharT* str, const CharT* end)
        {
            T _round_num = 0;
            T _point_num = 0;
            
            bool _negative = false;
            usize _num_len = 0;

            for (; str != end && !_is_number(*str); str++)
            {
                _negative = (*str == static_cast<CharT>('-'));
            }
            
            if (str == end)
            {
                return static_cast<T>(0);
            }

            for (; str != end && *str != static_cast<CharT>('.') && _is_number(*str); str++)
            {
                _round_num *= 10;
                _round_num += *str - static_cast<CharT>('0');
            }
            
            if (str == end || (*str != static_cast<CharT>('.') && !_is_number(*str)))
            {
                return _negative ? -_round_num : _round_num;
            }

            for (str += 1; str != end && _is_number(*str); str++, _num_len++)
            {
                _point_num *= 10;
                _point_num += *str - '0';
            }
            
            for (; _num_len != 0; _num_len--, _point_num *= static_cast<T>(0.1))
                ;
            
            return _negative ? -(_round_num + _point_num) : _round_num + _point_num;
        }

        ///
        /// @brief Orders two characters for comparison.
        ///
        /// @details Single-byte characters compare as unsigned values, as `strcmp`
        /// and `std::string_view` do, so bytes above `0x7f` sort after ASCII whatever
        /// the signedness of `char`. Wider characters compare by value.
        ///
        /// @param lhs - Left-hand character.
        /// @param rhs - Right-hand character.
        /// @return `-1` when `lhs < rhs`, `1` when `lhs > rhs`, `0` when equal.
        ///
        static constexpr i32 order(CharT lhs, CharT rhs)
        {
            if constexpr (sizeof(CharT) == 1)
            {
                const unsigned char _left = static_cast<unsigned char>(lhs);
                const unsigned char _right = static_cast<unsigned char>(rhs);

                return _left < _right ? -1 : (_left > _right ? 1 : 0);
            }
            else
            {
                return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0);
            }
        }

        ///
/// @brief Lexicographically compares two null-terminated strings.
        ///
        /// @param lhs - Left-hand string.
        /// @param rhs - Right-hand string.
        /// @return `-1` when `lhs < rhs`, `1` when `lhs > rhs`, `0` when equal.
        ///
        static constexpr i32 compare(const CharT* lhs, const CharT* rhs)
        {
            usize _index = 0;

            for (; lhs[_index] && rhs[_index]; _index++)
            {
                if (lhs[_index] != rhs[_index])
                {
                    return order(lhs[_index], rhs[_index]);
                }
            }
            
            return order(lhs[_index], rhs[_index]);
        }

        ///
        /// @brief Lexicographically compares up to `len` characters of two strings.
        ///
        /// Comparison also stops early at the first null terminator in either
        /// string, matching `compare(const CharT*, const CharT*)` semantics
        /// bounded to at most `len` characters.
        ///
        /// At run time on GCC and Clang, equal ranges are confirmed with one
        /// `__builtin_memcmp` call before the scan.
        ///
        /// @param lhs - Left-hand string.
        /// @param rhs - Right-hand string.
        /// @param len - Maximum number of characters to compare.
        /// @return `-1` when `lhs < rhs`, `1` when `lhs > rhs`, `0` when equal
        /// within the compared prefix.
        ///
        static constexpr i32 compare(const CharT* lhs, const CharT* rhs, usize len)
        {
            #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
            // Equal bytes are equal characters, so an equal range compares as 0.
            if (!is_constant_evaluated() && len != 0
                && __builtin_memcmp(lhs, rhs, len * sizeof(CharT)) == 0)
            {
                return 0;
            }
            #endif

            constexpr usize _block = 32;
            usize _index = 0;

            // Blocks that are equal and hold no terminator are skipped whole.
            // The inner loop has no early exit, so the compiler can vectorize it.
            for (; _index + _block <= len; _index += _block)
            {
                bool _stop = false;

                for (usize _offset = 0; _offset < _block; _offset++)
                {
                    _stop = _stop | (lhs[_index + _offset] != rhs[_index + _offset])
                        | (lhs[_index + _offset] == 0);
                }

                if (_stop)
                {
                    break;
                }
            }

            // Exact scan of the remaining characters, or of the first block
            // that differs or ends a string.
            for (; _index < len; _index++)
            {
                if (lhs[_index] != rhs[_index])
                {
                    return order(lhs[_index], rhs[_index]);
                }

                if (lhs[_index] == 0)
                {
                    return 0;
                }
            }

            return 0;
        }

        ///
        /// @brief Copies up to `len` characters of `src` into `dest`.
        ///
        /// Copying also stops early at the terminating `'\0'` of `src`. When
        /// exactly `len` non-null characters are copied (`src` was at least
        /// that long), `dest` is left without a written terminator; the
        /// caller must otherwise ensure termination in that case.
        ///
        /// @param dest - Destination buffer with room for at least `len + 1`
        /// characters.
        /// @param src - Null-terminated source string.
        /// @param len - Maximum number of characters to copy.
        /// @return `nullptr` when `dest` is `nullptr`; otherwise a pointer one
        /// past the last character written to `dest`.
        ///
        static constexpr CharT* copy(CharT* dest, const CharT* src, usize len)
        {
            if (dest == nullptr)
                return nullptr;

            for (; *src && len != 0; len--, dest++, src++)
                *dest = *src;

            if (len != 0)
                *dest = *src;

            return dest;
        }

        ///
        /// @brief Copies `src`, including its terminator, into `dest`.
        ///
        /// @param dest - Destination buffer, large enough to hold `src`.
        /// @param src - Null-terminated source string.
        /// @return `nullptr` when `dest` is `nullptr`; otherwise `dest`.
        ///
        static constexpr CharT* copy(CharT* dest, const CharT* src)
        {
            if (dest == nullptr)
                return nullptr;

            CharT* ptr = dest;

            for (; *src; dest++, src++)
                *dest = *src;

            *dest = *src;
            return ptr;
        }

        ///
        /// @brief Copies `src` into `dest` up to (excluding) the first `letter`.
        ///
        /// @param dest - Destination buffer, null-terminated on return.
        /// @param src - Null-terminated source string.
        /// @param letter - Character at which copying stops.
        /// @return `nullptr` when `dest` is `nullptr`; otherwise `dest`.
        ///
        static constexpr CharT* copy_until(CharT* dest, const CharT* src, CharT letter)
        {
            if (dest == nullptr)
                return nullptr;

            CharT* ptr = dest;

            for(; *src != '\0' && *src != letter; dest++, src++)
                *dest = *src;
            
            *dest = '\0';
            return ptr;
        }

        ///
        /// @brief Appends `src` to the end of the null-terminated string `dest`.
        ///
        /// @param dest - Destination string, must have room for `src`'s
        /// characters plus a terminator past its current length.
        /// @param src - Null-terminated string to append.
        /// @return `dest`.
        ///
        static constexpr CharT* add(CharT* dest, const CharT* src)
        {
            usize _index = length(dest);
            usize _index_helper = 0;

            for (; src[_index_helper] != '\0'; _index_helper++)
            {
                dest[_index + _index_helper] = src[_index_helper];
            }

            dest[_index + _index_helper] = '\0';
            return dest;
        }

        ///
        /// @brief Appends `src` to `dest` starting at index `len`.
        ///
        /// Unlike `add(CharT*, const CharT*)`, the append position is supplied
        /// by the caller instead of being computed from `dest`'s length.
        ///
        /// @param dest - Destination buffer with room for `src`'s characters
        /// plus a terminator past index `len`.
        /// @param src - Null-terminated string to append.
        /// @param len - Index in `dest` at which appending begins.
        /// @return `dest`.
        ///
        static constexpr CharT* add(CharT* dest, const CharT* src, usize len)
        {
            usize _index = len;
            usize _index_helper = 0;

            for (; src[_index_helper] != '\0'; _index_helper++)
            {
                dest[_index + _index_helper] = src[_index_helper];
            }

            dest[_index + _index_helper] = '\0';
            return dest;
        }
    };

    /// @brief `basic_cstring` specialized for `char`.
    using cstring = basic_cstring<char>;
    /// @brief `basic_cstring` specialized for `wchar`.
    using wcstring = basic_cstring<wchar>;
    /// @brief `basic_cstring` specialized for `char8`.
    using u8cstring = basic_cstring<char8>;
    /// @brief `basic_cstring` specialized for `char16`.
    using u16cstring = basic_cstring<char16>;
    /// @brief `basic_cstring` specialized for `char32`.
    using u32cstring = basic_cstring<char32>;
}