#pragma once

/// @file Io.hpp
/// @brief File, console, buffered input, and formatted output operations.
/// @details Provides the project io object, typed reads, line/chunk handling,
/// stream flushing, and print/println helpers with explicit error results.

#include "_IoDetail.hpp"

#include "SStream.hpp"
#include "../Containers/String.hpp"

namespace hsd
{
    /// @brief Buffered file/console handle combining formatted output with typed,
    /// separator-aware input.
    /// @details Inherits its character buffer privately from `sstream`; reads are
    /// staged into that buffer via `read_chunk`/`read_line_chunk` before being
    /// parsed, and writes are staged into it by `print` before `flush` commits
    /// them to the underlying `_file`. Move-only, matching the underlying
    /// `io_detail::file_handler`.
    class io : private sstream
    {
    private:
        /// @brief Platform file/stream handle used for the actual reads and writes.
        io_detail::file_handler _file;
        /// @brief Set once a read reports fewer bytes than requested (end of file).
        bool _is_eof = false;

        /// @brief Constructs a stream around an already-open file handle.
        /// @param file Handle to take ownership of; reserves a 1024-byte buffer.
        inline io(io_detail::file_handler&& file)
            : _file{hsd::move(file)}
        {
            get_stream().reserve(1024);
        }

        /// @brief Accesses the inherited character buffer.
        /// @return Mutable reference to `*this` viewed as an `sstream`.
        inline sstream& get_stream()
        {
            return *this;
        }

        /// @brief Accesses the inherited character buffer.
        /// @return Const reference to `*this` viewed as an `sstream`.
        inline const sstream& get_stream() const
        {
            return *this;
        }

        /// @brief Tests whether `ch` separates values in the input.
        /// @param ch Character to test.
        /// @return True for space, tab, newline and carriage return.
        static constexpr bool _is_space(char ch)
        {
            return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
        }

    public:
        /// @brief Copying is disabled; a file handle cannot be duplicated.
        inline io(const io&) = delete;
        inline io& operator=(const io&) = delete;

        /// @brief Transfers the buffer, file handle, and eof state from `other`.
        /// @param other Stream left in a moved-from state.
        inline io(io&& other)
            : sstream{hsd::move(other.get_stream())}, 
            _file{hsd::move(other._file)}, _is_eof{other._is_eof} 
        {}

        /// @brief Swaps the buffer, file handle, and eof state with `rhs`.
        /// @param rhs Stream to exchange state with.
        /// @return `*this` after the swap.
        inline io& operator=(io&& rhs)
        {
            hsd::swap(get_stream(), rhs.get_stream());
            hsd::swap(_file, rhs._file);
            hsd::swap(_is_eof, rhs._is_eof);

            return *this;
        }

        /// @brief Closes the underlying file handle.
        inline ~io()
        {
            close();
        }

        /// @brief Reports whether the underlying file handle is open.
        /// @return `true` if the file/stream is currently open.
        inline bool is_open() const
        {
            return _file.is_open();
        }

        /// @brief Reports whether the last read reached end of file.
        /// @return Value of the internal eof flag, set by `read_chunk`/`read_line_chunk`.
        inline bool is_eof() const
        {
            return _is_eof;
        }

        /// @brief Closes the underlying file handle.
        /// @details Safe to call multiple times.
        inline void close()
        {
            _file.close();
        }

        /// @brief Reserves buffer capacity for subsequent reads and writes.
        /// @param capacity Minimum number of characters the buffer should hold.
        inline void set_stream_capacity(usize capacity)
        {
            get_stream().reserve(capacity);
        }

        /// @brief Exposes the currently buffered data without consuming it.
        /// @return A view over the initialized portion of the internal buffer.
        inline const string_view get_stream_data() const
        {
            return get_stream();
        }

        /// @brief Parses buffered, separator-delimited fields into `args`.
        /// @tparam Args Destination types assigned from successive fields.
        /// @param args Destination objects to assign.
        /// @return Empty success, or a runtime error forwarded from `sstream::set_data`.
        template <typename... Args>
        inline auto set_data(Args&... args)
        {
            return get_stream().set_data(hsd::forward<Args&>(args)...);
        }

        /// @brief Writes the buffered contents to the file and clears the buffer.
        /// @return A reference to `*this` on success, or a runtime error if the
        /// handle is read-only, not open, or the underlying write call fails.
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

