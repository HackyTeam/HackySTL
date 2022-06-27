#pragma once

#include "FormatGenerator.hpp"
#include "SStream.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#include <windows.h>
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
    enum class io_options : DWORD
    {
        none = 0,
        read = GENERIC_READ,
        write = GENERIC_WRITE,
        read_write = GENERIC_READ | GENERIC_WRITE,
        append = FILE_APPEND_DATA,
        rw_append = GENERIC_READ | FILE_APPEND_DATA,
        rw_create = GENERIC_READ | GENERIC_WRITE | CREATE_ALWAYS
    };

    namespace io_detail
    {
        static inline auto file_error_code()
        {
            return ::GetLastError();
        }

        static inline const char* file_error_msg()
        {
            static char _msg_buf[256]{};
            
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, file_error_code(), 
                MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), 
                _msg_buf, sizeof(_msg_buf), nullptr
            );

            return _msg_buf;
        }

        class file_handler
        {
        private:
            HANDLE _handle = nullptr;
            OVERLAPPED _overlapped = {};
            bool _is_overlapped = false;
            bool _is_external = false;
            io_options _mode = io_options::none;

        public:
            inline file_handler() = default;

            inline file_handler(const file_handler&) = delete;
            inline file_handler& operator=(const file_handler&) = delete;

            inline file_handler(file_handler&& other)
            {
                swap(other._handle, _handle);
                swap(other._overlapped, _overlapped);
                swap(other._is_overlapped, _is_overlapped);
                swap(other._is_external, _is_external);
                swap(other._mode, _mode);
            }

            inline file_handler& operator=(file_handler&& other)
            {
                swap(other._handle, _handle);
                swap(other._overlapped, _overlapped);
                swap(other._is_overlapped, _is_overlapped);
                swap(other._is_external, _is_external);
                swap(other._mode, _mode);

                return *this;
            }

            inline ~file_handler()
            {
                close();
            }

            inline void close()
            {
                if (_handle != nullptr)
                {
                    if (_is_overlapped == false)
                    {
                        if (CloseHandle(_handle) == 0)
                        {
                            panic(file_error_msg());
                        }
                    }
                    else if (_is_external == false)
                    {
                        if (CancelIoEx(_handle, &_overlapped) == 0)
                        {
                            panic(file_error_msg());
                        }
                    }

                    _handle = nullptr;
                }
            }

            inline bool is_open() const
            {
                return _handle != nullptr;
            }

            inline bool only_read() const
            {
                return _mode == io_options::read;
            }

            inline bool only_write() const
            {
                return _mode == io_options::write;
            }

            inline auto write(const void* data, u64 size)
            {
                DWORD _written = 0;

                if (WriteFile(_handle, data, size, &_written, &_overlapped) == 0)
                {
                    if (file_error_code() != ERROR_IO_PENDING)
                    {
                        return static_cast<DWORD>(-1);
                    }

                    if (GetOverlappedResult(_handle, &_overlapped, &_written, true) == 0)
                    {
                        return static_cast<DWORD>(-1);
                    }
                }

                return _written;
            }

            inline auto read(void* data, u64 size)
            {
                DWORD _read = 0;

                if (ReadFile(_handle, data, size, &_read, &_overlapped) == 0)
                {
                    if (file_error_code() != ERROR_IO_PENDING)
                    {
                        return static_cast<DWORD>(-1);
                    }

                    if (GetOverlappedResult(_handle, &_overlapped, &_read, true) == 0)
                    {
                        return static_cast<DWORD>(-1);
                    }
                }

                return _read;
            }

            static inline file_handler open(
                const char* path, io_options mode)
            {
                file_handler _file;

                if (mode == io_options::rw_create)
                {
                    _file._handle = CreateFileA(
                        path,
                        static_cast<DWORD>(mode),
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        nullptr, CREATE_ALWAYS,
                        FILE_FLAG_OVERLAPPED, nullptr
                    );
                }
                else
                {
                    _file._handle = CreateFileA(
                        path, static_cast<DWORD>(mode), 
                        FILE_SHARE_READ | FILE_SHARE_WRITE, 
                        nullptr, OPEN_EXISTING, 
                        FILE_FLAG_OVERLAPPED, nullptr
                    );

                    if (file_error_code() == ERROR_FILE_NOT_FOUND)
                    {
                        _file._handle = CreateFileA(
                            path, static_cast<DWORD>(mode), 
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            nullptr, CREATE_ALWAYS, 
                            FILE_FLAG_OVERLAPPED, nullptr
                        );
                    }
                }

                _file._mode = mode;
                _file._is_overlapped = true;
                _file._is_external = true;

                return _file;
            }

            static inline file_handler get_stdin()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_INPUT_HANDLE);
                _file._mode = io_options::read;
                _file._is_overlapped = true;
                _file._is_external = true;

                return _file;
            }

            static inline file_handler get_stdout()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_OUTPUT_HANDLE);
                _file._mode = io_options::write;
                _file._is_overlapped = true;
                _file._is_external = true;

                return _file;
            }

            static inline file_handler get_stderr()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_ERROR_HANDLE);
                _file._mode = io_options::write;
                _file._is_overlapped = true;
                _file._is_external = true;

                return _file;
            }
        };        
    } // namespace io_detail
    #else
    enum class io_options : i32
    {
        none = 0,
        read       = O_RDONLY,
        write      = O_WRONLY | O_CREAT | O_TRUNC,
        append     = O_WRONLY | O_CREAT | O_APPEND,
        read_write = O_RDWR,
        rw_create  = O_RDWR | O_CREAT | O_TRUNC,
        rw_append  = O_RDWR | O_CREAT | O_APPEND
    };

    namespace io_detail
    {
        static inline auto file_error_code()
        {
            return errno;
        }

        static inline const char* file_error_msg()
        {
            return strerror(file_error_code());
        }

        class file_handler
        {
        private:
            i32 _fd = -1;
            io_options _mode = io_options::none;
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

            static inline file_handler open(
                const char* path, io_options mode)
            {
                file_handler fd;
                fd._fd = ::open(path, static_cast<i32>(mode));
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

        inline result<reference<io>, runtime_error> flush()
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
            
            if (_sz == static_cast<decltype(_sz)>(-1))
            {
                return runtime_error {io_detail::file_error_msg()};
            }
            
            return {*this};
        }

        inline result<reference<io>, runtime_error> read_chunk()
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

            if (_sz == static_cast<decltype(_sz)>(-1))
            {
                return runtime_error {io_detail::file_error_msg()};
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
                get().
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
            const char* file_path, io_options mode = io_options::read)
            -> result<io, runtime_error>
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
