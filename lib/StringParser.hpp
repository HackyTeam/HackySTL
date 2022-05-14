#pragma once

#include "StringLiteral.hpp"
#include "StackArray.hpp"
#include "StringView.hpp"
#include "String.hpp"

namespace hsd
{
    template <typename CharT>
    class parser_value
    {
    private:
        using string_type = basic_string<CharT>;
        using string_view_type = basic_string_view<CharT>;

        string_view_type _view = "";

    public:
        constexpr parser_value() = default;

        constexpr parser_value(const CharT* str, usize length)
            : _view{str, length}
        {}

        constexpr parser_value(const parser_value&) = default;
        constexpr parser_value& operator=(const parser_value&) = default;

        constexpr auto to_i8() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i8>(_view.begin(), _view.end());
        }

        constexpr auto to_i16() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i16>(_view.begin(), _view.end());
        }

        constexpr auto to_i32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i32>(_view.begin(), _view.end());
        }

        constexpr auto to_i64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<i64>(_view.begin(), _view.end());
        }

        constexpr auto to_isize() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<isize>(_view.begin(), _view.end());
        }

        constexpr auto to_u8() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u8>(_view.begin(), _view.end());
        }

        constexpr auto to_u16() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u16>(_view.begin(), _view.end());
        }

        constexpr auto to_u32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u32>(_view.begin(), _view.end());
        }

        constexpr auto to_u64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<u64>(_view.begin(), _view.end());
        }

        constexpr auto to_usize() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<usize>(_view.begin(), _view.end());
        }

        constexpr auto to_f32() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f32>(_view.begin(), _view.end());
        }

        constexpr auto to_f64() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f64>(_view.begin(), _view.end());
        }

        constexpr auto to_f128() const
        {
            return hsd::basic_cstring<CharT>::template 
                parse<f128>(_view.begin(), _view.end());
        }

        inline auto to_string() const
        {
            return string_type{_view};
        }
    };

    template < basic_string_literal fmt, usize N >
    static constexpr auto parse_string(const auto* str)
    {
        class parser_error : public runtime_error
        {
        private:
            usize _index;
        
        public:
            constexpr parser_error(const char* err, usize index)
                : runtime_error{err}, _index{index}
            {}
            
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
