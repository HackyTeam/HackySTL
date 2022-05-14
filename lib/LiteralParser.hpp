#pragma once

#include "StringLiteral.hpp"
#include "Vector.hpp"
#include "CString.hpp"
#include "Swtich.hpp"

namespace hsd
{
    template <typename CharT>
    struct fmt_style
    {
        template <usize N>
        using lit_type = basic_string_literal<CharT, N>;

        static constexpr lit_type<3> attr_base = {
            {'\x1b', '[', '\0'}
        };

        static constexpr lit_type<2> attr_sep = {
            {';', '\0'}
        };

        static constexpr lit_type<2> attr_end = {
            {'m', '\0'}
        };

        static constexpr lit_type<6> fg_base = {
            {'3', '8', ';', '5', ';', '\0'}
        };

        static constexpr lit_type<6> bg_base = {
            {'4', '8', ';', '5', ';', '\0'}
        };

        static constexpr lit_type<5> reset_attr = {
            {'\x1b', '[', '0', 'm', '\0'}
        };

        struct fmt_color
        {
            usize length = 0;
            CharT value[4] = {};
        };

        static constexpr usize bold   = 1;   // bold
        static constexpr usize dim    = 2;   // dim
        static constexpr usize italic = 4;   // italic
        static constexpr usize undrln = 8;   // underline
        static constexpr usize blink  = 16;  // blink
        static constexpr usize revrse = 32;  // reverse
        static constexpr usize hidden = 64;  // hidden
        static constexpr usize strike = 128; // striketrough
        static constexpr usize foregr = 256; // foreground
        static constexpr usize backgr = 512; // background
        static constexpr usize size   = 10;  // size

        usize tag = 0;
        fmt_color foreground = {};
        fmt_color background = {};
    };

    template <typename CharT>
    struct fmt_common
    {
        static constexpr usize none  = 0; 
        static constexpr usize hex   = 1;  // hexadecimal
        static constexpr usize exp   = 2;  // exponent
        static constexpr usize ovr   = 4;  // override
        static constexpr usize style = 8;  // style

        usize tag;
        fmt_style<CharT> style_val;
    };

    template <typename CharT>
    struct format_info
    {
        const CharT* format;
        usize length;

        fmt_common<CharT> base;
    };

    template <typename CharT, usize N>
    struct format_literal
    {
        using char_type = CharT;
        basic_string_literal<CharT, N> format;

        fmt_common<CharT> base;
    };

    namespace parse_lit_detail
    {
        struct switchable
        {
            u64 value[2] = {};

            constexpr bool operator==(const switchable& other) const
            {
                return value[0] == other.value[0] && value[1] == other.value[1];
            }
        };

        static constexpr void _check_duplicate_tag(
            usize tag, usize duplicate);

        template <typename CharT>
        static constexpr auto _find(const CharT* str, CharT letter);

        template <typename CharT>
        static constexpr auto _to_switchable(const CharT* str, usize sz)
            -> switchable;

        template <typename CharT, usize N>
        static constexpr auto _to_switchable(const CharT (&str)[N])
            -> switchable;

        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT* fmt, usize pos_current
        );

