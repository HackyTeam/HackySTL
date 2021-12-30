#pragma once

#include "_IoOverload.hpp"
#include "Vector.hpp"
#include "CString.hpp"

namespace hsd
{
    namespace sstream_detail
    {
        template <usize N, typename CharT>
        inline static_vector<const CharT*, N> split_data(const CharT* str, const CharT* sep)
        {
            static_vector<const CharT*, N> _buf;
            const CharT* _iter_f = str;

            auto _find_and_consume_sep = [](const CharT* str, const CharT* sep)
            {
                while (*str != '\0')
                {
                    if (*basic_cstring<CharT>::find_or_end(sep, *str) == *str)
                    {
                        break;
                    }

                    str++;
                }

                while (*str != '\0')
                {
                    if (*basic_cstring<CharT>::find_or_end(sep, *str) != *str)
                    {
                        break;
                    }

                    str++;
                }

                return str;  
            };

            for (; *_iter_f == ' '; _iter_f++)
                ;

            const CharT* _iter_s = _find_and_consume_sep(_iter_f, sep);

            while (*_iter_s != '\0')
            {
                for (; *_iter_s == ' '; _iter_s++)
                    ;

                _buf.emplace_back(_iter_f);
                _iter_f = _iter_s;
                _iter_s = _find_and_consume_sep(_iter_f, sep);
            }
            
            _buf.emplace_back(_iter_f);
            return _buf;
        }

