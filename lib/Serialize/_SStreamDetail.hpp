#pragma once

/// @file _SStreamDetail.hpp
/// @brief Internal stream tokenization and typed parsing helpers.
/// @details Splits buffered text on separators and converts tokens into user
/// supplied values for SStream and Io.

#include "../Containers/Vector.hpp"
#include "../Containers/StringView.hpp"

namespace hsd::sstream_detail
{
    ///
    /// @brief Advances past any leading characters that appear in `sep`.
    ///
    /// @tparam CharT - Character type scanned.
    /// @param str - Null-terminated position to scan from.
    /// @param sep - Null-terminated set of separator characters.
    /// @return Pointer to the first character of `str` that is either the
    /// null terminator or not present in `sep`.
    ///
    ///
    /// @brief Tests whether `ch` is one of the separator characters.
    ///
    /// @tparam CharT - Character type compared.
    /// @param ch - Character to test.
    /// @param sep - Null-terminated set of separator characters.
    /// @return True when `ch` appears in `sep`.
    ///
    template <typename CharT>
    constexpr bool _is_sep(CharT ch, const CharT* sep)
    {
        for (; *sep != static_cast<CharT>('\0'); ++sep)
        {
            if (*sep == ch)
            {
                return true;
            }
        }
        return false;
    }

    ///
    /// @brief Advances past separator characters, stopping at `end`.
    ///
    /// @tparam CharT - Character type scanned.
    /// @param str - Position to scan from.
    /// @param end - One past the last readable character; never dereferenced at or after it.
    /// @param sep - Null-terminated set of separator characters.
    /// @return Pointer to the first character in `[str, end)` that is not a separator, or `end`.
    ///
    template <typename CharT>
    constexpr const CharT* _consume_sep(const CharT* str, const CharT* end, const CharT* sep)
    {
        while (str != end && _is_sep(*str, sep))
        {
            ++str;
        }

        return str;
    }

    ///
    /// @brief Advances to the next separator character, stopping at `end`.
    ///
    /// @tparam CharT - Character type scanned.
    /// @param str - Position to scan from.
    /// @param end - One past the last readable character; never dereferenced at or after it.
    /// @param sep - Null-terminated set of separator characters.
    /// @return Pointer to the first character in `[str, end)` that is a separator, or `end`.
    ///
    template <typename CharT>
    constexpr const CharT* _find_sep(const CharT* str, const CharT* end, const CharT* sep)
    {
        while (str != end && !_is_sep(*str, sep))
        {
            ++str;
        }

        return str;
    }

    ///
    /// @brief Splits a bounded buffer into up to `N` separator-delimited tokens.
    ///
    /// @tparam N - Maximum number of token views collected before the
    /// remainder is folded into the final token.
    /// @tparam CharT - Character type scanned.
    /// @param str - Buffer scanned for tokens; not required to be
    /// null-terminated within `str_size`.
    /// @param str_size - Number of valid characters available at `str`.
    /// @param sep - Null-terminated set of separator characters.
    /// @return A fixed-capacity vector of `basic_string_view` tokens with
    /// leading/repeated separators consumed; once `N - 1` tokens are
    /// collected the remaining text is appended as the final token.
    ///
    template <usize N, typename CharT>
    static inline auto split_data(const CharT* str, usize str_size, const CharT* sep)
    {
        static_vector<basic_string_view<CharT>, N> _buf;
        const CharT* _end = str + str_size;
        const CharT* _first = _consume_sep(str, _end, sep);

        while (_first != _end && _buf.size() < N - 1)
        {
            const CharT* _second = _find_sep(_first, _end, sep);
            _buf.emplace_back(_first, static_cast<usize>(_second - _first));
            _first = _consume_sep(_second, _end, sep);
        }

        if (_first != _end)
        {
            const CharT* _second = _find_sep(_first, _end, sep);
            _buf.emplace_back(_first, static_cast<usize>(_second - _first));
        }
        else if (str_size != 0 && _is_sep(str[str_size - 1], sep) && _buf.size() < N)
        {
            // A trailing separator completes the last field. The empty remainder
            // after it is reported as an extra token, so the field counts as complete.
            _buf.emplace_back(_end, usize{0});
        }

        return _buf;
    }

