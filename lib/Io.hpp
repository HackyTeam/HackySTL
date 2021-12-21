#pragma once

#include "_IoDetail.hpp"
#include "String.hpp"

namespace hsd
{
    struct io_options
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

    template <typename CharT>
    class io
    {
    protected:
        FILE* _file_buf = nullptr;
        const char* _file_mode = "";
        bool _is_console_file = false;
        basic_sstream<CharT> _io_buf;

    private:
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

        inline auto _read_chr(CharT& chr, const CharT* sep) -> bool
        {
            if constexpr(is_same<CharT, char>::value)
            {
                chr = static_cast<CharT>(fgetc(_file_buf));
                return (chr != EOF && basic_cstring<CharT>::find(sep, chr) == nullptr);
            }
            else if constexpr(is_same<CharT, wchar>::value)
            {
                chr = static_cast<CharT>(fgetwc(_file_buf));
                return (chr != WEOF && basic_cstring<CharT>::find(sep, chr) == nullptr);
            }
            else
            {
                return false;
            }
        };

        inline auto _unget_chr(CharT chr)
        {
            if constexpr(is_same<CharT, char>::value)
            {
                ungetc(chr, _file_buf);
            }
            else if constexpr(is_same<CharT, wchar>::value)
            {
                ungetwc(chr, _file_buf);
            }
        };

    public:

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

        inline void set_buffer_capacity(usize capacity)
        {
            _io_buf.reserve(capacity);
        }

        inline void set_separators(const CharT* separators)
        {
            _io_buf.set_separators(separators);
        }

        inline usize buffer_capacity() const
        {
            return _io_buf.capacity();
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

        inline Result< reference<basic_sstream<CharT>>, runtime_error > read_line()
        {
            _io_buf.clear();
            
            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }

            if constexpr(is_same<CharT, char>::value)
            {
                if (fgets(_io_buf.data(), _io_buf.capacity(), _file_buf) == nullptr)
                { 
                    _io_buf.clear();
                }
            }
            else if constexpr(is_same<CharT, wchar>::value)
            {
                if (fgetws(_io_buf.data(), _io_buf.capacity(), _file_buf) == nullptr)
                {
                    _io_buf.clear();
                }
            }
            else
            {
                return runtime_error {
                    "Unsupported character type"
                };
            }
            
            return {_io_buf};
        }

        inline Result< reference<basic_sstream<CharT>>, runtime_error > read()
        {
            _io_buf.clear();
            CharT _chr;

            if (only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            
            while (_chr != EOF && !_read_chr(_chr, _io_buf.get_separators()))
                ;
            
            if (_chr != EOF) 
            {
                _unget_chr(_chr);

                while (_read_chr(_chr, _io_buf.get_separators()))
                {
                    _io_buf.push_back(_chr);
                }
            }

            return {_io_buf};
        }

        template <typename T>
        inline T read_value()
        {
            if (read().is_ok())
            {
                T value;
                _io_buf.set_data(value).unwrap();
                
                return value;
            }

            return T{};
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
            static_assert(
                is_same<char_type, CharT>::value, 
                "Unsupported character type"
            );
            
            tuple _args_tup = {forward<Args&>(args)...};
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

    namespace io_detail
    {
        template <typename CharT>
        struct warpper : public io<CharT>
        {
            inline warpper() = default;

            inline warpper(usize capacity)
            {
                this->set_buffer_capacity(capacity);
            }

            inline void set_file_buf(FILE* file_buf)
            {
                this->_file_buf = file_buf;
            }
            
            inline void set_is_console_file(bool is_console_file)
            {
                this->_is_console_file = is_console_file;
            }

            inline void set_file_mode(const char* file_mode)
            {
                this->_file_mode = file_mode;
            }

            inline io<CharT>& to_base()
            {
                return *this;
            }
        };
    } // namespace io_detail
    

    template <typename CharT>
    static inline auto& cout()
    {
        static io_detail::warpper<CharT> _instance{};
        _instance.set_is_console_file(true);
        _instance.set_file_mode(io_options::text::write);
        _instance.set_file_buf(stdout);

        return _instance.to_base();
    }

    template <typename CharT>
    static inline io<CharT>& cerr()
    {
        static io_detail::warpper<CharT> _instance{};
        _instance.set_is_console_file(true);
        _instance.set_file_mode(io_options::text::write);
        _instance.set_file_buf(stderr);

        return _instance.to_base();
    }

    template <typename CharT>
    static inline io<CharT>& cin()
    {
        static io_detail::warpper<CharT> _instance{1024};
        _instance.set_is_console_file(true);
        _instance.set_file_mode(io_options::text::read);
        _instance.set_file_buf(stdin);

        return _instance;
    }

    template <typename CharT>
    static inline auto load_file(const char* file_path, 
        const char* open_option = io_options::text::read)
        -> Result<io<CharT>, runtime_error>
    {
        static io_detail::warpper<CharT> _instance{};
        _instance.set_is_console_file(false);
        _instance.set_file_mode(open_option);
        
        auto* _file_buf = fopen(file_path, open_option);
        
        if (_file_buf == nullptr)
        {
            return runtime_error{"File not found"};
        }

        _instance.set_file_buf(_file_buf);
        return move(_instance.to_base());
    }
} // namespace hsd

#define hsd_read_line()\
    hsd::cin<char>().template read_line().unwrap()

#define hsd_read()\
    hsd::cin<char>().template read().unwrap()

#define hsd_read_value(value)\
    value = hsd::cin<char>().template read_value<hsd::remove_cvref_t<decltype(value)>>()

#define hsd_wread_line()\
    hsd::cin<hsd::wchar>().template read_line().unwrap()

#define hsd_wread()\
    hsd::cin<hsd::wchar>().template read().unwrap()

#define hsd_wread_value(value)\
    value = hsd::cin<hsd::wchar>().template read_value<hsd::remove_cvref_t<decltype(value)>>()

#define hsd_print(fmt, ...)\
    hsd::cout<char>().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_print_err(fmt, ...)\
    hsd::cerr<char>().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_println(fmt, ...)\
    hsd::cout<char>().template print<fmt "\n">(__VA_ARGS__).unwrap()

#define hsd_println_err(fmt, ...)\
    hsd::cerr<char>().template print<fmt "\n">(__VA_ARGS__).unwrap()

#define hsd_wprint(fmt, ...)\
    hsd::cout<hsd::wchar>().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_wprint_err(fmt, ...)\
    hsd::cerr<hsd::wchar>().template print<fmt>(__VA_ARGS__).unwrap()

#define hsd_wprintln(fmt, ...)\
    hsd::cout<hsd::wchar>().template print<fmt L"\n">(__VA_ARGS__).unwrap()

#define hsd_wprintln_err(fmt, ...)\
    hsd::cerr<hsd::wchar>().template print<fmt L"\n">(__VA_ARGS__).unwrap()
