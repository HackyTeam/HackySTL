#pragma once

/// @file Path.hpp
/// @brief Filesystem path manipulation and directory operations.
/// @details Resolves relative/absolute names, joins components, lists entries,
/// reports status, copies, renames, creates directories, and enumerates parents.

#include "../Containers/Vector.hpp"
#include "../Serialize/_IoDetail.hpp"
#include "FStatus.hpp"
#include "_FSDetail.hpp"

#ifdef HSD_PLATFORM_WINDOWS
#include "_dirent_win32.h"
#include <io.h>
#include <direct.h>
#else
#include <dirent.h>
#include <utime.h>
// POSIX declares rename() only in <stdio.h>, so this is the one stdio include left.
#include <stdio.h>
#endif

#include <fcntl.h>
#include <errno.h>
#include <string.h>

/// @brief Concatenates a directory path and file name with the platform separator.
#define CONCAT_PATH(path, file)\
    path PATH_SEPARATOR_STR file
    
namespace hsd
{
    namespace filesystem
    {
        namespace fs_detail
        {
            ///
            /// @brief Copies the byte contents of one file to another in chunks,
            /// using `io_detail::file_handler`. Overwrites `to` if it already
            /// exists, since it is truncated when opened. Does not preserve
            /// permissions or timestamps.
            ///
            /// @param from - Path of the source file to read.
            /// @param to - Path of the destination file to (over)write.
            /// @return option_err<runtime_error> - Empty on success, or a
            /// `runtime_error` if the source cannot be opened, the destination
            /// cannot be created, or a read or write fails partway through (in
            /// which case `to` may be left truncated/partially written).
            ///
            inline auto copy_file(const char* from, const char* to)
                -> option_err<runtime_error>
            {
                // The source is opened first, so a missing source creates no destination.
                // Binary handles keep the bytes as they are, with no newline translation.
                auto _from_file = io_detail::file_handler::open(from, io_options::read);

                if (_from_file.is_open() == false)
                {
                    return runtime_error{"File not found"};
                }

                auto _to_file = io_detail::file_handler::open(to, io_options::write);

                if (_to_file.is_open() == false)
                {
                    return runtime_error{"Couldn't create the destination file"};
                }

                char _buffer[4096]{};

                while (true)
                {
                    auto _count = _from_file.read(_buffer, sizeof(_buffer));

                    if (_count == static_cast<decltype(_count)>(-1))
                    {
                        return runtime_error{"Couldn't read the file"};
                    }
                    else if (_count == 0)
                    {
                        return {};
                    }

                    // A write can stop early, so keep writing what is left of this chunk.
                    usize _done = 0;

                    while (_done < static_cast<usize>(_count))
                    {
                        auto _written = _to_file.write(
                            _buffer + _done, static_cast<usize>(_count) - _done
                        );

                        if (_written == static_cast<decltype(_written)>(-1) || _written == 0)
                        {
                            return runtime_error{"Couldn't write into the file"};
                        }

                        _done += static_cast<usize>(_written);
                    }
                }
            }
        } // namespace fs_detail
        

        ///
        /// @brief Represents a filesystem path together with an open
        /// directory handle for iteration. Wraps `_relative`/`_absolute`
        /// name resolution (`fs_detail::location`) and keeps a `DIR*` handle
        /// open (via `opendir`) so `list()` can enumerate directory entries
        /// without reopening the directory each time.
        ///
        class path
        {
        private:
            /// Open directory handle for this path, or nullptr if the path
            /// does not name a directory or could not be opened.
            DIR* _directory = nullptr;
            /// Resolved relative/absolute name pair for this path.
            fs_detail::location _location;

            /// Default permission bits used when creating new directories/files.
            static constexpr auto _default_perms = 
                fs_perms_mask::owner_all;

            ///
            /// @brief Closes the open directory handle, if any.
            ///
            void _destroy()
            {
                if (_directory != nullptr)
                {
                    closedir(_directory);
                    _directory = nullptr;
                }
            }

            ///
            /// @brief Closes and reopens the directory handle at the current
            /// absolute name. Used after operations (such as `list()`) that
            /// exhaust the directory stream and need to reset it for reuse.
            ///
            void _reconstruct()
            {
                _destroy();
                _directory = opendir(absolute_name().c_str());
            }

