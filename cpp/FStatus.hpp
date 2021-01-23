#pragma once

#include "Result.hpp"

#if defined(HSD_PLATFORM_POSIX)
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace hsd
{
    #if defined(HSD_PLATFORM_POSIX)
    namespace filesystem
    {
        class fs_status
        {
        private:
            struct stat _status;
            bool _exists = false;

        public:
            inline fs_status(const char* filename)
            {
                _exists = lstat(filename, &_status) != -1;
            }

            inline bool operator==(const fs_status& rhs) const
            {
                return (
                    _status.st_atim.tv_nsec == rhs._status.st_atim.tv_nsec &&
                    _status.st_ctim.tv_nsec == rhs._status.st_ctim.tv_nsec &&
                    _status.st_mtim.tv_nsec == rhs._status.st_mtim.tv_nsec &&
                    _status.st_atim.tv_sec  == rhs._status.st_atim.tv_sec  &&
                    _status.st_ctim.tv_sec  == rhs._status.st_ctim.tv_sec  &&
                    _status.st_mtim.tv_sec  == rhs._status.st_mtim.tv_sec  &&
                    _status.st_blksize      == rhs._status.st_blksize      &&
                    _status.st_blocks       == rhs._status.st_blocks       &&
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

            inline bool exists() const
            {
                return _exists;
            }

            inline auto is_directory() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISDIR(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_character() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISCHR(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_block_file() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISBLK(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_regular_file() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISREG(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_fifo_file() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISFIFO(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_symlink() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISLNK(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto is_socket() const
                -> Result<bool, runtime_error>
            {
                if(_exists == true)
                    return S_ISSOCK(_status.st_mode);

                return runtime_error{"File/Directory not found"};
            }

            inline auto size() const
                -> Result<usize, runtime_error>
            {
                if(_exists == true)
                    return _status.st_size;

                return runtime_error{"File/Directory not found"};
            }
        };
    } // namespace filesystem
    #endif
} // namespace hsd