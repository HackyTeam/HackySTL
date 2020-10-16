#pragma once

#include "_IoDetail.hpp"

namespace hsd
{
    class io : private io_detail::bufferable
    {
    public:
        static u8sstream& read_line()
        {
            do
            {
                _u8io_buf.reset_data();
                char* _str = fgets(_u8io_buf.data(), 4096, stdin);

                if(_str == nullptr)
                    throw std::runtime_error("Input requirements not satisfied");
            }while(_u8io_buf.c_str()[0] == '\n');

            return _u8io_buf;
        }

        static u8sstream& read()
        {
            do
            {
                _u8io_buf.reset_data();
                scanf("%s", _u8io_buf.data());
            }while(_u8io_buf.c_str()[0] == '\n');

            return _u8io_buf;
        }

        static wsstream& wread_line()
        {
            do
            {
                _wio_buf.reset_data();
                wchar* _str = fgetws(_wio_buf.data(), 4096, stdin);

                if(_str == nullptr)
                    throw std::runtime_error("Input requirements not satisfied");
            }while(_wio_buf.c_str()[0] == '\n');

            return _wio_buf;
        }

        static wsstream& wread()
        {
            do
            {
                _wio_buf.reset_data();
                wscanf(L"%ls", _wio_buf.data());
            }while(_wio_buf.c_str()[0] == '\n');

            return _wio_buf;
        }

        template< io_detail::format_literal fmt, typename... Args >
        static void print(Args&&... args)
        {
            using io_detail::_print;
            vector<u8string> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                printf(_fmt_buf[sizeof...(Args)].c_str());
            }
        }

        template< io_detail::format_literal fmt, typename... Args >
        static void wprint(Args&&... args)
        {
            using io_detail::_print;
            vector<wstring> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                wprintf(_fmt_buf[sizeof...(Args)].c_str());
            }
        }

        template< io_detail::format_literal fmt, typename... Args >
        static void err_print(Args&&... args)
        {
            using io_detail::_print;
            vector<u8string> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args, stderr), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                fprintf(stderr, _fmt_buf[sizeof...(Args)].c_str());
            }
        }

        template< io_detail::format_literal fmt, typename... Args >
        static void err_wprint(Args&&... args)
        {
            using io_detail::_print;
            vector<wstring> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args, stderr), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                fwprintf(stderr, _fmt_buf[sizeof...(Args)].c_str());
            }
        }
    };

    class file : private io_detail::bufferable
    {
    private:
        FILE* _file_buf = nullptr;
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

        u8sstream& read_line()
        {
            _u8io_buf.reset_data();
            
            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(fgets(_u8io_buf.data(), 4096, _file_buf) == nullptr)
            { 
                _u8io_buf.reset_data();
            }
            
            return _u8io_buf;
        }

        wsstream& wread_line()
        {
            _wio_buf.reset_data();
            
            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(fgetws(_wio_buf.data(), 4096, _file_buf) == nullptr)
            { 
                _wio_buf.reset_data();
            }
            
            return _wio_buf;
        }

        u8sstream& read()
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

        wsstream& wread()
        {
            _wio_buf.reset_data();

            if(only_write())
            {
                throw std::runtime_error("Cannot read file. It is in write mode");
            }
            if(fwscanf(_file_buf, L"%ls", _wio_buf.data()) == EOF)
            {
                _wio_buf.reset_data();
            }

            return _wio_buf;
        }

        template< io_detail::format_literal fmt, typename... Args >
        void print(Args&&... args)
        {
            if(only_read())
                throw std::runtime_error("Cannot write file. It is in read mode");

            using io_detail::_print;
            vector<u8string> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf, this]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args, _file_buf), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                fprintf(_file_buf, _fmt_buf[sizeof...(Args)].c_str());
            }
        }

        template< io_detail::format_literal fmt, typename... Args >
        void wprint(Args&&... args)
        {
            if(only_read())
                throw std::runtime_error("Cannot write file. It is in read mode");

            using io_detail::_print;
            vector<wstring> _fmt_buf = io_detail::split(fmt.data, fmt.size() - 1);

            if(sizeof...(Args) + 1 != _fmt_buf.size())
            {
                throw std::runtime_error("Arguments don\'t match");
            }
            else
            {
                [&_fmt_buf, this]<usize... Ints>(index_sequence<Ints...>, auto&... print_args)
                {
                    (_print(_fmt_buf[Ints], print_args, _file_buf), ...);
                }(make_index_sequence<sizeof...(Args)>{}, args...);

                fwprintf(_file_buf, _fmt_buf[sizeof...(Args)].c_str());
            }
        }
    };

} // namespace hsd