    ///
    /// @brief Parses an integral value from decimal or `0x`-prefixed hexadecimal text.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Integral destination type (excluding character types).
    /// @param from - Text to parse; a leading `-`/`+` sign is accepted for
    /// decimal input only.
    /// @param to - Destination updated with the parsed value on success.
    /// @return No error on success; a `runtime_error` when `from` is empty or
    /// contains a character outside the expected digit set.
    ///
    template <typename CharT, typename T>
    requires (IsIntegral<T> && !IsChar<T>)
    static constexpr option_err<runtime_error> _parse_from(const basic_string_view<CharT>& from, T& to)
    {
        if (from.length() == 0)
        {
            return runtime_error{"Empty input"};
        }

        if (from.length() > 2 && from[0] == '0' && (from[1] == 'x' || from[1] == 'x'))
        {
            to = 0;

            for (usize _idx = 2; _idx < from.length(); ++_idx)
            {
                to <<= 4;
                const CharT _chr = from[_idx];

                if (_chr >= '0' && _chr <= '9')
                {
                    to |= _chr - '0';
                }
                else if (_chr >= 'a' && _chr <= 'f')
                {
                    to |= 10 + (_chr - 'a');
                }
                else if (_chr >= 'A' && _chr <= 'F')
                {
                    to |= 10 + (_chr - 'A');
                }
                else
                {
                    return runtime_error{"Invalid hexadecimal input"};
                }
            }
        }
        else
        {
            remove_unsigned_t<T> _val = 0;

            bool _is_neg = from[0] == '-';
            auto _copy = from;

            if (from[0] == '-' || from[0] == '+')
            {
                _copy = {from.data() + 1, from.length() - 1};
            }

            for (const CharT _chr : _copy)
            {
                _val *= 10;

                if (_chr >= '0' && _chr <= '9')
                {
                    _val += _chr - '0';
                }
                else
                {
                    return runtime_error{"Invalid decimal input"};
                }
            }

            if (_is_neg == true)
            {
                _val = -_val;
            }

            to = _val;
        }

        return {};
    }

    ///
    /// @brief Parses a fixed-point floating value scanned from the least
    /// significant digit toward the most significant.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Floating-point destination type.
    /// @param from - Text to parse; a leading `-`/`+` sign is accepted and a
    /// single `.` marks the fractional boundary.
    /// @param to - Destination updated with the parsed value on success.
    /// @return No error on success; a `runtime_error` when `from` is empty or
    /// contains a character other than a digit, sign, or `.`.
    ///
    template <typename CharT, FloatingPointType T>
    static constexpr option_err<runtime_error> _parse_from_fixed(const basic_string_view<CharT>& from, T& to)
    {
        T _point = 0;
        T _round = 0;
        T _mul = 1;

        if (from.length() == 0)
        {
            return runtime_error{"Empty input"};
        }

        constexpr T _ten = 10;
        bool _has_point = false;
        const bool _is_negative = from[0] == '-';
        const bool _is_positive = from[0] == '+';

        // Walk from the last character back to the first, stopping before a sign.
        const usize _stop = (_is_positive || _is_negative) ? 1 : 0;

        for (usize _index = from.length(); _index != _stop; --_index)
        {
            const CharT _chr = from[_index - 1];

            if (_chr >= '0' && _chr <= '9')
            {
                if (_has_point == false)
                {
                    _point /= _ten;
                    _point += (_chr - '0') / _ten;
                }

                _round += (_chr - '0') * _mul;
                _mul *= _ten;
            }
            else if (_chr == '.' && _has_point == false)
            {
                _round = 0; _mul = 1;
                _has_point = true;
            }
            else
            {
                return runtime_error{"Invalid character"};
            }
        }

        to = (_has_point == true) ? _round + _point : _round;
        to = (_is_negative == true) ? -to : to;

        return {};
    }

    ///
    /// @brief Parses a floating value written as `<mantissa>e<exponent>` /
    /// `<mantissa>E<exponent>`.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Floating-point destination type.
    /// @param from - Full literal text, mantissa and exponent combined.
    /// @param exp_pos - Index of the `e`/`E` marker within `from`.
    /// @param to - Destination updated with the parsed value on success.
    /// @return No error on success; a `runtime_error` forwarded from parsing
    /// either the exponent or the mantissa on failure.
    ///
    template <typename CharT, FloatingPointType T>
    static constexpr option_err<runtime_error> _parse_from_exp(const basic_string_view<CharT>& from, usize exp_pos, T& to)
    {
        isize _exp = 0;
        constexpr T _ten = 10;
        
        auto _res = _parse_from<CharT>({from.data() + exp_pos + 1, from.length() - exp_pos - 1}, _exp);

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        _res = _parse_from_fixed<CharT>({from.data(), exp_pos}, to);
        
        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        } 
        
