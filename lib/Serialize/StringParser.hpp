#pragma once

/// @file StringParser.hpp
/// @brief Compile-time-pattern string parsing into typed parser values.
/// @details Matches literal separators, captures fields, and exposes checked
/// integer, floating-point, string, and string-view conversions.

#include "StringLiteral.hpp"
#include "../Containers/StringView.hpp"
#include "../Containers/String.hpp"

namespace hsd
{
    template <typename CharT>
    /// @brief Non-owning view over a single captured field with typed conversions.
    /// @tparam CharT Character type of the captured field.
    /// @details Instances are produced by `parse_string`; they merely reference
    /// the caller-supplied buffer and must not outlive it.
    class parser_value
    {
    private:
        /// @brief Owning string type returned by `to_string`.
        using string_type = basic_string<CharT>;
        /// @brief Non-owning view type used for storage and `to_string_view`.
        using string_view_type = basic_string_view<CharT>;

        /// @brief View over the captured field; empty until constructed with data.
        string_view_type _view = "";

    public:
        /// @brief Constructs an empty, unbound field.
        constexpr parser_value() = default;

        /// @brief Binds the field to `length` characters starting at `str`.
        /// @param str Pointer to the first character of the field.
        /// @param length Number of characters in the field.
        constexpr parser_value(const CharT* str, usize length)
            : _view{str, length}
        {}

        /// @brief Copies the view from `other`; both refer to the same source characters.
        constexpr parser_value(const parser_value&) = default;
        /// @brief Copy-assigns the view from `other`.
        /// @return `*this` after the assignment.
        constexpr parser_value& operator=(const parser_value&) = default;

        /// @brief Parses the field as a signed 8-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_i8() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i8>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a signed 16-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_i16() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i16>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a signed 32-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_i32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i32>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a signed 64-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_i64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i64>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a pointer-sized signed integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_isize() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<isize>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as an unsigned 8-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_u8() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u8>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as an unsigned 16-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_u16() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u16>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as an unsigned 32-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_u32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u32>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as an unsigned 64-bit integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_u64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u64>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a pointer-sized unsigned integer.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_usize() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<usize>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a 32-bit floating-point value.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_f32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f32>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a 64-bit floating-point value.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_f64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f64>(_view.begin(), _view.end());
        }

        /// @brief Parses the field as a 128-bit floating-point value.
        /// @return The parsed value, or a parse error on invalid input.
        constexpr auto to_f128() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f128>(_view.begin(), _view.end());
        }

        /// @brief Copies the field into an owning string.
        /// @return A new string with the same contents as the field.
        inline auto to_string() const
        {
            return string_type{_view};
        }

        /// @brief Returns the field as a non-owning view.
        /// @return The underlying view; valid only as long as the source buffer is.
        inline auto to_string_view() const
        {
            return _view;
        }
    };

    /// @brief Matches `str` against the literal/placeholder pattern `fmt` and
    /// captures up to `N` placeholder fields.
    /// @tparam fmt Compile-time pattern; literal characters must match `str`
    /// exactly, and `{}` placeholders capture a field up to the next literal
    /// character (or the end of `str`).
    /// @tparam N Maximum number of captured fields.
    /// @param str Null-terminated subject string to match against `fmt`.
    /// @return A buffer of captured `parser_value` fields on success, or a
    /// `parser_error` (with the failing pattern index) when a literal
    /// character in `fmt` does not match `str`, or when a `{` is not closed
    /// by a matching `}`.
    /// @details The returned `parser_value` fields view directly into `str`;
    /// they must not outlive the buffer passed in.
    template < basic_string_literal fmt, usize N >
    static constexpr auto parse_string(const auto* str)
    {
        /// @brief Error reported when `fmt` does not match `str`.
        class parser_error : public runtime_error
        {
        private:
            /// @brief Index into `fmt` where the mismatch was detected.
            usize _index;
        
        public:
            /// @brief Constructs the error from `err` and the failing pattern index.
            constexpr parser_error(const char* err, usize index)
                : runtime_error{err}, _index{index}
            {}
            
            /// @brief Returns the index into `fmt` where the mismatch occurred.
            /// @return Zero-based index into the pattern.
            constexpr usize index() const
            {
                return _index;
            }
        };
        
        using char_type = typename decltype(fmt)::char_type;
        using val_type = parser_value<char_type>;
        using buf_type = static_vector<val_type, N>;
        using res_type = result<buf_type, parser_error>;
        
        buf_type _buf = {};
        bool _maybe_format = false, _valid_format = false;
        
        for (usize _index = 0; auto _chr : fmt)
        {
            if (_valid_format == false)
            {
                if (
                    (str[_index] == '\0' && _chr != '\0') || 
                    (_chr != str[_index] && _chr != '{' && _chr != '}')
                ){
                    return res_type {
                        parser_error {
                            "StringParser: invalid literal", _index
                        }
                    };
                }
                else if (_chr == '{')
                {
                    _maybe_format = true;
                }
                else if (_chr == '}' && _maybe_format == true)
                {
                    _maybe_format = false;
                    _valid_format = true;
                }
                else if (_chr != '}' && _maybe_format == true)
                {
                    return res_type {
                        parser_error {
                            "StringParser: invalid literal", _index
                        }
                    };
                }
                else
                {
                    _index++;
                }
            }

            else
            {
                usize _start_pos = _index;
                
                for (; str[_index] != _chr && str[_index] != '\0'; _index++)
                    ;

                _valid_format = false;
                _buf.push_back(val_type {
                    str + _start_pos, _index - _start_pos
                });

                if (str[_index] != '\0')
                    _index++;
            }
        }

        return res_type{_buf};
    }
} // namespace hsd
