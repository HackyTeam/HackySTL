#pragma once

#include <math.h>
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <stdexcept>

#include "SStream.hpp"
#include "StackArray.hpp"
#include "Pair.hpp"
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
        class string_literal
        {
        public:
            CharT data[N]{};
            using char_type = CharT;

            constexpr string_literal() {}

            constexpr string_literal(const CharT (&str)[N])
            {
                copy_n(str, N, data);
            }

            constexpr string_literal(const CharT* str, usize len)
            {
                copy_n(str, len, data);
            }

            constexpr usize size() const
            {
                return N;
            }
        };

        template<usize N>
        using u8string_literal = string_literal<char, N>;
        template<usize N>
        using wstring_literal = string_literal<wchar, N>;

        template<string_literal fmt, usize N>
        static constexpr auto split()
        {
            using char_type = decltype(fmt)::char_type;
            usize _index = 0;
            stack_array< pair<const char_type*, usize>, N > _buf;
            const char_type* _iter_f = fmt.data;
            const char_type* _iter_s = cstring<char_type>::find(_iter_f, '{');

            if(_iter_s != nullptr && *(_iter_s + 1) != '}')
            {
                throw std::runtime_error("invalid character after \'{\'");
            }
            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf[_index++] = {_iter_f, static_cast<usize>(_iter_s - _iter_f)};
                _iter_f = _iter_s + 2;
                _iter_s = cstring<char_type>::find(_iter_f, '{');

                if(_iter_s != nullptr && *(_iter_s + 1) != '}')
                    throw std::runtime_error("invalid character after \'{\'");
            }

            _buf[_index] = {_iter_f, static_cast<usize>(fmt.data + fmt.size() - _iter_f)};
            return pair{_buf, _index + 1};
        }

        template<u8string_literal str>
        static void _print(FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s", str.data);
        }

        template<u8string_literal str>
        static void _print(char val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.data, val);
        }

        template<u8string_literal str>
        static void _print(uchar val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%c", str.data, val);
        }

        template<u8string_literal str>
        static void _print(i16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hd", str.data, val);
        }

        template<u8string_literal str>
        static void _print(u16 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hu", str.data, val);
        }

        template<u8string_literal str>
        static void _print(i32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%d", str.data, val);
        }

        template<u8string_literal str>
        static void _print(u32 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%u", str.data, val);
        }

        template<u8string_literal str>
        static void _print(i64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%lld", str.data, val);
        }

        template<u8string_literal str>
        static void _print(u64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%llu", str.data, val);
        }

        template<u8string_literal str>
        static void _print(isize val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%zd", str.data, val);
        }

        template<u8string_literal str>
        static void _print(usize val, FILE* file_buf = stdout)
        {
            
            fprintf(file_buf, "%s%zu", str.data, val);
        }

        template<u8string_literal str>
        static void _print(f32 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.data, val);
            }
            else
            {
                fprintf(file_buf, "%s%f", str.data, val);
            }
        }

        template<u8string_literal str>
        static void _print(f64 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%e", str.data, val);
            }
            else
            {
                fprintf(file_buf, "%s%lf", str.data, val);
            }
        }

        template<u8string_literal str>
        static void _print(f128 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fprintf(file_buf, "%s%Le", str.data, val);
            }
            else
            {
                fprintf(file_buf, "%s%Lf", str.data, val);
            }
        }

        template<u8string_literal str>
        static void _print(char* val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.data, val);
        }

        template<u8string_literal str>
        static void _print(const char* val, FILE* file_buf = stdout)
        {
            
            fprintf(file_buf, "%s%s", str.data, val);
        }

        template<u8string_literal str>
        static void _print(u8string& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%s", str.data, val.c_str());
        }
        
        template<u8string_literal str, typename... Args>
        static void _print(tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s(", str.data);

            if constexpr(val.size() == 1)
            {
                _print<"">(val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<"">(val.template get<Ints>(), file_buf), 
                        _print<"">(", ", file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print<"">(val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        template<u8string_literal str, typename... Args>
        static void _print(const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s(", str.data);

            if constexpr(val.size() == 1)
            {
                _print<"">(val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<"">(val.template get<Ints>(), file_buf), 
                        _print<"">(", ", file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print<"">(val.template get<val.size() - 1>());
            }

            fprintf(file_buf, ")");
        }

        template<wstring_literal str>
        static void _print(FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls", str.data);
        }

        template<wstring_literal str>
        static void _print(wchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lc", str.data, val);
        }

        template<wstring_literal str>
        static void _print(char val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.data, val);
        }

        template<wstring_literal str>
        static void _print(uchar val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%c", str.data, val);
        }

        template<wstring_literal str>
        static void _print(i16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hd", str.data, val);
        }

        template<wstring_literal str>
        static void _print(u16 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hu", str.data, val);
        }

        template<wstring_literal str>
        static void _print(i32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%d", str.data, val);
        }

        template<wstring_literal str>
        static void _print(u32 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%u", str.data, val);
        }

        template<wstring_literal str>
        static void _print(i64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lld", str.data, val);
        }

        template<wstring_literal str>
        static void _print(u64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%llu", str.data, val);
        }

        template<wstring_literal str>
        static void _print(isize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zd", str.data, val);
        }

        template<wstring_literal str>
        static void _print(usize val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%zu", str.data, val);
        }

        template<wstring_literal str>
        static void _print(f32 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%ls%e", str.data, val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%f", str.data, val);
            }
        }

        template<wstring_literal str>
        static void _print(f64 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%ls%e", str.data, val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%lf", str.data, val);
            }
        }

        template<wstring_literal str>
        static void _print(f128 val, FILE* file_buf = stdout)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                fwprintf(file_buf, L"%ls%Le", str.data, val);
            }
            else
            {
                fwprintf(file_buf, L"%ls%Lf", str.data, val);
            }
        }

        template<wstring_literal str>
        static void _print(char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.data, val);
        }

        template<wstring_literal str>
        static void _print(const char* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.data, val);
        }

        template<wstring_literal str>
        static void _print(wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.data, val);
        }

        template<wstring_literal str>
        static void _print(const wchar* val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.data, val);
        }

        template<wstring_literal str>
        static void _print(u8string& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%s", str.data, val.c_str());
        }

        template<wstring_literal str>
        static void _print(wstring& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%ls", str.data, val.c_str());
        }

        template<wstring_literal str, typename... Args>
        static void _print(tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls(", str.data);

            if constexpr(val.size() == 1)
            {
                _print<L"">(val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<L"">(val.template get<Ints>(), file_buf), 
                        _print<L"">(L", ", file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print<L"">(val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }

        template<wstring_literal str, typename... Args>
        static void _print(const tuple<Args...>& val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls(", str.data);

            if constexpr(val.size() == 1)
            {
                _print<L"">(val.template get<0>(), file_buf);
            }
            else if constexpr(val.size() > 1)
            {
                [&]<usize... Ints>(index_sequence<Ints...>) {
                    ((
                        _print<L"">(val.template get<Ints>(), file_buf), 
                        _print<L"">(L", ", file_buf)
                    ), ...);
                }(make_index_sequence<val.size() - 1>{});
                
                _print<L"">(val.template get<val.size() - 1>());
            }

            fwprintf(file_buf, L")");
        }
    }
} // namespace hsd