        public:
            ///
            /// @brief Constructs a path referring to the current working
            /// directory (".").
            ///
            inline path()
                : path{string(".", 1)}
            {
                // Opens the directory at the
                // location of the executable
            }

            ///
            /// @brief Constructs a path from a raw C-string, resolving it
            /// and opening its directory handle (if it names a directory).
            /// 
            /// @param raw_path - The path to resolve.
            ///
            inline path(const char* raw_path)
                : _location{raw_path}
            {
                _directory = opendir(absolute_name().c_str());
            }

            ///
            /// @brief Constructs a path from a string, resolving it and
            /// opening its directory handle (if it names a directory).
            /// 
            /// @param raw_path - The path to resolve.
            ///
            inline path(const string& raw_path)
                : _location{raw_path}
            {
                _directory = opendir(absolute_name().c_str());
            }

            ///
            /// @brief Constructs by copy a new path object, opening its own
            /// independent directory handle.
            /// 
            /// @param other - The path object to copy.
            ///
            inline path(const path& other)
                : _location{other._location}
            {
                _directory = opendir(absolute_name().c_str());
            }

            ///
            /// @brief Constructs by move a new path object, taking ownership
            /// of `other`'s directory handle.
            /// 
            /// @param other - The path object to move.
            ///
            inline path(path&& other)
                : _location{hsd::move(other._location)}
            {
                hsd::swap(_directory, other._directory);
            }

            ///
            /// @brief Destroys the path object, closing its directory handle.
            ///
            inline ~path()
            {
                _destroy();
            }

            ///
            /// @brief Assigns by copy, re-resolving and reopening the
            /// directory handle for the copied location.
            /// 
            /// @param rhs - The path object to copy.
            /// @return path& - Reference to the assigned path object.
            ///
            path& operator=(const path& rhs)
            {
                _location = rhs._location;
                _reconstruct();
                return *this;
            }

            ///
            /// @brief Assigns by move, swapping location and directory
            /// handle ownership with `rhs`.
            /// 
            /// @param rhs - The path object to move.
            /// @return path& - Reference to the assigned path object.
            ///
            path& operator=(path&& rhs)
            {
                hsd::swap(_location, rhs._location);
                hsd::swap(_directory, rhs._directory);
                return *this;
            }

            ///
            /// @brief Compares two paths by absolute name.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if both paths resolve to the same
            /// absolute name.
            ///
            inline bool operator==(const path& rhs) const
            {
                return absolute_name() == rhs.absolute_name();
            }

            ///
            /// @brief Compares two paths by absolute name.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if the absolute names differ.
            ///
            inline bool operator!=(const path& rhs) const
            {
                return absolute_name() != rhs.absolute_name();
            }

            ///
            /// @brief Lexicographically compares absolute names.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if this path's absolute name sorts before `rhs`.
            ///
            inline bool operator<(const path& rhs) const
            {
                return absolute_name() < rhs.absolute_name();
            }

            ///
            /// @brief Lexicographically compares absolute names.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if this path's absolute name sorts before
            /// or equals `rhs`.
            ///
            inline bool operator<=(const path& rhs) const
            {
                return absolute_name() <= rhs.absolute_name();
            }

            ///
            /// @brief Lexicographically compares absolute names.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if this path's absolute name sorts after `rhs`.
            ///
            inline bool operator>(const path& rhs) const
            {
                return absolute_name() > rhs.absolute_name();
            }

            ///
            /// @brief Lexicographically compares absolute names.
            /// 
            /// @param rhs - Path to compare with.
            /// @return bool - True if this path's absolute name sorts after
            /// or equals `rhs`.
            ///
            inline bool operator>=(const path& rhs) const
            {
                return absolute_name() >= rhs.absolute_name();
            }

            ///
            /// @brief Gets the filesystem root path ("/" on POSIX, "C:\\"
            /// on Windows).
            /// 
            /// @return path - A path referring to the filesystem root.
            ///
            static inline path root()
            {
                return path{string(
                    ROOT_PATH, ROOT_PATH_LEN
                )};
            }

