#pragma once

#include "String.hpp"
#include <stdlib.h>

#if defined(HSD_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace hsd
{
    namespace filesystem
    {
        namespace fs_detail
        {
            struct Location
            {
                string _relative;
                string _absolute;

                inline Location() = default;
                inline ~Location() = default;

                inline Location(const char* relative_loc)
                    : _relative{relative_loc}
                {
                    char _buffer[2048]{};
                    
                    #if defined(HSD_PLATFORM_WINDOWS)
                    GetFullPathNameA(_relative.c_str(), 2048, _buffer, nullptr);
                    #else
                    realpath(_relative.c_str(), _buffer);
                    #endif
                    
                    _absolute = string{_buffer};
                }

                inline Location(const string& relative_loc)
                    : _relative{relative_loc}
                {
                    char _buffer[2048]{};
                    
                     #if defined(HSD_PLATFORM_WINDOWS)
                    GetFullPathNameA(_relative.c_str(), 2048, _buffer, nullptr);
                    #else
                    realpath(_relative.c_str(), _buffer);
                    #endif

                    _absolute = string{_buffer};
                }

                inline Location(string&& relative_loc)
                    : _relative{move(relative_loc)}
                {
                    char _buffer[2048]{};
                    
                     #if defined(HSD_PLATFORM_WINDOWS)
                    GetFullPathNameA(_relative.c_str(), 2048, _buffer, nullptr);
                    #else
                    realpath(_relative.c_str(), _buffer);
                    #endif

                    _absolute = string{_buffer};
                }

                inline Location(const Location& other)
                    : _relative{other._relative},
                      _absolute{other._absolute}
                {}

                inline Location(Location&& other)
                    : _relative{move(other._relative)},
                      _absolute{move(other._absolute)}
                {}

                inline Location& operator=(const Location& rhs)
                {
                    _relative = rhs._relative;
                    _absolute = rhs._absolute;
                    return *this;
                }

                inline Location& operator=(Location&& rhs)
                {
                    _relative = move(rhs._relative);
                    _absolute = move(rhs._absolute);
                    return *this;
                }
            };
        } // namespace fs_detail
    } // namespace filesystem
} // namespace hsd