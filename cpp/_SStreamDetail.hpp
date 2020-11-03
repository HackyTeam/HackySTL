#pragma once

#include <wchar.h>
#include <stdio.h>

#include "StringLiteral.hpp"
#include "StackArray.hpp"
#include "Vector.hpp"
#include "String.hpp"

namespace hsd
{
    namespace sstream_detail
    {
        template <typename CharT>
        static auto split_data(const CharT* str, usize size)
        {
            vector< pair<const CharT*, usize> > _buf;
            const CharT* _iter_f = str;
            const CharT* _iter_s = basic_cstring<CharT>::find(_iter_f, ' ');

            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf.emplace_back(_iter_f, static_cast<usize>(_iter_s - _iter_f));
                _iter_f = _iter_s + 1;
                _iter_s = basic_cstring<CharT>::find(_iter_f, ' ');
            }
            
            _buf.emplace_back(_iter_f, static_cast<usize>((str + size) - _iter_f));
            return _buf;
        }

        template <basic_string_literal fmt, usize N>
        static constexpr auto split_literal()
        {
            using char_type = decltype(fmt)::char_type;
            usize _index = 0;
            stack_array< pair<const char_type*, usize>, N > _buf;
            const char_type* _iter_f = fmt.data;
            const char_type* _iter_s = basic_cstring<char_type>::find(_iter_f, '{');

            if(_iter_s != nullptr && *(_iter_s + 1) != '}')
            {
                throw std::runtime_error("invalid character after \'{\'");
            }
            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf[_index++] = {_iter_f, static_cast<usize>(_iter_s - _iter_f)};
                _iter_f = _iter_s + 2;
                _iter_s = basic_cstring<char_type>::find(_iter_f, '{');

                if(_iter_s != nullptr && *(_iter_s + 1) != '}')
                    throw std::runtime_error("invalid character after \'{\'");
            }

