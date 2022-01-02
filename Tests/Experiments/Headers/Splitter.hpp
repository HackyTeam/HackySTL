#pragma once

#include <_IoOverload.hpp>
#include <Vector.hpp>
#include <CString.hpp>
#include <Swtich.hpp>

namespace hsd
{
    namespace sstream_detail
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
        static constexpr auto _to_switchable(const CharT(&str)[N])
            -> switchable;

        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT* fmt, usize pos_current);

        template <typename CharT>
        static constexpr Result<uchar, runtime_error> _parse_color(
            const CharT* color_fmt, usize& pos_current
        );

        template < basic_string_literal fmt, usize N >
        static constexpr auto parse_literal()
        {
            using char_type = typename decltype(fmt)::char_type;
            using cstring_type = basic_cstring<char_type>;
            using info_type = format_info<char_type>;
            using info_base_type = fmt_common<char_type>;
            using buf_type = static_vector<info_type, N>;
            using res_type = Result<buf_type, runtime_error>;

            constexpr const char_type* _fmt_end = fmt.data + fmt.size();
            constexpr auto _npos = static_cast<usize>(-1);

            buf_type _buf;
            usize _tag = 0;

            uchar _fg = 0, _bg = 0;
            const char_type* _fmt_ptr = fmt.data;

            usize _pos_current = _find<char_type>(_fmt_ptr, '{');
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

                    auto _switch_token = _to_switchable(
                        _fmt_ptr + _pos_prev, _pos_current - _pos_prev
                    );

                    switch_cases(
                        _switch_token,
                        EqualCase{
                            _to_switchable("x"), [&] 
                            {
                                _check_duplicate_tag(_tag, info_base_type::hex);
                                _tag |= info_base_type::hex;
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("hex"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::hex);
                                _tag |= info_base_type::hex;
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("e"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::exp);
                                _tag |= info_base_type::exp;
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("exp"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::exp);
                                _tag |= info_base_type::exp;
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("fg"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::fg);
                                _tag |= info_base_type::fg;
                                _fg = _parse_color(_fmt_ptr, _pos_current).unwrap();
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("foreground"), [&] 
                            {
                                _check_duplicate_tag(_tag, info_base_type::fg);
                                _tag |= info_base_type::fg;
                                _fg = _parse_color(_fmt_ptr, _pos_current).unwrap();
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("bg"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::bg);
                                _tag |= info_base_type::bg;
                                _bg = _parse_color(_fmt_ptr, _pos_current).unwrap();
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        EqualCase{
                            _to_switchable("background"), [&]
                            {
                                _check_duplicate_tag(_tag, info_base_type::bg);
                                _tag |= info_base_type::bg;
                                _bg = _parse_color(_fmt_ptr, _pos_current).unwrap();
                                _check_following_fmt(_fmt_ptr, _pos_current);
                            }
                        },
                        DefaultCase{
                            []{ hsd_panic("Invalid format token"); }
                        }
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
                                .foreground = _fg,
                                .background = _bg
                            }
                        }
                    );

                    _fmt_ptr += _pos_current + 1;
                    _pos_current = _curly_pos = _find(_fmt_ptr, '{');
                    _tag = _fg = _bg = 0;
                }
            }

            _buf.emplace_back(
                info_type {
                    .format = _fmt_ptr,
                    .length = static_cast<usize>(_fmt_end - _fmt_ptr),
                    .base = {
                        .tag = _tag,
                        .foreground = _fg,
                        .background = _bg
                    }
                }
            );

            return res_type{_buf};
        }

        static constexpr void _check_duplicate_tag(
            usize tag, usize duplicate)
        {
            if (tag & duplicate) hsd_panic("Duplicate tag");
        };

        template <typename CharT>
        static constexpr auto _find(const CharT* str, CharT letter)
        {
            const CharT* _start_ptr = str;

            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    return static_cast<usize>(str - _start_ptr);
            }

            return static_cast<usize>(-1);
        };

        template <typename CharT>
        static constexpr auto _to_switchable(const CharT* str, usize sz)
            -> switchable
        {
            switchable _result = {};

            for (usize i = 0; i < sz; i++)
            {
                _result.value[i % 2] |= 
                    static_cast<u64>(str[i]) << ((i / 2) * 8);
            }

            return _result;
        };

        template <typename CharT, usize N>
        static constexpr auto _to_switchable(const CharT(&str)[N])
            -> switchable
        {
            return _to_switchable(str, N - 1);
        };

        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT* fmt, usize pos_current)
        {
            if (fmt[pos_current] != '}' && fmt[pos_current] != ',')
            {
                hsd_panic("Invalid format literal: must be followed by '}' or ','");
            }
        }

        template <typename CharT>
        static constexpr Result<uchar, runtime_error> _parse_color(
            const CharT* color_fmt, usize& pos_current)
        {
            if (color_fmt[pos_current] != '=')
            {
                return runtime_error{"Expected '=' after color format"};
            }

            pos_current++;

            if (color_fmt[pos_current] <= '9' && color_fmt[pos_current] >= '0')
            {
                uchar _color = basic_cstring<CharT>::template 
                    parse<uchar>(color_fmt + pos_current);

                // Ternary operator always loves you :^)
                pos_current += _color > 99 ? 3 : _color > 9 ? 2 : 1;

                return _color;
            }
            else
            {
                return runtime_error{"Expected color value after '='"};
            }
        }
    } // namespace sstream_detail
} // namespace hsd