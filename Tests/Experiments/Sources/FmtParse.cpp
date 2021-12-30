#include <StringLiteral.hpp>
#include <CString.hpp>
#include <Vector.hpp>
#include <Result.hpp>

template <typename CharT>
struct format_info
{
    const CharT *format;
    hsd::usize length;

    enum fmt_tag : hsd::uchar
    {
        none = 0,
        hex  = 1, // hexadecimal
        exp  = 2, // exponent
        fg   = 4, // foreground color
        bg   = 8, // background color
    };

    hsd::uchar tag;
    hsd::uchar foreground;
    hsd::uchar background;
};

template < hsd::basic_string_literal fmt, hsd::usize N >
static constexpr auto parse_literal()
{
    using char_type = typename decltype(fmt)::char_type;
    using info_type = format_info<char_type>;
    using buf_type = hsd::static_vector<info_type, N>;
    using res_type = hsd::Result<buf_type, hsd::runtime_error>;

    constexpr const char_type* _fmt_end = fmt.data + fmt.size();
    constexpr auto _npos = static_cast<hsd::usize>(-1);
    constexpr auto _find = [](const char_type* str, char_type letter)
    {
        const char_type *_start_ptr = str;

        for (; *str != '\0'; str++)
        {
            if (*str == letter)
                return static_cast<hsd::usize>(str - _start_ptr);
        }

        return static_cast<hsd::usize>(-1);
    };

    buf_type _buf;
    hsd::uchar _tag = 0, _fg = 0, _bg = 0;
    const char_type* _iter_f = fmt.data;
    hsd::usize _pos_s = _find(_iter_f, '{');

    while (_pos_s != _npos && *(_iter_f + _pos_s) != '\0')
    {
        if (_pos_s != _npos && *(_iter_f + _pos_s) != '}')
        {
            if (*(_iter_f + _pos_s) == ',' && *(_iter_f + _pos_s + 1) == '}')
            {
                return res_type {
                    hsd::runtime_error {
                        "Empty token after ','"
                    }
                };
            }

            _pos_s++;

            if (*(_iter_f + _pos_s) == 'x')
            {
                if ((_tag & info_type::hex) || (_tag & info_type::exp))
                {
                    return res_type {
                        hsd::runtime_error {
                            "Duplicate 'x' or 'e' in format"
                        }
                    };
                }

                _tag |= info_type::hex;
                _pos_s++;

                if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'x' must be followed by '}' or ','"
                        }
                    };
                }
            }
            else if (*(_iter_f + _pos_s) == 'e')
            {
                if (_tag & info_type::hex || _tag & info_type::exp)
                {
                    return res_type {
                        hsd::runtime_error {
                            "Duplicate 'x' or 'e' in format"
                        }
                    };
                }

                _tag |= info_type::exp;
                _pos_s++;

                if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'e' must be followed by '}' or ','"
                        }
                    };
                }
            }
            else if (*(_iter_f + _pos_s) == 'f' && *(_iter_f + _pos_s + 1) == 'g')
            {
                if (_tag & info_type::fg)
                {
                    return res_type {
                        hsd::runtime_error {
                            "Duplicate 'fg' in format"
                        }
                    };
                }

                _tag |= info_type::fg;
                _pos_s += 2;

                if (*(_iter_f + _pos_s) != '=')
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'fg' must be followed by '='"
                        }
                    };
                }

                _pos_s++;

                if (*(_iter_f + _pos_s) <= '9' && *(_iter_f + _pos_s) >= '0')
                {
                    _fg = hsd::basic_cstring<char_type>::template 
                        parse<hsd::uchar>(_iter_f + _pos_s);

                    // Ternary operator always loves you :^)
                    _pos_s += _fg > 99 ? 3 : _fg > 9 ? 2 : 1;

                    if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                    {
                        return res_type {
                            hsd::runtime_error {
                                "Invalid format: 'fg=num' must be followed by '}' or ','"
                            }
                        };
                    }
                }
                else
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'fg' must be followed by a number"
                        }
                    };
                }
            }
            else if (*(_iter_f + _pos_s) == 'b' && *(_iter_f + _pos_s + 1) == 'g')
            {
                if (_tag & info_type::bg)
                {
                    return res_type {
                        hsd::runtime_error {
                            "Duplicate 'bg' in format"
                        }
                    };
                }

                _tag |= info_type::bg;
                _pos_s += 2;

                if (*(_iter_f + _pos_s) != '=')
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'bg' must be followed by '='"
                        }
                    };
                }

                _pos_s++;

                if (*(_iter_f + _pos_s) <= '9' && *(_iter_f + _pos_s) >= '0')
                {
                    _bg = hsd::basic_cstring<char_type>::template 
                        parse<hsd::uchar>(_iter_f + _pos_s);

                    // Ternary operator always loves you :^)
                    _pos_s += _bg > 99 ? 3 : _bg > 9 ? 2 : 1;

                    if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                    {
                        return res_type {
                            hsd::runtime_error {
                                "Invalid format: 'bg=num' must be followed by '}' or ','"
                            }
                        };
                    }
                }
                else
                {
                    return res_type {
                        hsd::runtime_error {
                            "Invalid format: 'bg' must be followed by a number"
                        }
                    };
                }
            }
            else if (*(_iter_f + _pos_s) != '}')
            {
                return res_type {
                    hsd::runtime_error {
                        "Invalid format: '{' must be followed by 'x', 'e', 'f' or 'b'"
                    }
                };
            }
        }
        else
        {
            _buf.emplace_back(
                info_type {
                    .format = _iter_f,
                    .length = _pos_s,
                    .tag = _tag,
                    .foreground = _fg,
                    .background = _bg,
                }
            );

            _iter_f += _pos_s + 1;
            _pos_s = _find(_iter_f, '{');
            _tag = _fg = _bg = 0;
        }
    }

    _buf.emplace_back(
        info_type {
            .format = _iter_f,
            .length = static_cast<hsd::usize>(_fmt_end - _iter_f),
            .tag = _tag,
            .foreground = _fg,
            .background = _bg,
        }
    );

    return res_type{_buf};
}

template <hsd::basic_string_literal fmt, typename... Args>
inline hsd::Result<void, hsd::runtime_error> print(Args&&...)
{
    auto _fmt_buf = parse_literal<
        fmt, sizeof...(Args) + 1>().unwrap();

    assert(
        _fmt_buf.size() == sizeof...(Args) + 1
    );

    return {};
}

int main()
{
    print<"this is a test {x,fg=123,bg=42}, now go">(123).unwrap();
    print<"this is a test {x,bg=42}, now go">(123).unwrap();
    print<"this is a test {x,fg=123}, now go">(123).unwrap();
    print<"this is a test {x}, now go">(123).unwrap();
    print<"this is a test {fg=123,bg=42}, now go">(123).unwrap();
    print<"this is a test {fg=123}, now go">(123).unwrap();
    print<"this is a test {bg=42}, now go">(123).unwrap();
    print<"this is a test {}, now go">(123).unwrap();
}