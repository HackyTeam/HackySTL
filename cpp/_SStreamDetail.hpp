#pragma once

#include <wchar.h>
#include "Vector.hpp"
#include "String.hpp"
#include "Pair.hpp"

namespace hsd
{
    namespace sstream_detail
    {
        template <typename CharT>
        static vector< string<CharT> > split(const CharT* str, usize size)
        {
            vector< string<CharT> > _buf;
            const CharT* _iter_f = str;
            const CharT* _iter_s = cstring<CharT>::find(_iter_f, ' ');

            while (_iter_s != nullptr && *_iter_s != '\0')
            {
                _buf.emplace_back(_iter_f, _iter_s - _iter_f);
                _iter_f = _iter_s + 1;
                _iter_s = cstring<CharT>::find(_iter_f, ' ');
            }
            
            _buf.emplace_back(_iter_f, (str + size) - _iter_f);
            return _buf;
        }

		template <typename CharT>
        void _parse(string<CharT>&, auto&);
    
        template <>
        void _parse<char>(u8string& str, char& val)
        {
            sscanf(str.c_str(), "%c", &val);
        }

        template <>
        void _parse<char>(u8string& str, uchar& val)
        {
            sscanf(str.c_str(), "%c", &val);
        }

        template <>
        void _parse<char>(u8string& str, i16& val)
        {
            sscanf(str.c_str(), "%hd", &val);
        }

        template <>
        void _parse<char>(u8string& str, u16& val)
        {
            sscanf(str.c_str(), "%hu", &val);
        }

        template <>
        void _parse<char>(u8string& str, i32& val)
        {
            sscanf(str.c_str(), "%d", &val);
        }

        template <>
        void _parse<char>(u8string& str, u32& val)
        {
            sscanf(str.c_str(), "%u", &val);
        }

        template <>
        void _parse<char>(u8string& str, i64& val)
        {
            sscanf(str.c_str(), "%lld", &val);
        }

        template <>
        void _parse<char>(u8string& str, u64& val)
        {
            sscanf(str.c_str(), "%llu", &val);
        }

        template <>
        void _parse<char>(u8string& str, f32& val)
        {
            sscanf(str.c_str(), "%f", &val);
        }

        template <>
        void _parse<char>(u8string& str, f64& val)
        {
            sscanf(str.c_str(), "%lf", &val);
        }

        template <>
        void _parse<char>(u8string& str, f128& val)
        {
            sscanf(str.c_str(), "%Lf", &val);
        }

        template <>
        void _parse<char>(u8string& str, u8string& val)
        {
            val = move(str);
        }

        template <>
        void _parse<wchar>(wstring& str, char& val)
        {
            swscanf(str.c_str(), L"%c", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, uchar& val)
        {
            swscanf(str.c_str(), L"%c", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, wchar& val)
        {
            swscanf(str.c_str(), L"%lc", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, i16& val)
        {
            swscanf(str.c_str(), L"%hd", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, u16& val)
        {
            swscanf(str.c_str(), L"%hu", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, i32& val)
        {
            swscanf(str.c_str(), L"%d", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, u32& val)
        {
            swscanf(str.c_str(), L"%u", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, i64& val)
        {
            swscanf(str.c_str(), L"%lld", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, u64& val)
        {
            swscanf(str.c_str(), L"%llu", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, f32& val)
        {
            swscanf(str.c_str(), L"%f", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, f64& val)
        {
            swscanf(str.c_str(), L"%lf", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, f128& val)
        {
            swscanf(str.c_str(), L"%Lf", &val);
        }

        template <>
        void _parse<wchar>(wstring& str, u8string& val)
        {
            val = str;
        }

        template <>
        void _parse<wchar>(wstring& str, wstring& val)
        {
            val = move(str);
        }
    } // namespace sstream_detail
} // namespace hsd
