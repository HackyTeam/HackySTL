#pragma once

#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdexcept>

#include "SStream.hpp"
#include "Tuple.hpp"

namespace hsd
{
    namespace io_detail
    {
        class bufferable
        {
        protected:
            static inline u8sstream _u8io_buf{4096};
            static inline wsstream _wio_buf{4096};
        };

        template<typename CharT, usize N>
        class format_literal
        {
        public:
            CharT data[N];

            constexpr format_literal(const CharT (&str)[N])
            {
                copy_n(str, N, data);
            }

            constexpr usize size() const
            {
                return N;
            }
        };

        static vector< u8string > split(const char* str, usize size)
        {
            vector<u8string> _buf;
            const char* _iter_f = str;
            const char* _iter_s = u8cstring::find(_iter_f, '{');

            if(_iter_s != nullptr && *(_iter_s + 1) != '}')
            {
                throw std::runtime_error("invalid character after \'{\'");
            }
            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf.emplace_back(_iter_f, _iter_s - _iter_f);
                _iter_f = _iter_s + 2;
                _iter_s = u8cstring::find(_iter_f, '{');

                if(_iter_s != nullptr && *(_iter_s + 1) != '}')
                    throw std::runtime_error("invalid character after \'{\'");
            }
            
            _buf.emplace_back(_iter_f, (str + size) - _iter_f);
            return _buf;
        }

        static vector< wstring > split(const wchar* str, usize size)
        {
            vector<wstring> _buf;
            const wchar_t* _iter_f = str;
            const wchar_t* _iter_s = wcstring::find(_iter_f, '{');

            if(_iter_s != nullptr && *(_iter_s + 1) != '}')
            {
                throw std::runtime_error("invalid character after \'{\'");
            }
            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf.emplace_back(_iter_f, _iter_s - _iter_f);
                _iter_f = _iter_s + 2;
                _iter_s = wcstring::find(_iter_f, '{');

                if(_iter_s != nullptr && *(_iter_s + 1) != '}')
                    throw std::runtime_error("invalid character after \'{\'");
            }
            
            _buf.emplace_back(_iter_f, (str + size) - _iter_f);
            return _buf;
        }

        static void _print(u8string& str, char val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.c_str(), val);
        }

        static void _print(u8string& str, uchar val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.c_str(), val);
        }

        static void _print(u8string& str, i16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hd", str.c_str(), val);
        }

        static void _print(u8string& str, u16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hu", str.c_str(), val);
        }

        static void _print(u8string& str, i32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%d", str.c_str(), val);
        }

        static void _print(u8string& str, u32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%u", str.c_str(), val);
        }

        static void _print(u8string& str, i64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%lld", str.c_str(), val);
        }

        static void _print(u8string& str, u64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%llu", str.c_str(), val);
        }

        static void _print(u8string& str, isize val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%zd", str.c_str(), val);
        }

        static void _print(u8string& str, usize val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%zu", str.c_str(), val);
        }

        static void _print(u8string& str, f32 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%f", str.c_str(), val);
            }
        }

        static void _print(u8string& str, f64 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%lf", str.c_str(), val);
            }
        }

        static void _print(u8string& str, f128 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%Le", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%Lf", str.c_str(), val);
            }
        }

        static void _print(u8string& str, char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val);
        }

        static void _print(u8string& str, const char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val);
        }

        static void _print(u8string& str, u8string& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val.c_str());
        }
        
        template<typename... Args>
        static void _print(u8string& str, tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.c_str());
            fprintf(file_buf, "(");
            u8string _before_fmt = "";
            u8string _after_fmt = ", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        template<typename... Args>
        static void _print(u8string& str, const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.c_str());
            fprintf(file_buf, "(");
            u8string _before_fmt = "";
            u8string _after_fmt = ", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        static void _print(wstring& str, wchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lc", str.c_str(), val);
        }

        static void _print(wstring& str, char val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.c_str(), val);
        }

        static void _print(wstring& str, uchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.c_str(), val);
        }

        static void _print(wstring& str, i16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hd", str.c_str(), val);
        }

        static void _print(wstring& str, u16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hu", str.c_str(), val);
        }

        static void _print(wstring& str, i32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%d", str.c_str(), val);
        }

        static void _print(wstring& str, u32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%u", str.c_str(), val);
        }

        static void _print(wstring& str, i64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lld", str.c_str(), val);
        }

        static void _print(wstring& str, u64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%llu", str.c_str(), val);
        }

        static void _print(wstring& str, isize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zd", str.c_str(), val);
        }

        static void _print(wstring& str, usize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zu", str.c_str(), val);
        }

        static void _print(wstring& str, f32 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%ls%e", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%f", str.c_str(), val);
            }
        }

        static void _print(wstring& str, f64 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"l%s%e", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%lf", str.c_str(), val);
            }
        }

        static void _print(wstring& str, f128 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%s%Le", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%s%Lf", str.c_str(), val);
            }
        }

        static void _print(wstring& str, char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val);
        }

        static void _print(wstring& str, const char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val);
        }

        static void _print(wstring& str, wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val);
        }

        static void _print(wstring& str, const wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val);
        }

        static void _print(wstring& str, u8string& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val.c_str());
        }

        static void _print(wstring& str, wstring& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val.c_str());
        }

        template<typename... Args>
        static void _print(wstring& str, tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls", str.c_str());
            fwprintf(file_buf, L"(");
            wstring _before_fmt = L"";
            wstring _after_fmt = L", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }

        template<typename... Args>
        static void _print(wstring& str, const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls", str.c_str());
            fwprintf(file_buf, L"(");
            wstring _before_fmt = L"";
            wstring _after_fmt = L", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }
    }
} // namespace hsd
