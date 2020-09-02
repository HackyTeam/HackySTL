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

        static hsd::vector< hsd::u8string > split(const char* str, size_t size)
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

        static hsd::vector< hsd::wstring > split(const wchar_t* str, size_t size)
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
    }

    class io : private io_detail::bufferable
    {
    public:
        static hsd::u8sstream& read_line()
        {
            _u8io_buf.reset_data();
            size_t _io_buf_len = 4096;
            char* _u8line_buf = _u8io_buf.data();
            long _len = getline(&_u8line_buf, &_io_buf_len, stdin);
            _u8line_buf[_len - 1] = '\0';
            return _u8io_buf;
        }

        static hsd::u8sstream& read()
        {
            _u8io_buf.reset_data();
            scanf("%s", _u8io_buf.data());
            return _u8io_buf;
        }

        static hsd::wsstream& wread()
        {
            _wio_buf.reset_data();
            scanf("%ls", _wio_buf.data());
            return _wio_buf;
        }

        template< size_t N, typename... Args >
        static void print(const char (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::u8string> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::u8string> _args_buf = {
                hsd::forward<hsd::u8string>(hsd::u8string::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter :  _print_buf)
                    printf("%c", _letter);
            }
        }

        template< size_t N, typename... Args >
        static void wprint(const wchar_t (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::wstring> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::wstring> _args_buf = {
                hsd::forward<hsd::wstring>(hsd::wstring::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter : _print_buf)
                    printf("%lc", _letter);
            }
        }

        template< size_t N, typename... Args >
        static void err_print(const char (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::u8string> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::u8string> _args_buf = {
                hsd::forward<hsd::u8string>(hsd::u8string::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter :  _print_buf)
                    fprintf(stderr, "%c", _letter);
            }
        }

        template< size_t N, typename... Args >
        static void err_wprint(const wchar_t (&fmt)[N], Args&&... args)
        {
            hsd::vector<hsd::wstring> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::wstring> _args_buf = {
                hsd::forward<hsd::wstring>(hsd::wstring::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter : _print_buf)
                    fprintf(stderr, "%c", _letter);
            }
        }
    };

    class file : private io_detail::bufferable
    {
    private:
        FILE* _file_buf;
        const char* _file_mode;

        bool only_write()
        {
            return (u8cstring::compare(_file_mode, "w") == 0
            || u8cstring::compare(_file_mode, "wb") == 0);
        }

        bool only_read()
        {
            return (u8cstring::compare(_file_mode, "r") == 0
            || u8cstring::compare(_file_mode, "rb") == 0);
        }

    public:
        struct options
        {
            struct text
            {
                static inline constexpr char read[] = "r";
                static inline constexpr char write[] = "w";
                static inline constexpr char append[] = "a";
                static inline constexpr char read_write[] = "r+";
                static inline constexpr char rw_create[] = "w+";
                static inline constexpr char rw_append[] = "a+";
            };

            struct binary
            {
                static inline constexpr char read[] = "rb";
                static inline constexpr char write[] = "wb";
                static inline constexpr char append[] = "ab";
                static inline constexpr char read_write[] = "r+b";
                static inline constexpr char rw_create[] = "w+b";
                static inline constexpr char rw_append[] = "a+b";
            };
        };

        file(const char* file_path, const char* open_option)
        {
            _file_buf = fopen(file_path, open_option);
            _file_mode = open_option;
            
            if(_file_buf == nullptr)
                throw std::runtime_error("File not found");
        }

        ~file()
        {
            fclose(_file_buf);
        }

        hsd::u8sstream& read_line()
        {
            _u8io_buf.reset_data();
            size_t _io_buf_len = 4096;
            char* _u8line_buf = _u8io_buf.data();
            
            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(getline(&_u8line_buf, &_io_buf_len, _file_buf) == EOF)
            { 
                _u8io_buf.reset_data();
            }
            else if(_u8line_buf[0] == '\n')
            {
                if(getline(&_u8line_buf, &_io_buf_len, _file_buf) == EOF)
                { 
                    _u8io_buf.reset_data();
                }
            }
            
            return _u8io_buf;
        }

        hsd::u8sstream& read()
        {
            _u8io_buf.reset_data();

            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(fscanf(_file_buf, "%s", _u8io_buf.data()) == EOF)
            {
                _u8io_buf.reset_data();
            }
            
            return _u8io_buf;
        }

        hsd::wsstream& wread()
        {
            _wio_buf.reset_data();

            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(fscanf(_file_buf, "%ls", _wio_buf.data()) == EOF)
            {
                _wio_buf.reset_data();
            }

            return _wio_buf;
        }

        template< size_t N, typename... Args >
        void print(const char (&fmt)[N], Args&&... args)
        {
            if(only_read())
                throw std::runtime_error("Cannot write file. It is in read mode");

            hsd::vector<hsd::u8string> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::u8string> _args_buf = {
                hsd::forward<hsd::u8string>(hsd::u8string::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter :  _print_buf)
                    fprintf(_file_buf, "%c", _letter);
            }
        }

        template< size_t N, typename... Args >
        void wprint(const wchar_t (&fmt)[N], Args&&... args)
        {
            if(only_read())
                throw std::runtime_error("Cannot write file. It is in read mode");

            hsd::vector<hsd::wstring> _fmt_buf = io_detail::split(fmt, N - 1);
            hsd::vector<hsd::wstring> _args_buf = {
                hsd::forward<hsd::wstring>(hsd::wstring::to_string(args))...
            };
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
                    _print_buf += _fmt_buf[index] + _args_buf[index];
                }
                if(_fmt_buf.size() != _args_buf.size())
                {
                    _print_buf += _fmt_buf[index];
                }
                for(auto _letter : _print_buf)
                    fprintf(_file_buf, "%lc", _letter);
            }
        }
    };

} // namespace hsd