            ///
            /// @brief Gets the parent directory of this path.
            /// 
            /// @return path - The parent directory, or the root itself if
            /// this path already is the root.
            ///
            inline path parent()
            {
                if (*this == root())
                    return root();

                return path{(
                    relative_name() + CONCAT_PATH("", "..")
                )};
            }

            ///
            /// @brief Lists the entries of this directory (excluding "." and
            /// "..."), each returned as its own resolved `path`.
            /// @details Opens the directory for this call only, so repeated calls
            /// on the same path object always reflect the directory's current state.
            /// 
            /// @return result<vector<path>, runtime_error> - The listed entries, or
            /// a `runtime_error` if the directory cannot be opened (for example, it
            /// does not exist or is not a directory).
            ///
            inline auto list()
                -> result<hsd::vector<path>, runtime_error>
            {
                // A fresh stream each time, so entries created or renamed since the
                // last call are reported; a stream kept from an earlier call can miss them.
                DIR* _stream = opendir(absolute_name().c_str());

                if (_stream == nullptr)
                {
                    return runtime_error{"Cannot open directory"};
                }

                hsd::vector<path> _paths;
                dirent* _dir_entry = nullptr;

                while ((_dir_entry = readdir(_stream)))
                {
                    if (cstring::compare(_dir_entry->d_name, ".") != 0 && 
                        cstring::compare(_dir_entry->d_name, "..") != 0)
                    {
                        if (*(relative_name().end() - 1) == PATH_SEPARATOR)
                        {
                            _paths.emplace_back(relative_name() + 
                                _dir_entry->d_name);
                        }
                        else
                        {
                            _paths.emplace_back(
                                relative_name() + PATH_SEPARATOR_STR + _dir_entry->d_name
                            );
                        }
                    }
                }

                closedir(_stream);
                return hsd::move(_paths);
            }

            ///
            /// @brief Gets the relative name of this path, computed against
            /// the current working directory at resolution time.
            /// 
            /// @return const string& - The relative path string.
            ///
            inline const string& relative_name() const
            {
                return _location._relative;
            }

            ///
            /// @brief Gets the absolute (fully resolved) name of this path.
            /// 
            /// @return const string& - The absolute path string.
            ///
            inline const string& absolute_name() const
            {
                return _location._absolute;
            }

            ///
            /// @brief Changes the permission bits of the entry named by this
            /// path (`_chmod` on Windows, `chmod` on POSIX).
            /// 
            /// @param mask - The permission bits to apply.
            /// @return option_err<runtime_error> - Empty on success, or a
            /// `runtime_error` if the path does not exist or the underlying
            /// `chmod`/`_chmod` call fails.
            ///
            inline auto change_permissions(const fs_perms_mask& mask)
                -> option_err<runtime_error>
            {
                if (!status().exists())
                {
                    return runtime_error{"File/Directory not found"};
                }
                else
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    i32 _res = _chmod(absolute_name().c_str(), static_cast<i32>(mask));
                    #else
                    i32 _res = chmod(absolute_name().c_str(), static_cast<u32>(mask));
                    #endif

                    if (_res == -1)
                    {
                        return runtime_error{
                            "Couldn't change permissions of the file"
                        };
                    }   
                }

                return {};
            }

