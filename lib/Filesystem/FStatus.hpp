#pragma once

/// @file FStatus.hpp
/// @brief Filesystem status, type, size, and permission queries.
/// @details Wraps platform file metadata in checked accessors used by path
/// operations and filesystem tests.

#include "../Base/Result.hpp"

#ifdef HSD_PLATFORM_POSIX
#include <unistd.h>
#endif

#include <sys/stat.h>

namespace hsd
{
    namespace filesystem
    {
        #ifdef HSD_PLATFORM_WINDOWS
        #define _S_IEVER (_S_IREAD | _S_IWRITE | _S_IEXEC)
        #else
        /// @brief Combined owner/group/other read-write-execute permission mask.
        #define S_IRWXA (S_IRWXO | S_IRWXU | S_IRWXG)
        #endif

        ///
        /// @brief Bit mask of POSIX/Windows file permission flags, combinable
        /// with `operator|`. Windows only exposes owner-level bits since the
        /// Win32 `_S_I*` constants have no group/other equivalents.
        ///
        enum class fs_perms_mask
        {
            #ifdef HSD_PLATFORM_WINDOWS
            owner_read   = _S_IREAD,
            owner_write  = _S_IWRITE,
            owner_exec   = _S_IEXEC,
            owner_all    = _S_IEVER
            #else
            owner_read   = S_IRUSR,
            owner_write  = S_IWUSR,
            owner_exec   = S_IXUSR,
            owner_all    = S_IRWXU,
            group_read   = S_IRGRP,
            group_write  = S_IWGRP,
            group_exec   = S_IXGRP,    
            group_all    = S_IRWXG,  
            others_read  = S_IROTH,
            others_write = S_IWOTH,
            others_exec  = S_IXOTH,
            others_all   = S_IRWXO,
            all_perms    = S_IRWXA
            #endif
        };

        ///
        /// @brief Combines two permission masks into one.
        /// 
        /// @param lhs - First mask.
        /// @param rhs - Second mask.
        /// @return fs_perms_mask - Bitwise OR of both masks.
        ///
        constexpr fs_perms_mask operator|(
            const fs_perms_mask& lhs, const fs_perms_mask& rhs)
        {
            return static_cast<fs_perms_mask>(
                static_cast<i32>(lhs) | static_cast<i32>(rhs)
            );
        }

        /// @brief Permission bits reported for a filesystem entry.
        struct fs_permissions
        {
            bool can_owner_read;            ///< File owner has read permission
            bool can_owner_write;           ///< File owner has write permission
            bool can_owner_exec;            ///< File owner has execute/search permission
            bool can_owner_do_everything;	///< File owner has read, write, and execute/search permissions

            #ifndef HSD_PLATFORM_WINDOWS
            bool can_group_read;	        ///< The file's user group has read permission
            bool can_group_write;           ///< The file's user group has write permission
            bool can_group_exec;	        ///< The file's user group has execute/search permission
            bool can_group_do_everything;   ///< The file's user group has read, write, and execute/search permissions

            bool can_others_read;           ///< Other users have read permission
            bool can_others_write;          ///< Other users have write permission
            bool can_others_exec;           ///< Other users have execute/search permission
            bool can_others_do_everything;  ///< Other users have read, write, and execute/search permissions

            bool can_all_do_everything;     ///< Everyone can read write and execute the file
            #endif
        };

        /// @brief Snapshot of filesystem entry type, size, and permissions.
        class fs_status
        {
        private:
            /// Raw platform stat buffer (`::stat` on POSIX and Windows).
            struct stat _status;
            /// Whether the queried path existed when this snapshot was taken.
            bool _exists = false;

        public:
            ///
            /// @brief Constructs a snapshot of the given path's metadata.
            /// On POSIX this uses `lstat` (does not follow symlinks, so a
            /// symlink itself is reported rather than its target); on
            /// Windows it uses `stat`. If the path does not exist, the
            /// object is still constructed but `exists()` returns false and
            /// every query method returns a `runtime_error`.
            /// 
            /// @param pathname - The path to query.
            ///
            inline fs_status(const char* pathname)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                _exists = stat(pathname, &_status) != -1;
                #else
                _exists = lstat(pathname, &_status) != -1;
                #endif
            }

            ///
            /// @brief Compares the raw stat buffers of two snapshots field
            /// by field (timestamps, size, device/inode identity, etc.).
            /// Does not compare `exists()`.
            /// 
            /// @param rhs - Snapshot to compare with.
            /// @return bool - True if every compared stat field is equal.
            ///
            inline bool operator==(const fs_status& rhs) const
            {
                return (
                    #if defined(HSD_PLATFORM_POSIX)
                    _status.st_atim.tv_nsec == rhs._status.st_atim.tv_nsec &&
                    _status.st_ctim.tv_nsec == rhs._status.st_ctim.tv_nsec &&
                    _status.st_mtim.tv_nsec == rhs._status.st_mtim.tv_nsec &&
                    _status.st_atim.tv_sec  == rhs._status.st_atim.tv_sec  &&
                    _status.st_ctim.tv_sec  == rhs._status.st_ctim.tv_sec  &&
                    _status.st_mtim.tv_sec  == rhs._status.st_mtim.tv_sec  &&
                    _status.st_blksize      == rhs._status.st_blksize      &&
                    _status.st_blocks       == rhs._status.st_blocks       &&
                    #elif defined(HSD_PLATFORM_WINDOWS)
                    _status.st_atime        == rhs._status.st_atime        &&
                    _status.st_ctime        == rhs._status.st_ctime        &&
                    _status.st_mtime        == rhs._status.st_mtime        &&
                    _status.st_atime        == rhs._status.st_atime        &&
                    _status.st_ctime        == rhs._status.st_ctime        &&
                    _status.st_mtime        == rhs._status.st_mtime        &&
                    #endif
                    _status.st_nlink        == rhs._status.st_nlink        &&
                    _status.st_mode         == rhs._status.st_mode         &&
                    _status.st_rdev         == rhs._status.st_rdev         &&
                    _status.st_size         == rhs._status.st_size         &&
                    _status.st_dev          == rhs._status.st_dev          &&
                    _status.st_gid          == rhs._status.st_gid          &&
                    _status.st_ino          == rhs._status.st_ino          &&
                    _status.st_uid          == rhs._status.st_uid
                );
            }