        template < basic_string_literal fmt, usize N >
        static constexpr auto parse_literal()
        {
            using char_type = typename decltype(fmt)::char_type;
            using info_type = format_info<char_type>;
            using info_base_type = fmt_common<char_type>;
            using buf_type = static_vector<info_type, N>;
            using res_type = Result<buf_type, runtime_error>;

            constexpr const char_type* _fmt_end = fmt.data + fmt.size();
            constexpr auto _npos = static_cast<usize>(-1);
            constexpr auto _find = [](const char_type* str, char_type letter)
            {
                const char_type *_start_ptr = str;

                for (; *str != '\0'; str++)
                {
                    if (*str == letter)
                        return static_cast<usize>(str - _start_ptr);
                }

                return static_cast<usize>(-1);
            };

            buf_type _buf;
            uchar _tag = 0, _fg = 0, _bg = 0;
            const char_type* _iter_f = fmt.data;
            usize _pos_s = _find(_iter_f, '{');
            usize _curly_pos = _pos_s; 

            while (_pos_s != _npos && *(_iter_f + _pos_s) != '\0')
            {
                if (_pos_s != _npos && *(_iter_f + _pos_s) != '}')
                {
                    if (*(_iter_f + _pos_s) == ',' && *(_iter_f + _pos_s + 1) == '}')
                    {
                        return res_type {
                            runtime_error {
                                "Empty token after ','"
                            }
                        };
                    }

                    _pos_s++;

                    if (*(_iter_f + _pos_s) == 'x')
                    {
                        if ((_tag & info_base_type::hex) || (_tag & info_base_type::exp))
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'x' or 'e' in format"
                                }
                            };
                        }

                        _tag |= info_base_type::hex;
                        _pos_s++;

                        if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'x' must be followed by '}' or ','"
                                }
                            };
                        }
                    }
                    else if (*(_iter_f + _pos_s) == 'e')
                    {
                        if (_tag & info_base_type::hex || _tag & info_base_type::exp)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'x' or 'e' in format"
                                }
                            };
                        }

                        _tag |= info_base_type::exp;
                        _pos_s++;

                        if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'e' must be followed by '}' or ','"
                                }
                            };
                        }
                    }
                    else if (*(_iter_f + _pos_s) == 'f' && *(_iter_f + _pos_s + 1) == 'g')
                    {
                        if (_tag & info_base_type::fg)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'fg' in format"
                                }
                            };
                        }

                        _tag |= info_base_type::fg;
                        _pos_s += 2;

                        if (*(_iter_f + _pos_s) != '=')
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'fg' must be followed by '='"
                                }
                            };
                        }

                        _pos_s++;

                        if (*(_iter_f + _pos_s) <= '9' && *(_iter_f + _pos_s) >= '0')
                        {
                            _fg = basic_cstring<char_type>::template 
                                parse<uchar>(_iter_f + _pos_s);

                            // Ternary operator always loves you :^)
                            _pos_s += _fg > 99 ? 3 : _fg > 9 ? 2 : 1;

                            if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                            {
                                return res_type {
                                    runtime_error {
                                        "Invalid format: 'fg=num' must be followed by '}' or ','"
                                    }
                                };
                            }
                        }
                        else
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'fg' must be followed by a number"
                                }
                            };
                        }
                    }
                    else if (*(_iter_f + _pos_s) == 'b' && *(_iter_f + _pos_s + 1) == 'g')
                    {
                        if (_tag & info_base_type::bg)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'bg' in format"
                                }
                            };
                        }

                        _tag |= info_base_type::bg;
                        _pos_s += 2;

                        if (*(_iter_f + _pos_s) != '=')
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'bg' must be followed by '='"
                                }
                            };
                        }

                        _pos_s++;

                        if (*(_iter_f + _pos_s) <= '9' && *(_iter_f + _pos_s) >= '0')
                        {
                            _bg = basic_cstring<char_type>::template 
                                parse<uchar>(_iter_f + _pos_s);

                            // Ternary operator always loves you :^)
                            _pos_s += _bg > 99 ? 3 : _bg > 9 ? 2 : 1;

                            if (*(_iter_f + _pos_s) != '}' && *(_iter_f + _pos_s) != ',')
                            {
                                return res_type {
                                    runtime_error {
                                        "Invalid format: 'bg=num' must be followed by '}' or ','"
                                    }
                                };
                            }
                        }
                        else
                        {
                            return res_type {
                                runtime_error {
                                    "Invalid format: 'bg' must be followed by a number"
                                }
                            };
                        }
                    }
                    else if (*(_iter_f + _pos_s) != '}')
                    {
                        return res_type {
                            runtime_error {
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
                            .length = _curly_pos,
                            .base = {
                                .tag = _tag,
                                .foreground = _fg,
                                .background = _bg
                            }
                        }
                    );

                    _iter_f += _pos_s + 1;
                    _pos_s = _curly_pos = _find(_iter_f, '{');
                    _tag = _fg = _bg = 0;
                }
            }

            _buf.emplace_back(
                info_type {
                    .format = _iter_f,
                    .length = static_cast<usize>(_fmt_end - _iter_f),
                    .base = {
                        .tag = _tag,
                        .foreground = _fg,
                        .background = _bg
                    }
                }
            );

            return res_type{_buf};
        }

        template <typename>
        class stream_parser;

        template <>
        class stream_parser<char>
        {
        private:
            pair<const char**, usize> _data;
            usize _index = 0;

        public:
            inline stream_parser(pair<const char**, usize> data)
                : _data{data}
            {}

            const auto& data() const
            {
                return _data;
            }

            auto& index()
            {
                return _index;
            }

            friend auto _parse_impl(stream_parser& p, char& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                val = static_cast<char>(*p._data.first[p._index++]);
                return {};
            }

            friend auto _parse_impl(stream_parser& p, uchar& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                val = static_cast<uchar>(*p._data.first[p._index++]);
                return {};
            }

            friend auto _parse_impl(stream_parser& p, i16& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%hd", &val);
                #else
                sscanf(p._data.first[p._index++], "%hd", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, i32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%d", &val);
                #else
                sscanf(p._data.first[p._index++], "%d", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, long& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%ld", &val);
                #else
                sscanf(p._data.first[p._index++], "%ld", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, i64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%lld", &val);
                #else
                sscanf(p._data.first[p._index++], "%lld", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u16& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%hu", &val);
                #else
                sscanf(p._data.first[p._index++], "%hu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%u", &val);
                #else
                sscanf(p._data.first[p._index++], "%u", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, ulong& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%lu", &val);
                #else
                sscanf(p._data.first[p._index++], "%lu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%llu", &val);
                #else
                sscanf(p._data.first[p._index++], "%llu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%f", &val);
                #else
                sscanf(p._data.first[p._index++], "%f", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%lf", &val);
                #else
                sscanf(p._data.first[p._index++], "%lf", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f128& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                sscanf_s(p._data.first[p._index++], "%Lf", &val);
                #else
                sscanf(p._data.first[p._index++], "%Lf", &val);
                #endif

                return {};
            }
        };

        template <>
        class stream_parser<wchar>
        {
        private:
            pair<const wchar**, usize> _data;
            usize _index = 0;

        public:
            inline stream_parser(pair<const wchar**, usize> data)
                : _data{data}
            {}

            const auto& data() const
            {
                return _data;
            }

            auto& index()
            {
                return _index;
            }

            friend auto _parse_impl(stream_parser& p, char& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                val = static_cast<char>(*p._data.first[p._index++]);
                return {};
            }

            friend auto _parse_impl(stream_parser& p, uchar& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                val = static_cast<uchar>(*p._data.first[p._index++]);
                return {};
            }

            friend auto _parse_impl(stream_parser& p, wchar& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                val = static_cast<wchar>(*p._data.first[p._index++]);
                return {};
            }

            friend auto _parse_impl(stream_parser& p, i16& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%hd", &val);
                #else
                swscanf(p._data.first[p._index++], L"%hd", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, i32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%d", &val);
                #else
                swscanf(p._data.first[p._index++], L"%d", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, long& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%ld", &val);
                #else
                swscanf(p._data.first[p._index++], L"%ld", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, i64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%lld", &val);
                #else
                swscanf(p._data.first[p._index++], L"%lld", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u16& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%hu", &val);
                #else
                swscanf(p._data.first[p._index++], L"%hu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%u", &val);
                #else
                swscanf(p._data.first[p._index++], L"%u", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, ulong& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%lu", &val);
                #else
                swscanf(p._data.first[p._index++], L"%lu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, u64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%llu", &val);
                #else
                swscanf(p._data.first[p._index++], L"%llu", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f32& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%f", &val);
                #else
                swscanf(p._data.first[p._index++], L"%f", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f64& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%lf", &val);
                #else
                swscanf(p._data.first[p._index++], L"%lf", &val);
                #endif

                return {};
            }

            friend auto _parse_impl(stream_parser& p, f128& val)
                -> Result<void, runtime_error>
            {
                if (p._index >= p._data.second)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                swscanf_s(p._data.first[p._index++], L"%Lf", &val);
                #else
                swscanf(p._data.first[p._index++], L"%Lf", &val);
                #endif

                return {};
            }
        };

        template <auto>
        class stream_writer;

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        class stream_writer<fmt>
        {
        private:
            char* _data;
            usize _size;
            usize _index;

        public:
            inline stream_writer(char* data, usize size, usize index)
                : _data{data}, _size{size}, _index{index}
            {}

            inline auto size() const
            {
                return _size;
            }

            inline auto* data()
            {
                return _data;
            }

            inline auto& index()
            {
                return _index;
            }

            friend Result<void, runtime_error> _write_impl(stream_writer& p)
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                u32 _write_len = snprintf(
                    p._data + p._index, p._size - p._index, fmt.format.data
                );
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, bool val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT_UNTAGGED(
                        fmt, "%s", _dest_pair, val == true ? "true" : "false"
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, char val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%hhx", "%c", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, char8 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%hhx", "%zc", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i16 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%hx", "%hd", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u16 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%hx", "%hu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%x", "%d", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%x", "%u", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%llx", "%lld", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%llx", "%llu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, long val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%lx", "%ld", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, ulong val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.hex, "0x%lx", "%lu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.exp, "%e", "%f", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.exp, "%le", "%lf", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f128 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT(
                        fmt, fmt.base.exp, "%Le", "%Lf", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, const char* val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT_UNTAGGED(
                        fmt, "%s", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {};
            }

            friend auto _write_impl(stream_writer& p, const char8* val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_SSTREAM_USE_FMT_UNTAGGED(
                        fmt, "%s", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {};
            }
        };

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        class stream_writer<fmt>
        {
        private:
            wchar* _data;
            usize _size;
            usize _index;

        public:
            inline stream_writer(wchar* data, usize size, usize index)
                : _data{data}, _size{size}, _index{index}
            {}

            inline auto size() const
            {
                return _size;
            }

            inline auto* data()
            {
                return _data;
            }

            inline auto& index()
            {
                return _index;
            }

            friend Result<void, runtime_error> _write_impl(stream_writer& p)
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                u32 _write_len = swprintf(
                    p._data + p._index, p._size - p._index, fmt.format.data
                );
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, bool val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT_UNTAGGED(
                        fmt, L"%s", _dest_pair, val == true ? "true" : "false"
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, char val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%hhx", L"%c", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, char8 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%hhx", L"%zc", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i16 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%hx", L"%hd", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u16 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%hx", L"%hu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%x", L"%d", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%x", L"%u", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, i64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%llx", L"%lld", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, u64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%llx", L"%llu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, long val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%lx", L"%ld", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, ulong val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.exp))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.hex, L"0x%lx", L"%lu", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f32 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.exp, L"%e", L"%f", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f64 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.exp, L"%le", L"%lf", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, f128 val)
                -> Result<void, runtime_error>
            requires (!(fmt.base.tag & fmt.base.hex))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT(
                        fmt, fmt.base.exp, L"%Le", L"%Lf", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {}; 
            }

            friend auto _write_impl(stream_writer& p, const char* val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT_UNTAGGED(
                        fmt, L"%s", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {};
            }

            friend auto _write_impl(stream_writer& p, const char8* val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT_UNTAGGED(
                        fmt, L"%s", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {};
            }

            friend auto _write_impl(stream_writer& p, const wchar* val)
                -> Result<void, runtime_error>
            requires (!((fmt.base.tag & fmt.base.hex) || (fmt.base.tag & fmt.base.exp)))
            {
                if (p._index >= p._size)
                {
                    return runtime_error {
                        "Invalid format: unexpected end of format string"
                    };
                }

                pair _dest_pair = {p._data + p._index, p._size - p._index};
                u32 _write_len = [&]{
                    HSD_WSSTREAM_USE_FMT_UNTAGGED(
                        fmt, L"%ls", _dest_pair, val
                    );
                }();
                
                p._index += _write_len;
                return {};
            }
        };
    } // namespace sstream_detail
} // namespace hsd