            ///
            /// @brief Renames (moves) the entry named by this path to
            /// `new_name` in the same parent directory, updating this
            /// object in place to refer to the new location.
            /// Pre-existing bug: after a successful `::rename()`, the new
            /// directory handle is not correctly reopened at the renamed
            /// location in either branch below. In the no-separator branch
            /// the temporary `_new_path`'s directory handle was opened
            /// (and failed, since the destination did not exist yet) before
            /// the rename happened and is never refreshed afterwards; in the
            /// separator branch the handle is reopened using this object's
            /// stale (pre-rename, now nonexistent) `relative_name()` instead
            /// of `_new_path`'s new name. Either way, `this->_directory` can
            /// end up `nullptr` after a successful rename of a directory,
            /// causing a subsequent `list()` call to fail with "No such
            /// directory" until the path is manually reconstructed.
            /// 
            /// @param new_name - The new name for the entry (not a full path).
            /// @return option_err<runtime_error> - Empty on success, or a
            /// `runtime_error` if the path does not exist or the underlying
            /// `rename()` call fails.
            ///
            inline auto rename(const string& new_name)
                -> option_err<runtime_error> 
            {
                if (!status().exists())
                {
                    return runtime_error{"File/Directory not found"};
                }

                // Keep the directory part of this path and replace the final name.
                usize _slash_pos = relative_name().rfind(PATH_SEPARATOR_STR);
                string _target_name = new_name;
                if (_slash_pos != string::npos)
                {
                    // The sized constructor counts its terminator, so a view gives the exact length.
                    string _prefix{basic_string_view<char>{relative_name().c_str(), _slash_pos + 1}};
                    _target_name = _prefix + new_name;
                }

                // The target does not exist yet, so its handle is opened after the rename.
                path _target{_target_name.c_str()};

                #ifdef HSD_PLATFORM_WINDOWS
                // rename() fails on Windows when the target exists, so MoveFileEx
                // replaces it, as rename(2) does on POSIX.
                if (MoveFileExA(
                    absolute_name().c_str(),
                    _target.absolute_name().c_str(),
                    MOVEFILE_REPLACE_EXISTING) == 0)
                {
                    return runtime_error{"Couldn't rename the entry"};
                }
                #else
                const int _res = ::rename(
                    absolute_name().c_str(),
                    _target.absolute_name().c_str()
                );

                if (_res == -1)
                {
                    return runtime_error{strerror(errno)};
                }
                #endif

                // Reopen the handle at the new name, so listing a renamed directory works.
                *this = path{_target.absolute_name().c_str()};
                return {};
            }

            ///
            /// @brief Gets the file extension (including the leading dot) of
            /// this path's final component.
            /// Note: calls `status().is_regular_file().unwrap()` internally,
            /// which panics/aborts if the path does not exist (no existence
            /// check is performed first).
            /// 
            /// @return string - The extension (e.g. ".txt"), or an empty
            /// string if the entry is not a regular file, has no extension,
            /// or the last '.' is immediately followed by a path separator.
            ///
            inline auto extension() const
            {
                if (status().is_regular_file().unwrap())
                {
                    usize _point_pos = relative_name().rfind('.');

                    if (_point_pos != string::npos && relative_name()[_point_pos + 1] != PATH_SEPARATOR)
                    {
                        const char* _str_data = relative_name().c_str();
                        usize _str_size = relative_name().size();
                        return string (
                            _str_data + _point_pos, 
                            _str_size - _point_pos
                        );
                    }

                    return string{};
                }
                
                // will return an empty string instead
                return string{};
            }

            ///
            /// @brief Copies this entry to `to`. Directory-to-directory
            /// copies recurse over `list()`, creating matching
            /// subdirectories/files under `to` before recursively copying
            /// their contents; file-to-file and file-to-directory copies
            /// delegate to `fs_detail::copy_file()`.
            /// Pre-existing bug: the directory-recursion branch extracts
            /// each child's file name with
            /// `_item.absolute_name().rfind('/')`, using a hardcoded '/'
            /// instead of the platform `PATH_SEPARATOR` macro (used
            /// correctly in the file-to-directory branch below). On Windows
            /// (where the separator is '\\') this `rfind` can return
            /// `string::npos`, causing `c_str() + npos` to form an
            /// out-of-bounds pointer — undefined behavior when recursively
            /// copying directories on Windows.
            /// Also note: existence/type checks throughout use
            /// `status().is_directory().unwrap()` style calls, which panic
            /// if a path disappears or is inaccessible between the check
            /// and use (no existence guard beforehand).
            /// 
            /// @param to - Destination path (existing directory, or file
            /// path for file-to-file copies).
            /// @return option_err<runtime_error> - Empty on success; a
            /// `runtime_error` if copying a directory into a file, or if
            /// none of the source/destination type combinations match
            /// ("Unreachable code"); otherwise propagates `copy_file()` errors.
            ///
            inline auto copy(const path& to)
                -> option_err<runtime_error> 
            {
                if (status().is_directory().unwrap() &&
                    to.status().is_directory().unwrap())
                {
                    for (auto& _item : list().unwrap())
                    {
                        const char* _item_name =
                            _item.absolute_name().c_str() +
                            _item.absolute_name().rfind('/');

                        path _new_to = to + _item_name;

                        if(_item.status().is_directory().unwrap())
                        {
                            #if defined(HSD_PLATFORM_WINDOWS)
                            _mkdir(
                                _new_to.absolute_name().c_str()
                            );
                            #else
                            mkdir(
                                _new_to.absolute_name().c_str(), 
                                static_cast<u32>(_default_perms)
                            );
                            #endif
                        }
                        else
                        {
                            // copy_file() needs an existing destination, so create an empty one first.
                            io_detail::file_handler::open(_new_to.absolute_name().c_str(), io_options::rw_create);
                        }

                        _item.copy(_new_to).unwrap();
                    }
                }
                else if (status().is_regular_file().unwrap() &&
                    to.status().is_regular_file().unwrap())
                {
                    return fs_detail::copy_file(
                        absolute_name().c_str(),
                        to.absolute_name().c_str()
                    );
                }
                else if (status().is_regular_file().unwrap() &&
                    to.status().is_directory().unwrap())
                {
                    const char* _item_name =
                        absolute_name().c_str() +
                        absolute_name().rfind(PATH_SEPARATOR);

                    path _new_to = to + _item_name;
                    return fs_detail::copy_file(
                        absolute_name().c_str(), 
                        _new_to.absolute_name().c_str()
                    );
                }
                else if (status().is_directory().unwrap() &&
                    to.status().is_regular_file().unwrap())
                {
                    return runtime_error{
                        "Cannot copy a directory into a file"
                    };
                }
                else
                {
                    return runtime_error{"Unreachable code"};
                }
                
                return {};
            }