        template <typename CharT>
        static constexpr auto _parse_color(
            const CharT* color_fmt, usize& pos_current
        ) -> result<typename fmt_style<CharT>::fmt_color, runtime_error>;
    } // namespace parse_lit_detail

    template <basic_string_literal fmt, usize N>
    static constexpr auto parse_literal()
    {
        using char_type = typename decltype(fmt)::char_type;
        using cstring_type = basic_cstring<char_type>;
        using info_type = format_info<char_type>;
        using info_base_type = fmt_common<char_type>;
        using buf_type = static_vector<info_type, N>;
        using res_type = result<buf_type, runtime_error>;
        using style_type = fmt_style<char_type>;

        constexpr const char_type *_fmt_end = fmt.data + fmt.size() - 1;
        constexpr auto _npos = static_cast<usize>(-1);

        buf_type _buf;
        usize _tag = 0;

        style_type _style = {};
        const char_type* _fmt_ptr = fmt.data;

        usize _pos_current = parse_lit_detail::
            _find<char_type>(_fmt_ptr, '{');
        
        usize _curly_pos = _pos_current;

        while (_pos_current != _npos && _fmt_ptr[_pos_current] != '\0')
        {
            if (_pos_current != _npos && _fmt_ptr[_pos_current] != '}')
            {
                if (_fmt_ptr[_pos_current] == ',' && _fmt_ptr[_pos_current + 1] == '}')
                {
                    return res_type {
                        runtime_error {
                            "Empty token after ','"
                        }
                    };
                }

                _pos_current++;
                usize _pos_prev = _pos_current;

                while (cstring_type::is_letter(_fmt_ptr[_pos_current]))
                {
                    _pos_current++;
                }

                if (_pos_current == _pos_prev && _fmt_ptr[_pos_current] != '}')
                {
                    return res_type {
                        runtime_error {
                            "Invalid token"
                        }
                    };
                }

                auto _switch_token = parse_lit_detail::_to_switchable(
                    _fmt_ptr + _pos_prev, _pos_current - _pos_prev);

                switch_cases(
                    _switch_token,
                    EqualCase {
                        parse_lit_detail::_to_switchable(""), []{}
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("hex"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_tag, info_base_type::hex);
                            
                            _tag |= info_base_type::hex;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("exp"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_tag, info_base_type::exp);
                            
                            _tag |= info_base_type::exp;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("ovr"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_tag, info_base_type::ovr);
                            
                            _tag |= info_base_type::ovr;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("fg"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.foregr);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.foregr;
                            
                            _style.foreground = parse_lit_detail::
                                _parse_color(_fmt_ptr, _pos_current).unwrap();
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("bg"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.backgr);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.backgr;

                            _style.background = parse_lit_detail::
                                _parse_color(_fmt_ptr, _pos_current).unwrap();
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("bold"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.bold);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.bold;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("dim"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.dim);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.dim;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("italic"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.italic);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.italic;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("undrln"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.undrln);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.undrln;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("blink"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.blink);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.blink;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("rev"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.revrse);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.revrse;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("hid"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.hidden);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.hidden;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    EqualCase {
                        parse_lit_detail::_to_switchable("strike"), [&]
                        {
                            parse_lit_detail::
                                _check_duplicate_tag(_style.tag, _style.strike);
                            
                            _tag |= info_base_type::style;
                            _style.tag |= _style.strike;
                            
                            parse_lit_detail::
                                _check_following_fmt(_fmt_ptr, _pos_current);
                        }
                    },
                    DefaultCase{[]{ hsd::panic("Invalid format token"); }}
                );
            }
            else
            {
                _buf.emplace_back(
                    info_type {
                        .format = _fmt_ptr,
                        .length = _curly_pos,
                        .base = {
                            .tag = _tag,
                            .style_val = _style,
                        }
                    }
                );

                _fmt_ptr += _pos_current + 1;
                _pos_current = _curly_pos = parse_lit_detail::
                    _find<char_type>(_fmt_ptr, '{');
                
                _tag = 0;
                _style = style_type{};
            }
        }

        _buf.emplace_back(
            info_type {
                .format = _fmt_ptr,
                .length = static_cast<usize>(_fmt_end - _fmt_ptr),
                .base = {
                    .tag = _tag,
                    .style_val = _style,
                }
            }
        );

        return res_type{_buf};
    }

    namespace parse_lit_detail
    {
        static constexpr void _check_duplicate_tag(
            usize tag, usize duplicate)
        {
            if (tag & duplicate) hsd::panic("Duplicate tag");
        };

        template <typename CharT>
        static constexpr auto _find(const CharT *str, CharT letter)
        {
            const CharT *_start_ptr = str;

            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    return static_cast<usize>(str - _start_ptr);
            }

            return static_cast<usize>(-1);
        };

        template <typename CharT>
        static constexpr auto _to_switchable(const CharT *str, usize sz)
            -> switchable
        {
            switchable _result = {};

            for (usize _index = 0; _index < sz; _index++)
            {
                _result.value[_index % 2] |=
                    static_cast<u64>(str[_index]) << ((_index / 2) * 8);
            }

            return _result;
        };

        template <typename CharT, usize N>
        static constexpr auto _to_switchable(const CharT (&str)[N])
            -> switchable
        {
            return _to_switchable(str, N - 1);
        };

        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT *fmt, usize pos_current)
        {
            if (fmt[pos_current] != '}' && fmt[pos_current] != ',')
            {
                hsd::panic("Invalid format literal: must be followed by '}' or ','");
            }
        }

        template <typename CharT>
        static constexpr auto _parse_color(
            const CharT *color_fmt, usize &pos_current)
            -> result<typename fmt_style<CharT>::fmt_color, runtime_error>
        {
            using color_type = typename fmt_style<CharT>::fmt_color;

            if (color_fmt[pos_current] != '=')
            {
                return runtime_error{"Expected '=' after color format"};
            }

            pos_current++;

            if (color_fmt[pos_current] <= '9' && color_fmt[pos_current] >= '0')
            {
                auto _color = basic_cstring<CharT>::template 
                    parse<uchar>(color_fmt + pos_current);

                color_type _res = {
                    // Ternary operator always loves you :^)
                    .length = _color > 99 ? 3u : _color > 9 ? 2u : 1u,
                    .value = {}
                };

                for (usize _index = 0; _index < _res.length; _index++)
                {
                    _res.value[_res.length - _index - 1] = _color % 10 + '0';
                    _color /= 10;
                }

                pos_current += _res.length;

                return _res;
            }
            else
            {
                return runtime_error{"Expected color value after '='"};
            }
        }
    } // namespace parse_lit_detail
} // namespace hsd