#pragma once

#include <stdio.h>
#include <stdexcept>

#include "../String/String.hpp"
#include "../Vector/Vector.hpp"

namespace hsd
{
    class io
    {
    private:
        static hsd::vector< hsd::u8string > _split(const char* str, size_t size)
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
            if(*_iter_f != '\0')
            {
                _buf.emplace_back(_iter_f, (str + size) - _iter_f);
            }
            
            return _buf;
        }

        static hsd::vector< hsd::wstring > _split(const wchar_t* str, size_t size)
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
            if(*_iter_f != '\0')
            {
                _buf.emplace_back(_iter_f, (str + size) - _iter_f);
            }
            
            return _buf;
        }
    public:
        static hsd::u8string& read()
        {
            static hsd::u8string _ret(4096);
            scanf("%s", _ret.data());
            return _ret;
        }

        static hsd::wstring& wread()
        {
            static hsd::wstring _ret(4096);
            scanf("%ls", _ret.data());
            return _ret;
        }

        template< size_t N, typename... Args >
        static void print(const char (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::u8string> _fmt_buf = _split(fmt, N - 1);
            hsd::vector<hsd::u8string> _args_buf = {hsd::u8string::to_string(args)...};
            u8string _print_buf;

            if(_args_buf.size() != _fmt_buf.size() && _args_buf.size() + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                size_t index = 0;

                for(; index < _args_buf.size(); index++)
                {
                    _print_buf = _print_buf + _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf = _print_buf + _fmt_buf[index];
                }
                
                printf("%s", _print_buf.c_str());
            }
        }

        template< size_t N, typename... Args >
        static void wprint(const wchar_t (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::wstring> _fmt_buf = _split(fmt, N - 1);
            hsd::vector<hsd::wstring> _args_buf = {hsd::wstring::to_string(args)...};
            wstring _print_buf;

            if(_args_buf.size() + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                size_t index = 0;

                for(; index < _args_buf.size(); index++)
                {
                    _print_buf = _print_buf + _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf = _print_buf + _fmt_buf[index];
                }
                for(auto _letter : _print_buf)
                    printf("%lc", _letter);
            }
        }
    };
} // namespace hsd