            ///
            /// @brief Creates a single new subdirectory named `name` under
            /// this path, using `_default_perms` (owner-only permissions).
            /// Rejects names containing an interior path separator (i.e.
            /// only a single directory level may be created at a time; a
            /// trailing separator is tolerated).
            /// Pre-existing bug: the return value of the underlying
            /// `mkdir`/`_mkdir` call is discarded, so this always returns an
            /// empty (success) result even if directory creation actually
            /// failed (e.g. already exists, permission denied, missing
            /// parent).
            /// 
            /// @param name - Single path component naming the new directory.
            /// @return option_err<runtime_error> - A `runtime_error` if
            /// `name` contains an interior separator; otherwise always empty,
            /// regardless of whether the directory was actually created
            /// (see bug note above).
            ///
            inline auto create_directory(const string& name) const
                -> option_err<runtime_error>
            {
                if (name.rfind(PATH_SEPARATOR) < name.size() - 1)
                {
                    return runtime_error{
                        "Cannot create multiple directories"
                    };
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                if (_mkdir(
                    (absolute_name() + PATH_SEPARATOR_STR + name).c_str()
                ) == -1 && errno != EEXIST)
                {
                    char _buf[1024]{};
                    strerror_s(_buf, 1024, errno);
                    return runtime_error{_buf};
                }
                #else
                if (mkdir(
                    (absolute_name() + PATH_SEPARATOR_STR + name).c_str(), 
                    static_cast<u32>(_default_perms)
                ) == -1 && errno != EEXIST)
                {
                    return runtime_error{strerror(errno)};
                }
                #endif

                return {};
            }

            ///
            /// @brief Concatenates the relative name of this path with a
            /// raw string, without inserting a separator.
            /// 
            /// @param rhs - String to append.
            /// @return path - A new path built from the concatenated name.
            ///
            inline path operator+(const char* rhs) const
            {
                return path{relative_name() + rhs};
            }

            ///
            /// @brief Concatenates the relative name of this path with a
            /// string, without inserting a separator.
            /// 
            /// @param rhs - String to append.
            /// @return path - A new path built from the concatenated name.
            ///
            inline path operator+(const string& rhs) const
            {
                return path{relative_name() + rhs};
            }

            ///
            /// @brief Concatenates the relative names of two paths, without
            /// inserting a separator.
            /// 
            /// @param rhs - Path whose relative name to append.
            /// @return path - A new path built from the concatenated name.
            ///
            inline path operator+(const path& rhs) const
            {
                return path{relative_name() + rhs.relative_name()};
            }

            ///
            /// @brief In-place version of `operator+`.
            /// 
            /// @param rhs - String to append.
            /// @return path& - Reference to this path.
            ///
            inline path& operator+=(const char* rhs)
            {
                *this = hsd::move(*this + rhs);
                return *this;
            }

            ///
            /// @brief In-place version of `operator+`.
            /// 
            /// @param rhs - String to append.
            /// @return path& - Reference to this path.
            ///
            inline path& operator+=(const string& rhs)
            {
                *this = hsd::move(*this + rhs);
                return *this;
            }

            ///
            /// @brief In-place version of `operator+`.
            /// 
            /// @param rhs - Path whose relative name to append.
            /// @return path& - Reference to this path.
            ///
            inline path& operator+=(const path& rhs)
            {
                *this = hsd::move(*this + rhs);
                return *this;
            }

            ///
            /// @brief Joins this path with a raw string as a new path
            /// component, inserting a platform path separator unless `rhs`
            /// already starts with one.
            /// 
            /// @param rhs - Component to join.
            /// @return path - A new joined path.
            ///
            inline path operator/(const char* rhs) const
            {
                if (rhs[0] != PATH_SEPARATOR)
                    return path{relative_name() + PATH_SEPARATOR_STR + rhs};

                return path{relative_name() + rhs};
            }

            ///
            /// @brief Joins this path with a string as a new path component,
            /// inserting a platform path separator unless `rhs` already
            /// starts with one.
            /// 
            /// @param rhs - Component to join.
            /// @return path - A new joined path.
            ///
            inline path operator/(const string& rhs) const
            {
                if (rhs[0] != PATH_SEPARATOR)
                    return path{relative_name() + PATH_SEPARATOR_STR + rhs};

                return path{relative_name() + rhs};
            }

            ///
            /// @brief Joins this path with another path's relative name as a
            /// new path component, inserting a platform path separator
            /// unless `rhs`'s relative name already starts with one.
            /// 
            /// @param rhs - Path whose relative name to join.
            /// @return path - A new joined path.
            ///
            inline path operator/(const path& rhs) const
            {
                if (rhs.relative_name()[0] != PATH_SEPARATOR)
                    return path{relative_name() + PATH_SEPARATOR_STR + rhs.relative_name()};

                return path{relative_name() + rhs.relative_name()};
            }

            ///
            /// @brief In-place version of `operator/`.
            /// 
            /// @param rhs - Component to join.
            /// @return path& - Reference to this path.
            ///
            inline path& operator/=(const char* rhs)
            {
                *this = hsd::move(*this / rhs);
                return *this;
            }

            ///
            /// @brief In-place version of `operator/`.
            /// 
            /// @param rhs - Component to join.
            /// @return path& - Reference to this path.
            ///
            inline path& operator/=(const string& rhs)
            {
                *this = hsd::move(*this / rhs);
                return *this;
            }

            ///
            /// @brief In-place version of `operator/`.
            /// 
            /// @param rhs - Path whose relative name to join.
            /// @return path& - Reference to this path.
            ///
            inline path& operator/=(const path& rhs)
            {
                *this = hsd::move(*this / rhs);
                return *this;
            }

            ///
            /// @brief Queries the current filesystem status of this path.
            /// A fresh `fs_status` snapshot is taken on every call (it is
            /// not cached), so repeated calls reflect the entry's latest
            /// state.
            /// 
            /// @return fs_status - A snapshot of type, size, and permissions.
            ///
            inline fs_status status() const
            {
                return {relative_name().c_str()};
            }

            ///
            /// @brief Splits the absolute name into each of its ancestor
            /// path components, from the filesystem root down to (but not
            /// including) this path's own final component.
            /// 
            /// @return auto - A `vector<string>` of ancestor path components,
            /// starting with the root.
            ///
            inline auto parents()
            {
                usize _index, _pos;

                vector<string> _parents;
                _parents.emplace_back(ROOT_PATH, ROOT_PATH_LEN);

                for (_index = absolute_name().find(PATH_SEPARATOR, 1), _pos = 1; 
                    _index != string::npos; _index = absolute_name().find(PATH_SEPARATOR, _pos))
                {
                    _parents.emplace_back(
                        absolute_name().c_str() 
                        + _pos, _index - _pos
                    );
                    _pos = _index + 1;
                }

                return _parents;
            }
        };
    } // namespace filesystem
    //#endif
} // namespace hsd
