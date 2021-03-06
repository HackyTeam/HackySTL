#pragma once

#include <stdio.h>
#include <wchar.h>
#include <locale.h>

#include "_IoOverload.hpp"
#include "SStream.hpp"

namespace hsd
{
    namespace io_detail
    {
        class bufferable
        {
        protected:
            static inline sstream _io_buf{4096};
            static inline wsstream _wio_buf{4096};
        };

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(FILE* file_buf = stdout)
        {
            fputs(str.data, file_buf);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(bool val, FILE* file_buf = stdout)
        {
            if (val == true)
            {
                fputs((str + "true").data, file_buf);
            }
            else
            {
                fputs((str + "false").data, file_buf);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(char val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%c").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(char8 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%zc").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(i16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%hd").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(u16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%hu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(i32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%d").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(u32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%u").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(i64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%lld").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(u64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%llu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(long val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%zd").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(ulong val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%zu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(f32 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fprintf(file_buf, (str + "%e").data, val);
            }
            else
            {
                fprintf(file_buf, (str + "%f").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(f64 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fprintf(file_buf, (str + "%e").data, val);
            }
            else
            {
                fprintf(file_buf, (str + "%lf").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(f128 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fprintf(file_buf, (str + "%Le").data, val);
            }
            else
            {
                fprintf(file_buf, (str + "%Lf").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(char8* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(const char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(const char8* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "%s").data, val);
        }
        
        template <basic_string_literal str, typename... Args>
        requires (IsSame<char, typename decltype(str)::char_type>)
        inline void _print(const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, (str + "(").data);
            // Because clang is broken
            constexpr usize _tup_size = sizeof...(Args);

            if constexpr (_tup_size == 1)
            {
                _print<"">(val.template get<0>(), file_buf);
            }
            else if constexpr (val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<"">(val.template get<Ints>(), file_buf), 
                        _print<", ">(file_buf)
                    ), ...);
                }(make_index_sequence<_tup_size - 1>{});
                
                _print<"">(val.template get<_tup_size - 1>());
            }

            fprintf(file_buf, ")");
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(FILE* file_buf = stdout)
        {
            fwprintf(file_buf, str.data);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(bool val, FILE* file_buf = stdout)
        {
            if (val == true)
            {
                fputws((str + L"true").data, file_buf);
            }
            else
            {
                fputws((str + L"false").data, file_buf);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(wchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%lc").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(char val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%c").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(char8 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%zc").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(i16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%hd").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(u16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%hu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(i32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%d").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(u32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%u").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(i64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%lld").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(u64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%llu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(long val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%zd").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(ulong val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%zu").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(f32 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fwprintf(file_buf, (str + L"%e").data, val);
            }
            else
            {
                fwprintf(file_buf, (str + L"%f").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(f64 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fwprintf(file_buf, (str + L"%e").data, val);
            }
            else
            {
                fwprintf(file_buf, (str + L"%lf").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(f128 val, FILE* file_buf = stdout)
        {
            auto _res = math::abs(math::floor(val) - val);

            if ((_res < 0.0001 && _res != 0) || math::abs(val) > 1.e+10)
            {
                fwprintf(file_buf, (str + L"%Le").data, val);
            }
            else
            {
                fwprintf(file_buf, (str + L"%Lf").data, val);
            }
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(char8* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%ls").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(const char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(const char8* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%s").data, val);
        }

        template <basic_string_literal str>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(const wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"%ls").data, val);
        }

        template <basic_string_literal str, typename... Args>
        requires (IsSame<wchar, typename decltype(str)::char_type>)
        inline void _print(const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, (str + L"(").data);
            // Because clang is broken
            constexpr usize _tup_size = sizeof...(Args);

            if constexpr (_tup_size == 1)
            {
                _print<L"">(val.template get<0>(), file_buf);
            }
            else if constexpr (_tup_size > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<L"">(val.template get<Ints>(), file_buf), 
                        _print<L", ">(file_buf)
                    ), ...);
                }(make_index_sequence<_tup_size - 1>{});
                
                _print<L"">(val.template get<_tup_size - 1>());
            }

            fwprintf(file_buf, L")");
        }
    }
} // namespace hsd