        to *= math::pow(_ten, static_cast<T>(_exp));
        return {};
    }

    ///
    /// @brief Parses a floating value written as a `0x`-prefixed hexadecimal
    /// literal with a mandatory binary exponent (`p`/`P`).
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Floating-point destination type.
    /// @param from - Text to parse, starting with the `0x`/`0X` prefix.
    /// @param to - Destination updated with the parsed value on success.
    /// @return No error on success; a `runtime_error` when a hexadecimal
    /// digit is invalid or the mandatory exponent marker is missing.
    ///
    template <typename CharT, FloatingPointType T>
    static constexpr option_err<runtime_error> _parse_from_hex(const basic_string_view<CharT>& from, T& to)
    {
        bool _has_point = false;
        usize _exp_pos = -1;
        isize _exp = 0;

        T _div = 0.0625;
        T _two = 2;
        
        to = 0;

        for (usize _idx = 2; _idx < from.length(); ++_idx)
        {
            const CharT _chr = from[_idx];

            if (_chr != '.' && _chr != 'p' && _chr != 'P')
            {
                to *= (_has_point == false) ? 16 : 1;
                _div *= (_has_point == true) ? 0.0625 : 1;
            }

            if (_chr >= '0' && _chr <= '9')
            {
                to += (_has_point == false) ? (_chr - '0') : (_chr - '0') * _div;
            }
            else if (_chr >= 'a' && _chr <= 'f')
            {
                to += (_has_point == false) ? (10 + _chr - 'a') : (10 + _chr - 'a') * _div;
            }
            else if (_chr >= 'A' && _chr <= 'F')
            {
                to += (_has_point == false) ? (10 + _chr - 'A') : (10 + _chr - 'F') * _div;
            }
            else if (_chr == '.')
            {
                _has_point = true;
            }
            else if (_chr == 'p' || _chr == 'P')
            {
                _exp_pos = _idx;
                break;
            }
            else
            {
                return runtime_error{"Invalid hexadecimal input"};
            }
        }

        if (_exp_pos == from.npos)
        {
            return runtime_error{"Hexadecimal float literal requires exponent"};
        }

        auto _res = _parse_from<CharT>({from.data() + _exp_pos + 1, from.length() - _exp_pos - 1}, _exp);

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        to *= math::pow(_two, static_cast<T>(_exp));
        return {};
    }

    ///
    /// @brief Parses a floating-point value, dispatching to the hexadecimal,
    /// exponential, or fixed-point parser based on the literal's form.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Floating-point destination type.
    /// @param from - Text to parse.
    /// @param to - Destination updated with the parsed value on success.
    /// @return No error on success; a `runtime_error` forwarded from whichever
    /// specialized parser handled the literal.
    ///
    template <typename CharT, FloatingPointType T>
    static constexpr option_err<runtime_error> _parse_from(const basic_string_view<CharT>& from, T& to)
    {
        constexpr const CharT _arr[] = {'e', 'E', '\0'};

        if (from.length() > 2 && from[0] == '0' && (from[1] == 'x' || from[1] == 'x'))
        {
            return _parse_from_hex<CharT>(from, to);
        }
        else 
        {
            usize _exp_pos = from.find_first_of(_arr);

            if (_exp_pos != from.npos)
            {
                return _parse_from_exp(from, _exp_pos, to);
            }
            else
            {
                return _parse_from_fixed(from, to);
            }
        }
    }

    ///
    /// @brief Parses a single character value, transcoding from `CharT` to `T`.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Character destination type.
    /// @param from - Text holding the source character(s) to transcode.
    /// @param to - Destination updated with the first transcoded character.
    /// @return No error on success; a `runtime_error` when `from` is empty or
    /// the transcoding fails.
    ///
    template <typename CharT, CharacterType T>
    static constexpr option_err<runtime_error> _parse_from(const basic_string_view<CharT>& from, T& to)
    {
        if (from.length() == 0)
        {
            return runtime_error{"Empty input"};
        }

        T _arr[2] = {};

        auto _res = unicode::convert_to(_arr, 2, from.data(), from.length());
        to = _arr[0];

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        return {};
    }

    ///
    /// @brief Parses a character string, transcoding from `CharT` into a
    /// fixed-size `T` buffer.
    ///
    /// @tparam CharT - Character type of the source view.
    /// @tparam T - Character destination type.
    /// @tparam N - Capacity of the destination array.
    /// @param from - Text holding the source characters to transcode.
    /// @param to - Destination array updated with the transcoded characters.
    /// @return No error on success; a `runtime_error` when `from` is empty or
    /// the transcoding fails.
    ///
    template <typename CharT, CharacterType T, usize N>
    static constexpr option_err<runtime_error> _parse_from(const basic_string_view<CharT>& from, T (&to)[N])
    {
        if (from.length() == 0)
        {
            return runtime_error{"Empty input"};
        }

        auto _res = unicode::convert_to(to, N, from.data(), from.length());

        if (_res.is_ok() == false)
        {
            return _res.unwrap_err();
        }

        return {};
    }
} // namespace hsd::sstream_detail