            ///
            /// @brief Checks whether the queried path existed at construction.
            /// 
            /// @return bool - True if the path existed.
            ///
            inline bool exists() const
            {
                return _exists;
            }

            ///
            /// @brief Checks whether the entry is a directory.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_directory() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return (_status.st_mode & _S_IFDIR) == _S_IFDIR;
                    #else
                    return S_ISDIR(_status.st_mode);
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a character device.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_character() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return (_status.st_mode & _S_IFCHR) == _S_IFCHR;
                    #else
                    return S_ISCHR(_status.st_mode);
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a block device. Always
            /// false on Windows, which has no block-device stat bit.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_block_file() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return false;
                    #else
                    return S_ISBLK(_status.st_mode);
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a regular file.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_regular_file() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return (_status.st_mode & _S_IFREG) == _S_IFREG;
                    #else
                    return S_ISREG(_status.st_mode);
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a named pipe (FIFO).
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_fifo_file() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return (_status.st_mode & _S_IFIFO) == _S_IFIFO;
                    #else
                    return S_ISFIFO(_status.st_mode);
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a symbolic link. Always
            /// false on Windows. Note: since the snapshot is taken with
            /// `lstat` on POSIX, this reflects the link itself, not its
            /// target.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_symlink() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #if defined(HSD_PLATFORM_POSIX)
                    return S_ISLNK(_status.st_mode);
                    #elif defined(HSD_PLATFORM_WINDOWS)
                    return false;
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Checks whether the entry is a Unix domain socket.
            /// Always false on Windows.
            /// 
            /// @return result<bool, runtime_error> - The check result, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto is_socket() const
                -> result<bool, runtime_error>
            {
                if (_exists == true)
                {
                    #if defined(HSD_PLATFORM_POSIX)
                    return S_ISSOCK(_status.st_mode);
                    #elif defined(HSD_PLATFORM_WINDOWS)
                    return false;
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Reads the owner/group/others permission bits. On
            /// Windows only the owner-level fields are populated (Windows
            /// stat has no group/other distinction).
            /// 
            /// @return result<fs_permissions, runtime_error> - The
            /// permission bits, or a `runtime_error` if the path did not exist.
            ///
            inline auto permissions() const
                -> result<fs_permissions, runtime_error>
            {
                if (_exists == true)
                {
                    #ifdef HSD_PLATFORM_WINDOWS
                    return fs_permissions {
                        .can_owner_read = (_status.st_mode & _S_IREAD) == _S_IREAD,
                        .can_owner_write = (_status.st_mode & _S_IWRITE) == _S_IWRITE,
                        .can_owner_exec = (_status.st_mode & _S_IEXEC) == _S_IEXEC,
                        .can_owner_do_everything = (_status.st_mode & _S_IEVER) == _S_IEVER,
                    };
                    #else
                    return fs_permissions {
                        .can_owner_read           = (_status.st_mode & S_IRUSR) == S_IRUSR,
                        .can_owner_write          = (_status.st_mode & S_IWUSR) == S_IWUSR,
                        .can_owner_exec           = (_status.st_mode & S_IXUSR) == S_IXUSR,
                        .can_owner_do_everything  = (_status.st_mode & S_IRWXU) == S_IRWXU,
                        .can_group_read           = (_status.st_mode & S_IRGRP) == S_IRGRP,
                        .can_group_write          = (_status.st_mode & S_IWGRP) == S_IWGRP,
                        .can_group_exec           = (_status.st_mode & S_IXGRP) == S_IXGRP,
                        .can_group_do_everything  = (_status.st_mode & S_IRWXG) == S_IRWXG,
                        .can_others_read          = (_status.st_mode & S_IROTH) == S_IROTH,
                        .can_others_write         = (_status.st_mode & S_IWOTH) == S_IWOTH,
                        .can_others_exec          = (_status.st_mode & S_IXOTH) == S_IXOTH,
                        .can_others_do_everything = (_status.st_mode & S_IRWXO) == S_IRWXO,
                        .can_all_do_everything    = (_status.st_mode & S_IRWXA) == S_IRWXA
                    };
                    #endif
                }

                return runtime_error{"File/Directory not found"};
            }

            ///
            /// @brief Gets the size in bytes of the entry.
            /// 
            /// @return result<usize, runtime_error> - The size, or a
            /// `runtime_error` if the path did not exist.
            ///
            inline auto size() const
                -> result<usize, runtime_error>
            {
                if (_exists == true)
                    return static_cast<usize>(_status.st_size);

                return runtime_error{"File/Directory not found"};
            }
        };
    } // namespace filesystem
    //#endif
} // namespace hsd