#pragma once

/// @file _FSDetail.hpp
/// @brief Internal filesystem path-resolution implementation.
/// @details Stores relative and absolute locations and hides platform-specific
/// normalization from the public filesystem::path interface.

#include "../Containers/String.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(HSD_PLATFORM_WINDOWS)
/// @brief Platform directory separator character.
#define PATH_SEPARATOR '\\'
/// @brief Platform directory separator string.
#define PATH_SEPARATOR_STR "\\"
/// @brief Root path prefix for the Windows filesystem implementation.
#define ROOT_PATH "C:\\"
/// @brief Character length of the Windows root path prefix.
#define ROOT_PATH_LEN 3u
/// @brief Character length of the separator string.
#define PATH_SEPARATOR_STR_LEN 1u
#else
/// @brief Platform directory separator character.
#define PATH_SEPARATOR '/'
/// @brief Platform directory separator string.
#define PATH_SEPARATOR_STR "/"
/// @brief Root path prefix for the POSIX filesystem implementation.
#define ROOT_PATH "/"
/// @brief Character length of the POSIX root path prefix.
#define ROOT_PATH_LEN 1u
/// @brief Character length of the separator string.
#define PATH_SEPARATOR_STR_LEN 1u
#endif

///
/// @brief Used in identifying the classes
/// and functions in the standard library. 
///
namespace hsd
{
    ///
    /// @brief Namespace for file system related functions.
    ///
    namespace filesystem
    {
        ///
        /// @brief Namespace for file system implementation details.
        ///
        namespace fs_detail
        {
            ///
            /// @brief Structure for holding path information.
            ///
            class location
            {
            public:
                ///
                /// @brief Holds the relative path of a file or directory.
                ///
                string _relative;

                ///
                /// @brief Holds the absolute path of a file or directory.
                ///
                string _absolute;

            private:
                /// @brief Finds the end of the path component starting at `text`.
                static inline const char* _component_end(const char* text)
                {
                    while (*text != '\0' && *text != PATH_SEPARATOR)
                    {
                        ++text;
                    }
                    return text;
                }

                /// @brief Skips any run of separators starting at `text`.
                static inline const char* _skip_separators(const char* text)
                {
                    while (*text == PATH_SEPARATOR)
                    {
                        ++text;
                    }
                    return text;
                }

                /// @brief Tests whether two character runs of length `count` are equal.
                static inline bool _same_text(const char* lhs, const char* rhs, usize count)
                {
                    for (usize index = 0; index < count; ++index)
                    {
                        if (lhs[index] != rhs[index])
                        {
                            return false;
                        }
                    }
                    return true;
                }

                ///
                /// @brief Resolves `loc` into both an absolute path (via
                /// `GetFullPathNameA` on Windows or `realpath` on POSIX) and
                /// a `..`-prefixed relative path computed against the
                /// current working directory, storing both in `_absolute`
                /// and `_relative`. Uses a fixed 2048-byte stack buffer for
                /// the platform path calls, so paths longer than that are
                /// silently truncated (pre-existing limitation, not fixed
                /// here).
                /// 
                /// @param loc - The path of a file or directory to resolve.
                ///
                inline void _resolve_location(const char* loc)
                {
                    char _buffer[2048]{};

                    #if defined(HSD_PLATFORM_WINDOWS)
                    GetFullPathNameA(loc, 2048, _buffer, nullptr);
                    #else
                    realpath(loc, _buffer);
                    #endif

                    _absolute = string{_buffer};

                    char _cwd[2048]{};

                    #if defined(HSD_PLATFORM_WINDOWS)
                    GetCurrentDirectory(2048, _cwd);
                    #else
                    getcwd(_cwd, 2048);
                    #endif

                    // Drop the leading components the absolute path shares with
                    // the working directory; each shared component is compared whole.
                    const char* _abs_ptr = _skip_separators(_absolute.c_str());
                    const char* _cur_ptr = _skip_separators(_cwd);

                    while (true)
                    {
                        const char* _abs_end = _component_end(_abs_ptr);
                        const char* _cur_end = _component_end(_cur_ptr);
                        const usize _abs_len = static_cast<usize>(_abs_end - _abs_ptr);

                        if (_abs_len == 0 || _abs_len != static_cast<usize>(_cur_end - _cur_ptr) ||
                            !_same_text(_abs_ptr, _cur_ptr, _abs_len))
                        {
                            break;
                        }

                        _abs_ptr = _skip_separators(_abs_end);
                        _cur_ptr = _skip_separators(_cur_end);
                    }

                    #if defined(HSD_PLATFORM_WINDOWS)
                    // Another drive has no relative form, since ".." cannot climb to it.
                    // The working-directory cursor only moves past a shared component.
                    if (_cur_ptr == _skip_separators(_cwd))
                    {
                        _relative = _absolute;
                        return;
                    }
                    #endif

                    // One ".." for each working-directory component that is not shared.
                    _relative = "." PATH_SEPARATOR_STR;

                    for (const char* _rest = _cur_ptr; *_rest != '\0';)
                    {
                        const char* _rest_end = _component_end(_rest);

                        if (_rest_end != _rest)
                        {
                            _relative += ".." PATH_SEPARATOR_STR;
                        }

                        _rest = _skip_separators(_rest_end);
                    }

                    _relative += _abs_ptr;
                }

            public:
                ///
                /// @brief Constructs a new empty location object
                ///
                inline location() = default;

                ///
                /// @brief Constructs a new location object.
                /// 
                /// @param loc - The path of a file or directory.
                ///
                inline location(const char* loc)
                {
                    _resolve_location(loc);
                }

                ///
                /// @brief Constructs a new location object.
                /// 
                /// @param loc - The path of a file or directory.
                ///
                inline location(const string& loc)
                {
                    _resolve_location(loc.c_str());
                }

                ///
                /// @brief Constructs by copy a new location object.
                /// 
                /// @param other - The location object to copy.
                ///
                inline location(const location& other)
                    : _relative{other._relative},
                      _absolute{other._absolute}
                {}

                ///
                /// @brief Constructs by move a new location object.
                /// 
                /// @param other - The location object to move.
                ///
                inline location(location&& other)
                    : _relative{hsd::move(other._relative)},
                      _absolute{hsd::move(other._absolute)}
                {}

                ///
                /// @brief Assigns by copy a new location object.
                /// 
                /// @param rhs - The location object to copy.
                /// @return location& - A reference to the assigned location object.
                ///
                inline location& operator=(const location& rhs)
                {
                    _relative = rhs._relative;
                    _absolute = rhs._absolute;
                    return *this;
                }

                ///
                /// @brief Assigns by move a new location object.
                /// 
                /// @param rhs - The location object to move.
                /// @return location& - A reference to the assigned location object.
                ///
                inline location& operator=(location&& rhs)
                {
                    _relative = hsd::move(rhs._relative);
                    _absolute = hsd::move(rhs._absolute);
                    return *this;
                }
            };
        } // namespace fs_detail
    } // namespace filesystem
} // namespace hsd