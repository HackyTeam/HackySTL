#pragma once

#include "FormatGenerator.hpp"
#include "SStream.hpp"

#if defined(HSD_PLATFORM_WINDOWS)

#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#endif

namespace hsd
{
    #if defined(HSD_PLATFORM_WINDOWS)
    struct io_options {};
    #else
    struct io_options
    {
        static constexpr i32 read       = O_RDONLY;
        static constexpr i32 write      = O_WRONLY | O_CREAT | O_TRUNC;
        static constexpr i32 append     = O_WRONLY | O_CREAT | O_APPEND;
        static constexpr i32 read_write = O_RDWR;
        static constexpr i32 rw_create  = O_RDWR | O_CREAT | O_TRUNC;
        static constexpr i32 rw_append  = O_RDWR | O_CREAT | O_APPEND;
    };

    namespace io_detail
    {
        class file_handler
        {
        private:
            i32 _fd = -1;
            i32 _mode = 0;
            bool is_external = false;

        public:
            inline file_handler() = default;

            inline file_handler(const file_handler&) = delete;
            inline file_handler& operator=(const file_handler&) = delete;

            inline file_handler(file_handler&& other)
                : _fd{other._fd}, _mode{other._mode}, is_external{other.is_external}
            {
                other._fd = -1;
            }

            inline file_handler& operator=(file_handler&& rhs)
            {
                _fd = rhs._fd;
                _mode = rhs._mode;
                is_external = rhs.is_external;
                rhs._fd = -1;
                
                return *this;
            }

            inline ~file_handler()
            {
                close();
            }

            inline void close()
            {
                if (_fd != -1 && is_external == false)
                {
                    ::close(_fd);
                    _fd = -1;
                }
            }

            inline bool is_open() const
            {
                return _fd != -1;
            }

            inline bool only_read() const
            {
                return _mode == io_options::read;
            }

            inline bool only_write() const
            {
                return _mode == io_options::write;
            }

            inline isize write(const char* data, usize size)
            {
                return ::write(_fd, data, size);
            }

            inline isize read(char* data, usize size)
            {
                return ::read(_fd, data, size);
            }

            static inline file_handler open(const char* path, i32 mode)
            {
                file_handler fd;
                fd._fd = ::open(path, mode);
                fd._mode = mode;
                fd.is_external = true;

                return fd;
            }

            static inline file_handler get_stdin()
            {
                file_handler fd;
                fd._fd = 0;
                fd._mode = io_options::read;
                fd.is_external = false;

                return fd;
            }

            static inline file_handler get_stdout()
            {
                file_handler fd;
                fd._fd = 1;
                fd._mode = io_options::write;
                fd.is_external = false;

                return fd;
            }

            static inline file_handler get_stderr()
            {
                file_handler fd;
                fd._fd = 2;
                fd._mode = io_options::write;
                fd.is_external = false;

                return fd;
            }
        };  
    } // namespace io_detail
    #endif

    class io : private sstream
    {
    private:
        io_detail::file_handler _file;
        bool _is_eof = false;

        inline io(io_detail::file_handler&& file)
            : _file{move(file)}
        {}

    public:

        inline io(const io&) = delete;
        inline io& operator=(const io&) = delete;

        inline io(io&& other)
            : sstream{move(other.get_stream())}, 
            _file{move(other._file)}, _is_eof{other._is_eof} 
        {}

        inline io& operator=(io&& rhs)
        {
            swap(get_stream(), rhs.get_stream());
            swap(_file, rhs._file);
            swap(_is_eof, rhs._is_eof);

            return *this;
        }

        inline ~io()
        {
            close();
        }

        inline sstream& get_stream()
        {
            return *this;
        }

        inline bool is_open() const
        {
            return _file.is_open();
        }

        inline bool is_eof() const
        {
            return _is_eof;
        }

        inline void close()
        {
            _file.close();
        }

        inline Result< reference<io>, runtime_error > flush()
        {
            if (_file.only_read() == true)
            {
                return runtime_error {
                    "Cannot write file. It is in read mode"
                };
            }
            else if (_file.is_open() == false)
            {
                return runtime_error {
                    "Cannot write file. It is not open"
                };
            }

            auto _sz = _file.write(get_stream().data(), get_stream().size());
            get_stream().clear();
            
            if (_sz == -1)
            {
                return runtime_error {strerror(errno)};
            }
            
            return {*this};
        }

        inline Result< reference<io>, runtime_error > read_chunk()
        {
            if (_file.only_write())
            {
                return runtime_error {
                    "Cannot read file. It is in write mode"
                };
            }
            else if (_file.is_open() == false)
            {
                return runtime_error {
                    "Cannot read file. It is not open"
                };
            }

            auto _sz = _file.read(
                get_stream().data() + this->_size, 
                get_stream().capacity() - this->_size - 1
            );

            if (_sz == -1)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                //_is_eof = true;
                return runtime_error {"Undefined error"};
                #else
                return runtime_error {strerror(errno)};
                #endif
            }

            get_stream().data()[_sz + this->_size] = '\0';
            _is_eof = static_cast<usize>(_sz) < (get_stream().capacity() - 1);
            this->_size += _sz;
            
