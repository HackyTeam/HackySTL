#pragma once

/// @file _IoDetail.hpp
/// @brief Internal platform-specific file and standard-stream handlers.
/// @details Encapsulates POSIX descriptors and Windows handles behind the
/// common read, write, seek, and ownership operations used by Io.
#include "../Base/Types.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#include "../Base/CString.hpp"
#include "../Base/Limits.hpp"
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
    /// @brief Win32 file access/creation flags accepted by `file_handler::open`.
    enum class io_options : DWORD
    {
        /// @brief No access flags; used only as the default-constructed state.
        none = 0,
        /// @brief Open for reading only.
        read = GENERIC_READ,
        /// @brief Open for writing only.
        write = GENERIC_WRITE,
        /// @brief Open for both reading and writing.
        read_write = GENERIC_READ | GENERIC_WRITE,
        /// @brief Open for writing, appending to existing content.
        append = FILE_APPEND_DATA,
        /// @brief Open for reading and appending.
        rw_append = GENERIC_READ | FILE_APPEND_DATA,
        /// @brief Open for reading and writing, (re)creating the file.
        rw_create = GENERIC_READ | GENERIC_WRITE | CREATE_ALWAYS
    };

    /// @brief Implementation details backing `io` on Windows.
    namespace io_detail
    {
        /// @brief Returns the last Win32 error code for the calling thread.
        /// @return Value from `GetLastError()`.
        static inline auto file_error_code()
        {
            return ::GetLastError();
        }

        /// @brief Formats the last Win32 error code as a human-readable message.
        /// @return Pointer to a static, thread-unsafe message buffer valid until
        /// the next call to this function.
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

        /// @brief Owning wrapper over a Win32 `HANDLE` performing overlapped I/O.
        /// @details Move-only; the handle (or pending overlapped operation) is
        /// released by `close`, called automatically on destruction.
        class file_handler
        {
        private:
            /// @brief Underlying file, pipe, or console handle; `nullptr` when closed.
            HANDLE _handle = nullptr;
            /// @brief Overlapped-I/O state shared by `read`/`write`/`read_line`.
            /// @details `Offset`/`OffsetHigh` hold the position of the next transfer.
            OVERLAPPED _overlapped = {};
            /// @brief Whether `_handle` is a borrowed standard-stream handle.
            /// @details The process owns external handles, so they are never closed
            /// here. Every other handle is closed on destruction.
            bool _is_external = false;
            /// @brief Access mode the handle was opened with.
            io_options _mode = io_options::none;

        public:
            /// @brief Constructs a handler with no open handle.
            inline file_handler() = default;

            inline file_handler(const file_handler&) = delete;
            inline file_handler& operator=(const file_handler&) = delete;

            /// @brief Transfers ownership of the handle and state from `other`.
            /// @param other Handler whose state is swapped into `*this`.
            inline file_handler(file_handler&& other)
            {
                hsd::swap(other._handle, _handle);
                hsd::swap(other._overlapped, _overlapped);
                hsd::swap(other._is_external, _is_external);
                hsd::swap(other._mode, _mode);
            }

            /// @brief Transfers ownership of the handle and state from `other`.
            /// @param other Handler whose state is swapped into `*this`.
            /// @return `*this` after the transfer.
            inline file_handler& operator=(file_handler&& other)
            {
                hsd::swap(other._handle, _handle);
                hsd::swap(other._overlapped, _overlapped);
                hsd::swap(other._is_external, _is_external);
                hsd::swap(other._mode, _mode);

                return *this;
            }

            /// @brief Closes the handle, if still open.
            inline ~file_handler()
            {
                close();
            }

            /// @brief Releases the handle. A borrowed standard handle is only forgotten.
            /// @details Panics if the underlying `CloseHandle` call fails.
            /// Safe to call multiple times; a no-op once already closed.
            inline void close()
            {
                if (_handle != nullptr)
                {
                    if (_is_external == false && CloseHandle(_handle) == 0)
                    {
                        panic(file_error_msg());
                    }

                    _handle = nullptr;
                }
            }

            /// @brief Reports whether a handle is currently held.
            /// @return `true` if `_handle` is non-null.
            inline bool is_open() const
            {
                return _handle != nullptr;
            }

            /// @brief Reports whether the handle was opened in read-only mode.
            /// @return `true` if opened with `io_options::read`.
            inline bool only_read() const
            {
                return _mode == io_options::read;
            }

            /// @brief Reports whether the handle was opened in write-only mode.
            /// @return `true` if opened with `io_options::write`.
            inline bool only_write() const
            {
                return _mode == io_options::write;
            }

            /// @brief Writes `size` bytes from `data` to the handle.
            /// @param data Pointer to the bytes to write.
            /// @param size Number of bytes to write; at most `limits<DWORD>::max`.
            /// @return Number of bytes written, or `static_cast<DWORD>(-1)` on failure.
            /// @details Waits for completion of overlapped writes via `GetOverlappedResult`.
            /// A larger request fails instead of writing a truncated count.
            inline auto write(const void* data, u64 size)
            {
                DWORD _written = 0;

                if (size > limits<DWORD>::max)
                {
                    SetLastError(ERROR_FILE_TOO_LARGE);
                    return static_cast<DWORD>(-1);
                }

                if (WriteFile(_handle, data, static_cast<DWORD>(size), &_written, &_overlapped) == 0)
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

                advance(_written);
                return _written;
            }

            /// @brief Reads up to `size` bytes from the handle into `data`.
            /// @param data Destination buffer for the read bytes.
            /// @param size Capacity of `data` in bytes. At most `limits<DWORD>::max`
            /// bytes are read per call; callers already accept short reads.
            /// @return Number of bytes read, `0` at end of file, or
            /// `static_cast<DWORD>(-1)` on failure.
            /// @details Waits for completion of overlapped reads via `GetOverlappedResult`.
            inline auto read(void* data, u64 size)
            {
                DWORD _read = 0;
                const DWORD _count = (size > limits<DWORD>::max)
                    ? limits<DWORD>::max
                    : static_cast<DWORD>(size);

                if (ReadFile(_handle, data, _count, &_read, &_overlapped) == 0)
                {
                    // An overlapped read that starts at or past the end fails with
                    // ERROR_HANDLE_EOF, and that is an empty read, not an error.
                    if (file_error_code() == ERROR_HANDLE_EOF)
                    {
                        return static_cast<DWORD>(0);
                    }

                    if (file_error_code() != ERROR_IO_PENDING)
                    {
                        return static_cast<DWORD>(-1);
                    }

                    if (GetOverlappedResult(_handle, &_overlapped, &_read, true) == 0)
                    {
                        if (file_error_code() == ERROR_HANDLE_EOF)
                        {
                            return static_cast<DWORD>(0);
                        }

                        return static_cast<DWORD>(-1);
                    }
                }

                advance(_read);
                return _read;
            }

            /// @brief Moves the file position past `count` bytes that were transferred.
            /// @param count Number of bytes the last transfer moved.
            /// @details The system does not move the position of an overlapped file,
            /// so the next transfer starts where this one stopped. Borrowed standard
            /// handles are left as they are.
            inline void advance(DWORD count)
            {
                if (_is_external == false)
                {
                    const u64 _next = ((static_cast<u64>(_overlapped.OffsetHigh) << 32) | _overlapped.Offset) + count;
                    _overlapped.Offset = static_cast<DWORD>(_next);
                    _overlapped.OffsetHigh = static_cast<DWORD>(_next >> 32);
                }
            }

            
            /// @brief Opens the file at `path` for overlapped I/O.
            /// @param path Null-terminated path to the file.
            /// @param mode Access mode. Write modes create the file, and `write` and
            /// `rw_create` also empty an existing one. Read modes need the file to exist.
            /// These match the POSIX open flags.
            /// @return A `file_handler` owning the resulting handle. The caller
            /// must check `is_open()`, since `CreateFileA` failures are not
            /// otherwise reported here.
            static inline file_handler open(
                const char* path, io_options mode)
            {
                file_handler _file;

                DWORD _disposition = OPEN_EXISTING;

                if (mode == io_options::write || mode == io_options::rw_create)
                {
                    _disposition = CREATE_ALWAYS;
                }
                else if (mode == io_options::append || mode == io_options::rw_append)
                {
                    _disposition = OPEN_ALWAYS;
                }

                _file._handle = CreateFileA(
                    path, static_cast<DWORD>(mode),
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, _disposition,
                    FILE_FLAG_OVERLAPPED, nullptr
                );

                // A failed CreateFileA returns INVALID_HANDLE_VALUE, which must not
                // look like an open handle.
                if (_file._handle == INVALID_HANDLE_VALUE)
                {
                    _file._handle = nullptr;
                }

                _file._mode = mode;
                _file._is_external = false;

                return _file;
            }

            /// @brief Wraps the process's standard input handle.
            /// @return A non-owning, read-mode `file_handler` over `STD_INPUT_HANDLE`.
            static inline file_handler get_stdin()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_INPUT_HANDLE);
                _file._mode = io_options::read;
                _file._is_external = true;

                return _file;
            }

            /// @brief Wraps the process's standard output handle.
            /// @return A non-owning, write-mode `file_handler` over `STD_OUTPUT_HANDLE`.
            static inline file_handler get_stdout()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_OUTPUT_HANDLE);
                _file._mode = io_options::write;
                _file._is_external = true;

                return _file;
            }

            /// @brief Wraps the process's standard error handle.
            /// @return A non-owning, write-mode `file_handler` over `STD_ERROR_HANDLE`.
            static inline file_handler get_stderr()
            {
                file_handler _file;

                _file._handle = GetStdHandle(STD_ERROR_HANDLE);
                _file._mode = io_options::write;
                _file._is_external = true;

                return _file;
            }
        };        
    } // namespace io_detail
    #else
    /// @brief POSIX `open(2)` flag combinations accepted by `file_handler::open`.
    enum class io_options : i32
    {
        /// @brief No access flags; used only as the default-constructed state.
        none = 0,
        /// @brief Open existing file for reading only.
        read       = O_RDONLY,
        /// @brief Open for writing only, creating or truncating the file.
        write      = O_WRONLY | O_CREAT | O_TRUNC,
        /// @brief Open for writing only, appending to existing content.
        append     = O_WRONLY | O_CREAT | O_APPEND,
        /// @brief Open existing file for both reading and writing.
        read_write = O_RDWR,
        /// @brief Open for reading and writing, creating or truncating the file.
        rw_create  = O_RDWR | O_CREAT | O_TRUNC,
        /// @brief Open for reading and appending.
        rw_append  = O_RDWR | O_CREAT | O_APPEND
    };

    /// @brief Implementation details backing `io` on POSIX platforms.
    namespace io_detail
    {
        /// @brief Returns the last `errno` value set by a failed syscall.
        /// @return Current value of `errno`.
        static inline auto file_error_code()
        {
            return errno;
        }

        /// @brief Formats the last `errno` value as a human-readable message.
        /// @return Pointer to the C library's static `strerror` message buffer.
        static inline const char* file_error_msg()
        {
            return strerror(file_error_code());
        }

        /// @brief Owning wrapper over a POSIX file descriptor.
        /// @details Move-only; the descriptor is released by `close`, called
        /// automatically on destruction. Descriptors marked external (the
        /// standard streams) are never actually closed.
        class file_handler
        {
        private:
            /// @brief Underlying file descriptor; `-1` when closed.
            i32 _fd = -1;
            /// @brief Access mode the descriptor was opened with.
            io_options _mode = io_options::none;
            /// @brief Whether `_fd` is a borrowed standard-stream descriptor.
            /// @details External descriptors are left open by `close`, since they
            /// are not owned by this instance.
            bool _is_external = false;

        public:
            /// @brief Constructs a handler with no open descriptor.
            inline file_handler() = default;

            inline file_handler(const file_handler&) = delete;
            inline file_handler& operator=(const file_handler&) = delete;

            /// @brief Transfers ownership of the descriptor from `other`.
            /// @param other Handler whose descriptor is taken; left closed (`-1`).
            inline file_handler(file_handler&& other)
                : _fd{other._fd}, _mode{other._mode}, _is_external{other._is_external}
            {
                other._fd = -1;
            }

            /// @brief Transfers ownership of the descriptor from `rhs`.
            /// @param rhs Handler whose descriptor is taken; left closed (`-1`).
            /// @return `*this` after the transfer.
            /// @details Does not close any descriptor previously held by `*this`.
            inline file_handler& operator=(file_handler&& rhs)
            {
                _fd = rhs._fd;
                _mode = rhs._mode;
                _is_external = rhs._is_external;
                rhs._fd = -1;
                
                return *this;
            }

            /// @brief Closes the descriptor, if still open and not external.
            inline ~file_handler()
            {
                close();
            }

            /// @brief Releases the descriptor via `::close`, unless it is external.
            /// @details Safe to call multiple times; a no-op once already closed.
            inline void close()
            {
                if (_fd != -1 && _is_external == false)
                {
                    ::close(_fd);
                    _fd = -1;
                }
            }

            /// @brief Reports whether a descriptor is currently held.
            /// @return `true` if `_fd != -1`.
            inline bool is_open() const
            {
                return _fd != -1;
            }

            /// @brief Reports whether the descriptor was opened in read-only mode.
            /// @return `true` if opened with `io_options::read`.
            inline bool only_read() const
            {
                return _mode == io_options::read;
            }

            /// @brief Reports whether the descriptor was opened in write-only mode.
            /// @return `true` if opened with `io_options::write`.
            inline bool only_write() const
            {
                return _mode == io_options::write;
            }

            /// @brief Writes `size` bytes from `data` to the descriptor.
            /// @param data Pointer to the bytes to write.
            /// @param size Number of bytes to write.
            /// @return Number of bytes written, or `-1` on failure (see `errno`).
            inline isize write(const char* data, usize size)
            {
                return ::write(_fd, data, size);
            }

            /// @brief Reads up to `size` bytes from the descriptor into `data`.
            /// @param data Destination buffer for the read bytes.
            /// @param size Capacity of `data` in bytes.
            /// @return Number of bytes read, `0` at end of file, or `-1` on
            /// failure (see `errno`).
            inline isize read(char* data, usize size)
            {
                return ::read(_fd, data, size);
            }

            /// @brief Opens (or creates) the file at `path` with `::open`.
            /// @param path Null-terminated path to the file.
            /// @param mode Access mode passed through to `::open`.
            /// @return A `file_handler` owning the resulting descriptor. The
            /// caller must check `is_open()`, since `::open` failures are not
            /// otherwise reported here.
            static inline file_handler open(
                const char* path, io_options mode)
            {
                file_handler fd;
                // Files created here get rw-r--r-- before the umask, like `creat`.
                fd._fd = ::open(path, static_cast<i32>(mode), 0666);
                fd._mode = mode;
                // The descriptor belongs to this handler, so close() must release it.
                fd._is_external = false;

                return fd;
            }

            /// @brief Wraps the process's standard input descriptor (fd 0).
            /// @return A non-owning, read-mode `file_handler` over fd 0.
            static inline file_handler get_stdin()
            {
                file_handler fd;
                fd._fd = 0;
                fd._mode = io_options::read;
                fd._is_external = false;

                return fd;
            }

            /// @brief Wraps the process's standard output descriptor (fd 1).
            /// @return A non-owning, write-mode `file_handler` over fd 1.
            static inline file_handler get_stdout()
            {
                file_handler fd;
                fd._fd = 1;
                fd._mode = io_options::write;
                fd._is_external = true;

                return fd;
            }

            /// @brief Wraps the process's standard error descriptor (fd 2).
            /// @return A non-owning, write-mode `file_handler` over fd 2.
            static inline file_handler get_stderr()
            {
                file_handler fd;
                fd._fd = 2;
                fd._mode = io_options::write;
                fd._is_external = true;

                return fd;
            }
        };  
    } // namespace io_detail
    #endif
} // namespace hsd