            _buf[_index] = {_iter_f, static_cast<usize>(fmt.data + fmt.size() - _iter_f)};
            return pair{_buf, _index};
        }

		template <typename CharT>
        void _parse(pair<const CharT*, usize>&, auto&);
    
        template <>
        void _parse<char>(pair<const char*, usize>& str, char& val)
        {
            sscanf(str.first, "%c", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, uchar& val)
        {
            sscanf(str.first, "%c", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, i16& val)
        {
            sscanf(str.first, "%hd", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, u16& val)
        {
            sscanf(str.first, "%hu", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, i32& val)
        {
            sscanf(str.first, "%d", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, u32& val)
        {
            sscanf(str.first, "%u", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, i64& val)
        {
            sscanf(str.first, "%lld", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, u64& val)
        {
            sscanf(str.first, "%llu", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, f32& val)
        {
            sscanf(str.first, "%f", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, f64& val)
        {
            sscanf(str.first, "%lf", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, f128& val)
        {
            sscanf(str.first, "%Lf", &val);
        }

        template <>
        void _parse<char>(pair<const char*, usize>& str, string& val)
        {
            val = move(string(str.first, str.second));
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, char& val)
        {
            swscanf(str.first, L"%c", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, uchar& val)
        {
            swscanf(str.first, L"%c", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, wchar& val)
        {
            swscanf(str.first, L"%lc", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, i16& val)
        {
            swscanf(str.first, L"%hd", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, u16& val)
        {
            swscanf(str.first, L"%hu", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, i32& val)
        {
            swscanf(str.first, L"%d", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, u32& val)
        {
            swscanf(str.first, L"%u", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, i64& val)
        {
            swscanf(str.first, L"%lld", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, u64& val)
        {
            swscanf(str.first, L"%llu", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, f32& val)
        {
            swscanf(str.first, L"%f", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, f64& val)
        {
            swscanf(str.first, L"%lf", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, f128& val)
        {
            swscanf(str.first, L"%Lf", &val);
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, string& val)
        {
            val = move(wstring(str.first, str.second));
        }

        template <>
        void _parse<wchar>(pair<const wchar*, usize>& str, wstring& val)
        {
            val = move(wstring(str.first, str.second));
        }

        void _sub_from(i32& from, i32 amount)
        {
            if(amount < 0)
            {
                throw std::runtime_error("Buffer too small");
            }
            else
            {
                from -= amount;
            }
        }

        void _sub_from(usize& from, i32 amount)
        {
            if(amount < 0)
            {
                throw std::runtime_error("Buffer too small");
            }
            else
            {
                from -= amount;
            }
        }

        template <string_literal str>
        static i32 _write(pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, str.data);
        }

        template <string_literal str>
        static i32 _write(char val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%c").data, val);
        }

        template <string_literal str>
        static i32 _write(char8 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%zc").data, val);
        }

        template <string_literal str>
        static i32 _write(i16 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%hd").data, val);
        }

        template <string_literal str>
        static i32 _write(u16 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%hu").data, val);
        }

        template <string_literal str>
        static i32 _write(i32 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%d").data, val);
        }

        template <string_literal str>
        static i32 _write(u32 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%u").data, val);
        }

        template <string_literal str>
        static i32 _write(i64 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%lld").data, val);
        }

        template <string_literal str>
        static i32 _write(u64 val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%llu").data, val);
        }

        template <string_literal str>
        static i32 _write(isize val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%zd").data, val);
        }

        template <string_literal str>
        static i32 _write(usize val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%zu").data, val);
        }

        template <string_literal str>
        static i32 _write(f32 val, pair<char*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%e").data, val);
            }
            else
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%f").data, val);
            }
        }

        template <string_literal str>
        static i32 _write(f64 val, pair<char*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%e").data, val);
            }
            else
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%lf").data, val);
            }
        }

        template <string_literal str>
        static i32 _write(f128 val, pair<char*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%Le").data, val);
            }
            else
            {
                return snprintf(dest.first, dest.second, basic_string_literal(str, "%Lf").data, val);
            }
        }

        template <string_literal str>
        static i32 _write(const char* val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%s").data, val);
        }

        template <string_literal str>
        static i32 _write(const char8* val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%s").data, val);
        }

        template <string_literal str>
        static i32 _write(const string& val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%s").data, val.c_str());
        }

        template <string_literal str>
        static i32 _write(const u8string& val, pair<char*, usize> dest)
        {
            return snprintf(dest.first, dest.second, basic_string_literal(str, "%s").data, val.c_str());
        }
        
        template <string_literal str, typename... Args>
        static i32 _write(const tuple<Args...>& val, pair<char*, usize> dest)
        {
            i32 _len = dest.second;

            [&]<usize... Ints>(index_sequence<Ints...>) {
                (
                    (_sub_from(_len, _write<"">(val.template get<Ints>(), 
                    {dest.first + (dest.second - _len), _len}))), ...
                );
            }(make_index_sequence<val.size()>{});

            return dest.second - _len;
        }

        template <wstring_literal str>
        static i32 _write(pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, str.data);
        }

        template <wstring_literal str>
        static i32 _write(wchar val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%lc").data, val);
        }

        template <wstring_literal str>
        static i32 _write(char val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%c").data, val);
        }

        template <wstring_literal str>
        static i32 _write(char8 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%zc").data, val);
        }

        template <wstring_literal str>
        static i32 _write(i16 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%hd").data, val);
        }

        template <wstring_literal str>
        static i32 _write(u16 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%hu").data, val);
        }

        template <wstring_literal str>
        static i32 _write(i32 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%d").data, val);
        }

        template <wstring_literal str>
        static i32 _write(u32 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%u").data, val);
        }

        template <wstring_literal str>
        static i32 _write(i64 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%lld").data, val);
        }

        template <wstring_literal str>
        static i32 _write(u64 val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%llu").data, val);
        }

        template <wstring_literal str>
        static i32 _write(isize val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%zd").data, val);
        }

        template <wstring_literal str>
        static i32 _write(usize val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%zu").data, val);
        }

        template <wstring_literal str>
        static i32 _write(f32 val, pair<wchar*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return swprintf(dest.first, dest.second, basic_string_literal(str, L"%e").data, val);
            }
            else
            {
                swprintf(dest.first, dest.second, basic_string_literal(str, L"%f").data, val);
            }
        }

        template <wstring_literal str>
        static i32 _write(f64 val, pair<wchar*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return swprintf(dest.first, dest.second, basic_string_literal(str, L"%e").data, val);
            }
            else
            {
                return swprintf(dest.first, dest.second, basic_string_literal(str, L"%lf").data, val);
            }
        }

        template <wstring_literal str>
        static i32 _write(f128 val, pair<wchar*, usize> dest)
        {
            auto _res = abs(floor(val) - val);

            if((_res < 0.0001 && _res != 0) || abs(val) > 1.e+10)
            {
                return swprintf(dest.first, dest.second, basic_string_literal(str, L"%Le").data, val);
            }
            else
            {
                return swprintf(dest.first, dest.second, basic_string_literal(str, L"%Lf").data, val);
            }
        }

        template <wstring_literal str>
        static i32 _write(const char* val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%s").data, val);
        }

        template <wstring_literal str>
        static i32 _write(const char8* val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%s").data, val);
        }

        template <wstring_literal str>
        static i32 _write(const wchar* val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%ls").data, val);
        }

        template <wstring_literal str>
        static i32 _write(const string& val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%s").data, val.c_str());
        }

        template <wstring_literal str>
        static i32 _write(const u8string& val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%s").data, val.c_str());
        }

        template <wstring_literal str>
        static i32 _write(const wstring& val, pair<wchar*, usize> dest)
        {
            return swprintf(dest.first, dest.second, basic_string_literal(str, L"%ls").data, val.c_str());
        }

        template <wstring_literal str, typename... Args>
        static i32 _write(const tuple<Args...>& val, pair<wchar*, usize> dest)
        {
            i32 _len = dest.second;

            [&]<usize... Ints>(index_sequence<Ints...>) {
                (
                    (_sub_from(_len, _write<L" ">(val.template get<Ints>(), 
                    {dest.first + (dest.second - _len), _len}))), ...
                );
            }(make_index_sequence<val.size()>{});

            return dest.second - _len;
        }
    } // namespace sstream_detail
} // namespace hsd