        /// @brief Reads raw data from the file into the buffer.
        /// @return A reference to `*this` on success, or a runtime error if the
        /// handle is write-only, not open, or the underlying read call fails.
        /// @details Appends into the unused tail of the buffer, growing the buffer
        /// first when it is full. Sets `is_eof` only when the read returns no bytes,
        /// so short reads from pipes and terminals do not end the input early.
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

            if (get_stream().size() == get_stream().capacity())
            {
                get_stream().reserve(get_stream().capacity() * 2 + 64);
            }

            auto _sz = _file.read(
                this->get_current_ptr(), 
                this->get_remaining_capacity()
            );

            if (_sz == static_cast<decltype(_sz)>(-1))
            {
                return runtime_error {io_detail::file_error_msg()};
            }

            this->advance_size(static_cast<usize>(_sz));
            _is_eof = (_sz == 0);
            return {*this};
        }

        /// @brief Reads raw data from the file, exactly like `read_chunk`.
        /// @return A reference to `*this` on success, or a runtime error.
        /// @details Kept for existing callers. Line-oriented reading no longer
        /// seeks the file back to a separator, because that fails on pipes.
        inline result<reference<io>, runtime_error> read_line_chunk()
        {
            return read_chunk();
        }

        /// @brief Reads and parses the next whitespace-delimited value.
        /// @tparam T Destination type; must be default-constructible.
        /// @return The parsed value, or a runtime error when the text cannot be
        /// parsed, the underlying read fails, or the input is already exhausted.
        /// @details Leading whitespace is skipped. A value at the very end of the
        /// input ends at end of file, so it does not need a trailing separator.
        template <DefaultConstructible T>
        inline result<T, runtime_error> read_value()
        {
            using ret_type = result<T, runtime_error>;

            while (true)
            {
                const char* _data = get_stream().data();
                const usize _size = get_stream().size();

                usize _start = 0;
                while (_start < _size && _is_space(_data[_start]))
                {
                    ++_start;
                }

                usize _end = _start;
                while (_end < _size && !_is_space(_data[_end]))
                {
                    ++_end;
                }

                if (_start == _size && _is_eof)
                {
                    return runtime_error{"Reached end of file"};
                }

                if (_end < _size || _is_eof)
                {
                    T _value{};
                    using sstream_detail::_parse_from;
                    auto _res = _parse_from(
                        string_view{_data + _start, _end - _start}, _value);

                    get_stream().drop_front(_end);

                    return _res.is_ok() ? ret_type{hsd::move(_value)}
                        : ret_type{_res.unwrap_err()};
                }

                get_stream().drop_front(_start);

                auto _read_res = read_chunk();
                if (_read_res.is_ok() == false)
                {
                    return {_read_res.unwrap_err()};
                }
            }
        }

        /// @brief Reads and returns the next line, which ends at a newline.
        /// @return The line's text without its newline (and without one trailing
        /// carriage return), or a runtime error when the input is exhausted or
        /// the underlying read fails.
        /// @details Blank lines are returned as empty text. A final line without
        /// a newline is returned at end of file.
        inline result<string, runtime_error> read_line()
        {
            usize _scanned = 0;

            while (true)
            {
                const char* _data = get_stream().data();
                const usize _size = get_stream().size();

                for (; _scanned < _size; ++_scanned)
                {
                    if (_data[_scanned] == '\n')
                    {
                        usize _length = _scanned;
                        if (_length != 0 && _data[_length - 1] == '\r')
                        {
                            --_length;
                        }

                        string _line{string_view{_data, _length}};
                        get_stream().drop_front(_scanned + 1);
                        return _line;
                    }
                }

                if (_is_eof)
                {
                    if (_size == 0)
                    {
                        return runtime_error{"Reached end of file"};
                    }

                    usize _length = _size;
                    if (_data[_length - 1] == '\r')
                    {
                        --_length;
                    }

                    string _line{string_view{_data, _length}};
                    get_stream().clear();
                    return _line;
                }

                auto _read_res = read_chunk();
                if (_read_res.is_ok() == false)
                {
                    return {_read_res.unwrap_err()};
                }
            }
        }

