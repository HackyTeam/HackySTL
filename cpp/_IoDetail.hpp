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
            static inline hsd::u8sstream _u8io_buf{4096};
            static inline hsd::wsstream _wio_buf{4096};
        };

        // Although it is in an internal namespace,
        // this class will be used for having custom
        // printing support like std::ofstream (cout)
        struct printable
        {
            virtual void print() = 0;
            virtual void wprint() = 0;
        };

        template<typename CharT, hsd::usize N>
        class format_literal
        {
        public:
            CharT data[N];

            constexpr format_literal(const CharT (&str)[N])
            {
                hsd::copy_n(str, N, data);
            }

            constexpr hsd::usize size() const
            {
                return N;
            }
        };

        static hsd::vector< hsd::u8string > split(const char* str, usize size)
        {
            hsd::vector<hsd::u8string> _buf;
            const char* _iter_f = str;
            const char* _iter_s = hsd::u8cstring::find(_iter_f, '{');

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

        static hsd::vector< hsd::wstring > split(const wchar* str, usize size)
        {
            hsd::vector<hsd::wstring> _buf;
            const wchar_t* _iter_f = str;
            const wchar_t* _iter_s = hsd::wcstring::find(_iter_f, '{');

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

        static void _print(hsd::u8string& str, char val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, uchar val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, i16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hd", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, u16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hu", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, i32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%d", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, u32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%u", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, i64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%lld", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, u64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%llu", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, isize val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%zd", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, usize val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%zu", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, f32 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%f", str.c_str(), val);
            }
        }

        static void _print(hsd::u8string& str, f64 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%lf", str.c_str(), val);
            }
        }

        static void _print(hsd::u8string& str, f128 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%Le", str.c_str(), val);
            }
            else
            {
                fprintf(file_buf, "%s%Lf", str.c_str(), val);
            }
        }

        static void _print(hsd::u8string& str, char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, const char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, u8string& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.c_str(), val.c_str());
        }

        static void _print(hsd::u8string& str, printable& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.c_str());
            val.print();
        }

        template<typename... Args>
        static void _print(hsd::u8string& str, tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.c_str());
            fprintf(file_buf, "(");
            hsd::u8string _before_fmt = "";
            hsd::u8string _after_fmt = ", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<hsd::usize... Ints>(hsd::index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(hsd::make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        template<typename... Args>
        static void _print(hsd::u8string& str, const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.c_str());
            fprintf(file_buf, "(");
            hsd::u8string _before_fmt = "";
            hsd::u8string _after_fmt = ", ";

            if constexpr(val.size() == 1)
            {
                _print(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<hsd::usize... Ints>(hsd::index_sequence<Ints...>) {
                    ((
                        _print(_before_fmt, val.template get<Ints>(), file_buf), 
                        _print(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(hsd::make_index_sequence<val.size() - 1>{});
                
                _print(_before_fmt, val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        static void _wprint(hsd::wstring& str, wchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lc", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, char val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, uchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, i16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hd", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, u16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hu", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, i32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%d", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, u32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%u", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, i64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lld", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, u64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%llu", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, isize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zd", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, usize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zu", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, f32 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%ls%e", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%f", str.c_str(), val);
            }
        }

        static void _wprint(hsd::wstring& str, f64 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"l%s%e", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%lf", str.c_str(), val);
            }
        }

        static void _wprint(hsd::wstring& str, f128 val, FILE* file_buf = stdout)
        {
            if(abs(floor(val) - val) < 0.0001 || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%s%Le", str.c_str(), val);
            }
            else
            {
                fwprintf(file_buf, L"%s%Lf", str.c_str(), val);
            }
        }

        static void _wprint(hsd::wstring& str, char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, const char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, const wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, u8string& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.c_str(), val.c_str());
        }

        static void _wprint(hsd::wstring& str, wstring& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.c_str(), val.c_str());
        }

        template<typename... Args>
        static void _wprint(hsd::wstring& str, tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls", str.c_str());
            fwprintf(file_buf, L"(");
            hsd::wstring _before_fmt = L"";
            hsd::wstring _after_fmt = L", ";

            if constexpr(val.size() == 1)
            {
                _wprint(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<hsd::usize... Ints>(hsd::index_sequence<Ints...>) {
                    ((
                        _wprint(_before_fmt, val.template get<Ints>(), file_buf), 
                        _wprint(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(hsd::make_index_sequence<val.size() - 1>{});
                
                _wprint(_before_fmt, val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }

        template<typename... Args>
        static void _wprint(hsd::wstring& str, const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls", str.c_str());
            fwprintf(file_buf, L"(");
            hsd::wstring _before_fmt = L"";
            hsd::wstring _after_fmt = L", ";

            if constexpr(val.size() == 1)
            {
                _wprint(_before_fmt, val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<hsd::usize... Ints>(hsd::index_sequence<Ints...>) {
                    ((
                        _wprint(_before_fmt, val.template get<Ints>(), file_buf), 
                        _wprint(_before_fmt, _after_fmt, file_buf)
                    ), ...);
                }(hsd::make_index_sequence<val.size() - 1>{});
                
                _wprint(_before_fmt, val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }
    }
} // namespace hsd
