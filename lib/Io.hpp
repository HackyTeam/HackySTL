#pragma once

#include "_IoDetail.hpp"
#include "String.hpp"

namespace hsd
{
    class io : private io_detail::bufferable
    {
    private:
        FILE* _file_buf = nullptr;
        const char* _file_mode = "";
        bool _is_console_file = false;

        inline bool only_write()
        {
            return (
                cstring::compare(_file_mode, "w") == 0 || 
                cstring::compare(_file_mode, "wb") == 0
            );
        }

        inline bool only_read()
        {
            return (
                cstring::compare(_file_mode, "r") == 0 || 
                cstring::compare(_file_mode, "rb") == 0
            );
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

        inline io() = default;
        inline io(const io&) = delete;
        inline io& operator=(const io&) = delete;

        inline io(io&& other)
            : _file_buf{exchange(other._file_buf, nullptr)}, 
            _file_mode{exchange(other._file_mode, nullptr)},
            _is_console_file{exchange(other._is_console_file, true)}
        {}

        inline io& operator=(io&& rhs)
        {
            _file_buf = exchange(rhs._file_buf, nullptr);
            _file_mode = exchange(rhs._file_mode, nullptr);
            _is_console_file = exchange(rhs._is_console_file, true);

            return *this;
        }

        inline ~io()
        {
            close();
        }

        static inline auto load_file(const char* file_path, 
            const char* open_option = options::text::read)
            -> Result<io, runtime_error>
        {
            io _file;
            
            _file._is_console_file = false;
            _file._file_buf = fopen(file_path, open_option);
            _file._file_mode = open_option;
            
            if (_file._file_buf == nullptr)
                return runtime_error{"File not found"};

            return _file;
        }

        static inline io& cout()
        {
            static io _instance;
            _instance._is_console_file = true;
            _instance._file_mode = options::text::write;
            _instance._file_buf = stdout;

            return _instance;
        }

        static inline io& cerr()
        {
            static io _instance;
            _instance._is_console_file = true;
            _instance._file_mode = options::text::write;
            _instance._file_buf = stderr;

            return _instance;
        }

        static inline io& cin()
        {
            static io _instance;
            _instance._is_console_file = true;
            _instance._file_mode = options::text::read;
            _instance._file_buf = stdin;

            return _instance;
        }

        inline bool is_open() const
        {
            return (_file_buf != nullptr);
        }

        inline bool is_eof() const
        {
            return feof(_file_buf) != 0;
        }

        inline void close()
        {
            if (!_is_console_file && _file_buf != nullptr)
            {
                fclose(_file_buf);
                _file_buf = nullptr;
            }
        }

        inline Result<void, runtime_error> flush()
        {
            if (only_read())
            {
                fflush(_file_buf);
            }
            else
            {
                return runtime_error {
                    "Cannot flush in write or read-write mode"
                };
            }

            return {};
        }

        inline Result< reference<sstream<4096>>, runtime_error > read_line()
        {
            _io_buf.clear();
            
            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            if (fgets(_io_buf.data(), _io_buf.capacity(), _file_buf) == nullptr)
            { 
                _io_buf.clear();
            }
            
            return {_io_buf};
        }

        inline Result< reference<wsstream<4096>>, runtime_error > wread_line()
        {
            _wio_buf.clear();
            
            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            if (fgetws(_wio_buf.data(), _io_buf.capacity(), _file_buf) == nullptr)
            { 
                _wio_buf.clear();
            }
            
            return {_wio_buf};
        }

        inline Result< reference<sstream<4096>>, runtime_error > read()
        {
            _io_buf.clear();

            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            
            for (char c = fgetc(_file_buf); c != EOF && cstring::iswhitespace(c); c = fgetc(_file_buf))
            {
                _io_buf.push_back(c);
            }
            
            return {_io_buf};
        }

        template <usize N>
        inline Result< reference<wsstream<N>>, runtime_error > wread()
        {
            _wio_buf.clear();

            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            
            for (wchar c = fgetwc(_file_buf); c != EOF && wcstring::iswhitespace(c); c = fgetwc(_file_buf))
            {
                _io_buf.push_back(c);
            }

            return {_wio_buf};
        }

        template < basic_string_literal fmt, typename... Args >
        inline Result<void, runtime_error> print(Args&&... args)
        {
            if(only_read() == true)
            {
                return runtime_error {
                    "Cannot write file. It is in read mode"
                };
            }

            using char_type = typename decltype(fmt)::char_type;
            tuple<Args&...> _args_tup = {args...};
            constexpr auto _fmt_buf = sstream_detail::parse_literal<
                fmt, sizeof...(Args) + 1>().unwrap();

            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "The number of arguments doesn't match"
            );

            if constexpr (sizeof...(Args) != 0)
            {
                auto _forward_print = [&]<usize I>()
                {
                    using arg_type = decltype(_args_tup.template get<I>());
                    arg_type _arg = _args_tup.template get<I>();

                    constexpr auto _curr_fmt = 
                        format_literal<char_type, _fmt_buf[I].length + 1>
                        {
                            .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                            .tag = _fmt_buf[I].tag,
                            .foreground = _fmt_buf[I].foreground,
                            .background = _fmt_buf[I].background
                        };

                    io_detail::printer<_curr_fmt> _printer = {_file_buf};
                    _print_impl(_printer, forward<arg_type>(_arg));
                };

                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    ((_forward_print.template operator()<Ints>()), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }

            constexpr auto _last = _fmt_buf[sizeof...(Args)];
            constexpr auto _curr_fmt = 
                format_literal<char_type, _last.length + 1>
                {
                    .format = {_last.format, _last.length},
                    .tag = _last.tag,
                    .foreground = _last.foreground,
                    .background = _last.background
                };

            io_detail::printer<_curr_fmt> _printer = {_file_buf};
            
            _print_impl(_printer);
            return {};
        }
    };
} // namespace hsd

#define hsd_print(fmt, ...)\
    hsd::io::cout().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_print_err(fmt, ...)\
    hsd::io::cerr().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_println(fmt, ...)\
    hsd::io::cout().template print<fmt "\n">(__VA_ARGS__).unwrap()

#define hsd_println_err(fmt, ...)\
    hsd::io::cerr().template print<fmt "\n">(__VA_ARGS__).unwrap()
