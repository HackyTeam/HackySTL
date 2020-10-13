#pragma once

#include <stdio.h>
#include <stdexcept>

#include "SStream.hpp"
#include "Vector.hpp"

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

        template<hsd::usize N>
        class format_literal
        {
        public:
            char data[N];

            constexpr format_literal(const char (&str)[N])
            {
                hsd::copy_n(str, N, data);
            }

            constexpr hsd::usize size() const
            {
                return N;
            }
        };

        template<hsd::usize N>
        class wformat_literal
        {
        public:
            wchar data[N];

            constexpr wformat_literal(const wchar (&str)[N])
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

        static void _print(hsd::u8string& str, i8 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hhd", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, u8 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%hhu", str.c_str(), val);
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
            fprintf(file_buf, "%s%f", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, f64 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%lf", str.c_str(), val);
        }

        static void _print(hsd::u8string& str, f128 val, FILE* file_buf = stdout)
        {
            fprintf(file_buf, "%s%Lf", str.c_str(), val);
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
            fprintf(file_buf, "%s%s", str.c_str(), val.c_str());;
        }

        static void _wprint(hsd::wstring& str, i8 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hhd", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, u8 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%hhu", str.c_str(), val);
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
            fwprintf(file_buf, L"%ls%f", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, f64 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%lf", str.c_str(), val);
        }

        static void _wprint(hsd::wstring& str, f128 val, FILE* file_buf = stdout)
        {
            fwprintf(file_buf, L"%ls%Lf", str.c_str(), val);
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
    }
} // namespace hsd