            this->_is_complete = (
                cstring::find(
                    get_stream().get_separators(), 
                    get_stream().data()[this->_size - 1]
                ) != nullptr || _is_eof == true
            );

            return {*this};
        }

        template <typename T> requires (DefaultConstructible<T>)
        inline T read_value()
        {
            if (get_stream().size() != 0)
            {
                T value{};
                get_stream().set_data(value).unwrap();
                
                return value;
            }
            else if (_is_eof == false)
            {
                T value{};
                
                read_chunk().
                unwrap().
                get_stream().
                set_data(value).
                unwrap();

                return value;
            }

            return T{};
        }

        template < basic_string_literal fmt, typename... Args >
        inline io& print(Args&&... args)
        {   
            using char_type = typename decltype(fmt)::char_type;
            using tup_type = type_tuple<Args...>;

            static constexpr auto _fmt_buf =
                parse_literal<fmt, sizeof...(Args) + 1>().unwrap();

            static constexpr auto _last = _fmt_buf[tup_type::size];
            static constexpr auto _last_fmt = 
                format_literal<char_type, _last.length + 1>
                {
                    .format = {_last.format, _last.length},
                    .base = _last.base
                };


            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "The number of arguments doesn't match"
            );

            if constexpr (sizeof...(Args) != 0)
            {
                static constexpr auto _print_fmt =
                    []<usize... Ints>(index_sequence<Ints...>)
                    {
                        constexpr auto _gen_fmt = []<usize I>()
                        {
                            constexpr auto _curr_fmt = 
                                format_literal<char_type, _fmt_buf[I].length + 1>
                                {
                                    .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                                    .base = _fmt_buf[I].base
                                };

                            
                            constexpr auto _pretty_fmt = 
                                format<_curr_fmt, typename tup_type::template type_at<I>>();
                            
                            constexpr auto _sz = 
                                basic_cstring<char_type>::length(_pretty_fmt.data);
                            
                            static_assert(_pretty_fmt.size() == _sz + 1);
                            return _pretty_fmt;
                        };

                        return ((_gen_fmt.template operator()<Ints>()) + ...);
                    }(make_index_sequence<tup_type::size>{}) + _last_fmt.format;

                const tuple _args = (make_args(args) + ...);

                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    this->_size += snprintf(
                        get_stream().data() + this->_size, 
                        get_stream().capacity() - this->_size, 
                        _print_fmt.data, _args.template get<Ints>()...
                    );
                }(make_index_sequence<decltype(_args)::size()>{});
            }
            else
            {
                this->_size += snprintf(
                    get_stream().data() + this->_size, 
                    get_stream().capacity() - this->_size, 
                    "%s", _last_fmt.format.data
                );
            }

            return *this;
        }

        static inline auto& cout()
        {
            static io _cout{io_detail::file_handler::get_stdout()};
            return _cout;
        }

        static inline auto& cerr()
        {
            static io _cerr{io_detail::file_handler::get_stderr()};
            return _cerr;
        }

        static inline auto& cin()
        {
            static io _cin{io_detail::file_handler::get_stdin()};
            return _cin;
        }

        static inline auto load_file(
            const char* file_path, i32 mode = io_options::read)
            -> Result<io, runtime_error>
        {
            auto _file = io_detail::file_handler::open(file_path, mode);

            if (_file.is_open() == false)
            {
                return runtime_error {
                    "Cannot open file. It is not open"
                };
            }

            return io{move(_file)};
        }
    };

    namespace format_literals
    {
        template <hsd::basic_string_literal fmt>
        static constexpr auto operator""_fmt()
        {
            return [] { return fmt; };
        }
    } // namespace format_literals

    template <typename T, typename... Args>
    static void print(T&&, Args&&... args)
    {
        if (hsd::io::cout().get_stream().capacity() == 0)
        {
            hsd::io::cout().get_stream().reserve(1024);
        }

        static constexpr T _func{};

        hsd::io::cout().template 
        print<_func()>(hsd::forward<Args>(args)...).
        flush().
        unwrap();
    }

    template <typename T, typename... Args>
    static void println(T&&, Args&&... args)
    {
        if (hsd::io::cout().get_stream().capacity() == 0)
        {
            hsd::io::cout().get_stream().reserve(1024);
        }

        static constexpr T _func{};

        hsd::io::cout().template 
        print<_func() + "\n">(hsd::forward<Args>(args)...).
        flush().
        unwrap();
    }

    template <typename T, typename... Args>
    static void print_err(T&&, Args&&... args)
    {
        if (hsd::io::cerr().get_stream().capacity() == 0)
        {
            hsd::io::cerr().get_stream().reserve(1024);
        }

        static constexpr T _func{};

        hsd::io::cerr().template 
        print<_func()>(hsd::forward<Args>(args)...).
        flush().
        unwrap();
    }

    template <typename T, typename... Args>
    static void println_err(T&&, Args&&... args)
    {
        if (hsd::io::cerr().get_stream().capacity() == 0)
        {
            hsd::io::cerr().get_stream().reserve(1024);
        }

        static constexpr T _func{};

        hsd::io::cerr().template 
        print<_func() + "\n">(hsd::forward<Args>(args)...).
        flush().
        unwrap();
    }
} // namespace hsd
