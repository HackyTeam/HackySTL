#pragma once

#include "_IoOverload.hpp"
#include "Vector.hpp"
#include "CString.hpp"

namespace hsd
{
    namespace sstream_detail
    {
        template <usize N, typename CharT>
        inline auto split_data(const CharT* str)
        {
            static_vector< pair<const CharT*, usize>, N > _buf;
            const CharT* _iter_f = str;
            const CharT* _iter_s = basic_cstring<CharT>::find_or_end(_iter_f, ' ');

            while (*_iter_s != '\0')
            {
                _buf.emplace_back(_iter_f, static_cast<usize>(_iter_s - _iter_f));
                _iter_f = _iter_s + 1;
                _iter_s = basic_cstring<CharT>::find_or_end(_iter_f, ' ');
            }
            
            _buf.emplace_back(_iter_f, static_cast<usize>(_iter_s - _iter_f));
            return _buf;
        }

        template < basic_string_literal fmt, usize N >
        static constexpr auto parse_literal()
        {
            using char_type = typename decltype(fmt)::char_type;
            using info_type = format_info<char_type>;
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
                        if ((_tag & info_type::hex) || (_tag & info_type::exp))
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'x' or 'e' in format"
                                }
                            };
                        }

                        _tag |= info_type::hex;
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
                        if (_tag & info_type::hex || _tag & info_type::exp)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'x' or 'e' in format"
                                }
                            };
                        }

                        _tag |= info_type::exp;
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
                        if (_tag & info_type::fg)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'fg' in format"
                                }
                            };
                        }

                        _tag |= info_type::fg;
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
                        if (_tag & info_type::bg)
                        {
                            return res_type {
                                runtime_error {
                                    "Duplicate 'bg' in format"
                                }
                            };
                        }

                        _tag |= info_type::bg;
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
                            .tag = _tag,
                            .foreground = _fg,
                            .background = _bg,
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
                    .tag = _tag,
                    .foreground = _fg,
                    .background = _bg,
                }
            );

            return res_type{_buf};
        }
    
        template <>
        inline void _parse<char>(pair<const char*, usize>& str, char& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%c", &val);
            #else
            sscanf(str.first, "%c", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, uchar& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%c", &val);
            #else
            sscanf(str.first, "%c", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, i16& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%hd", &val);
            #else
            sscanf(str.first, "%hd", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, u16& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%hu", &val);
            #else
            sscanf(str.first, "%hu", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, i32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%d", &val);
            #else
            sscanf(str.first, "%d", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, u32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%u", &val);
            #else
            sscanf(str.first, "%u", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, long& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%zd", &val);
            #else
            sscanf(str.first, "%zd", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, ulong& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%zu", &val);
            #else
            sscanf(str.first, "%zu", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, i64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%lld", &val);
            #else
            sscanf(str.first, "%lld", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, u64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%llu", &val);
            #else
            sscanf(str.first, "%llu", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, f32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%f", &val);
            #else
            sscanf(str.first, "%f", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, f64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%lf", &val);
            #else
            sscanf(str.first, "%lf", &val);
            #endif
        }

        template <>
        inline void _parse<char>(pair<const char*, usize>& str, f128& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            sscanf_s(str.first, "%Lf", &val);
            #else
            sscanf(str.first, "%Lf", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, char& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%c", &val);
            #else
            swscanf(str.first, L"%c", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, uchar& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%c", &val);
            #else
            swscanf(str.first, L"%c", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, wchar& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%lc", &val);
            #else
            swscanf(str.first, L"%lc", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, i16& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%hd", &val);
            #else
            swscanf(str.first, L"%hd", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, u16& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%hu", &val);
            #else
            swscanf(str.first, L"%hu", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, i32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%d", &val);
            #else
            swscanf(str.first, L"%d", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, u32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%d", &val);
            #else
            swscanf(str.first, L"%d", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, long& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%zd", &val);
            #else
            swscanf(str.first, L"%zd", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, ulong& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%zu", &val);
            #else
            swscanf(str.first, L"%zu", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, i64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%lld", &val);
            #else
            swscanf(str.first, L"%lld", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, u64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%llu", &val);
            #else
            swscanf(str.first, L"%llu", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, f32& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%f", &val);
            #else
            swscanf(str.first, L"%f", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, f64& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%lf", &val);
            #else
            swscanf(str.first, L"%lf", &val);
            #endif
        }

        template <>
        inline void _parse<wchar>(pair<const wchar*, usize>& str, f128& val)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            swscanf_s(str.first, L"%Lf", &val);
            #else
            swscanf(str.first, L"%Lf", &val);
            #endif
        }

        inline auto _sub_from(i32& from, i32 amount)
            -> Result< void, runtime_error >
        {
            if (amount < 0)
            {
                return runtime_error{"Buffer too small"};
            }
            else
            {
                from -= amount;
            }

            return {};
        }

        inline auto _sub_from(usize& from, i32 amount)
            -> Result< void, runtime_error >
        {
            if (amount < 0)
            {
                return runtime_error{"Buffer too small"};
            }
            else
            {
                from -= static_cast<usize>(amount);
            }

            return {};
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));      
            return snprintf(dest.first, dest.second, fmt.format.data);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline void _write(bool val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));

            if (val == true)
            {
                HSD_SSTREAM_USE_FMT_UNTAGGED(fmt, "%s", dest, "true");
            }
            else
            {
                HSD_SSTREAM_USE_FMT_UNTAGGED(fmt, "%s", dest, "false");
            }
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(char val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%hhx", "%c", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(char8 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%hhx", "%zc", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(i16 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%hx", "%hd", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(u16 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%hx", "%hu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(i32 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%x", "%d", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(u32 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%x", "%u", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(i64 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%llx", "%lld", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(u64 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%llx", "%llu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(long val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%lx", "%ld", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(ulong val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT(fmt, fmt.hex, "0x%lx", "%lu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(f32 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_SSTREAM_USE_FMT(fmt, fmt.exp, "%e", "%f", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(f64 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_SSTREAM_USE_FMT(fmt, fmt.exp, "%le", "%lf", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(f128 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_SSTREAM_USE_FMT(fmt, fmt.exp, "%Le", "%Lf", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(const char* val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT_UNTAGGED(fmt, "%s", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(const char8* val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));
            HSD_SSTREAM_USE_FMT_UNTAGGED(fmt, "%s", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp || fmt.tag & fmt.hex));
            return swprintf(dest.first, dest.second, fmt.format.data);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline void _write(bool val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));

            if (val == true)
            {
                HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, L"%s", dest, "true");
            }
            else
            {
                HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, L"%s", dest, "false");
            }
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(char val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%c", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(char8 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%zc", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(wchar val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp || fmt.tag & fmt.hex));
            HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, L"%lc", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(i16 val, pair<char*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hd", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(u16 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(i32 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%x", L"%d", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(u32 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%x", L"%u", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(i64 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%lld", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(u64 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%llu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(long val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%ld", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(ulong val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%lu", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(f32 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.exp, L"%e", L"%f", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(f64 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.exp, L"%le", L"%lf", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(f128 val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex));
            HSD_WSSTREAM_USE_FMT(fmt, fmt.exp, L"%Le", L"%Lf", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(const char* val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, L"%s", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(const char8* val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, "%s", dest, val);
        }

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(const wchar* val, pair<wchar*, usize> dest)
        {
            static_assert(!(fmt.tag & fmt.hex || fmt.tag & fmt.exp));
            HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, L"%ls", dest, val);
        }
    } // namespace sstream_detail
} // namespace hsd