        /// @brief Appends formatted output to the internal buffer.
        /// @tparam T Compile-time format-literal callable; its value is not read.
        /// @tparam Args Types of the values consumed by the format description.
        /// @param args Values formatted according to `T`'s literal.
        /// @return `*this`, to allow chaining with `flush`.
        template <typename T, typename... Args>
        inline io& print(T&&, Args&&... args)
        {   
            get_stream().append_data(T{}, hsd::forward<Args>(args)...);
            return *this;
        }

        /// @brief Returns the process-wide standard output stream.
        /// @return Reference to a function-local static `io` wrapping stdout.
        static inline auto& cout()
        {
            static io _cout{io_detail::file_handler::get_stdout()};
            return _cout;
        }

        /// @brief Returns the process-wide standard error stream.
        /// @return Reference to a function-local static `io` wrapping stderr.
        static inline auto& cerr()
        {
            static io _cerr{io_detail::file_handler::get_stderr()};
            return _cerr;
        }

        /// @brief Returns the process-wide standard input stream.
        /// @return Reference to a function-local static `io` wrapping stdin.
        static inline auto& cin()
        {
            static io _cin{io_detail::file_handler::get_stdin()};
            return _cin;
        }

        /// @brief Opens a file at `file_path` for I/O.
        /// @param file_path Null-terminated path to the file.
        /// @param mode Access mode used to open the file; defaults to read-only.
        /// @return An owning `io` wrapping the opened file, or a runtime error
        /// when the file could not be opened.
        static inline auto load_file(
            const char* file_path, io_options mode = io_options::read)
            -> result<io, runtime_error>
        {
            auto _file = io_detail::file_handler::open(file_path, mode);

            if (_file.is_open() == false)
            {
                return runtime_error {
                    "Cannot open file. File not found"
                };
            }

            return io{hsd::move(_file)};
        }
    };

    /// @brief Writes formatted output to standard output.
    /// @tparam T Compile-time format-literal callable; its value is not read.
    /// @tparam Args Types of the values consumed by the format description.
    /// @param args Values formatted according to `T`'s literal.
    /// @details Flushes immediately and panics (via `.unwrap()`) if the flush fails.
    template <typename T, typename... Args>
    static void print(T&&, Args&&... args)
    {
        hsd::io::cout()
        .print(T{}, hsd::forward<Args>(args)...)
        .flush()
        .unwrap();
    }

    /// @brief Writes formatted output followed by a newline to standard output.
    /// @tparam T Compile-time format-literal callable; its value is not read.
    /// @tparam Args Types of the values consumed by the format description.
    /// @param args Values formatted according to `T`'s literal.
    /// @details Flushes immediately and panics (via `.unwrap()`) if the flush fails.
    template <typename T, typename... Args>
    static void println(T&&, Args&&... args)
    {
        static constexpr T _func{};
        static constexpr auto _fmt = hsd::format_literals::operator""_fmt<_func() + "\n">();

        hsd::io::cout()
        .print(_fmt, hsd::forward<Args>(args)...)
        .flush()
        .unwrap();
    }

    /// @brief Writes formatted output to standard error.
    /// @tparam T Compile-time format-literal callable; its value is not read.
    /// @tparam Args Types of the values consumed by the format description.
    /// @param args Values formatted according to `T`'s literal.
    /// @details Flushes immediately and panics (via `.unwrap()`) if the flush fails.
    template <typename T, typename... Args>
    static void print_err(T&&, Args&&... args)
    {
        hsd::io::cerr()
        .print(T{}, hsd::forward<Args>(args)...)
        .flush()
        .unwrap();
    }

    /// @brief Writes formatted output followed by a newline to standard error.
    /// @tparam T Compile-time format-literal callable; its value is not read.
    /// @tparam Args Types of the values consumed by the format description.
    /// @param args Values formatted according to `T`'s literal.
    /// @details Flushes immediately and panics (via `.unwrap()`) if the flush fails.
    template <typename T, typename... Args>
    static void println_err(T&&, Args&&... args)
    {
        static constexpr T _func{};
        static constexpr auto _fmt = hsd::format_literals::operator""_fmt<_func() + "\n">();

        hsd::io::cerr()
        .print(_fmt, hsd::forward<Args>(args)...)
        .flush()
        .unwrap();
    }
} // namespace hsd
