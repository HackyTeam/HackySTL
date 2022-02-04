#pragma once

#include "LiteralParser.hpp"
namespace hsd
{
    namespace sstream_detail
    {
        template <usize N, typename CharT>
        inline auto split_data(const CharT* str, const CharT* sep)
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
    } // namespace sstream_detail
} // namespace hsd
