#pragma once

#include "String.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#include <windows.h>
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
            struct Location
            {
                ///
                /// @brief Holds the relative path of a file or directory.
                ///
                string _relative;

                ///
                /// @brief Holds the absolute path of a file or directory.
                ///
                string _absolute;

                ///
                /// @brief Constructs a new empty Location object
                ///
                inline Location() = default;

                ///
                /// @brief Constructs a new Location object.
                /// 
                /// @param relative_loc - The relative path of a file or directory.
                ///
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

                ///
                /// @brief Constructs a new Location object.
                /// 
                /// @param relative_loc - The relative path of a file or directory.
                ///
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

                ///
                /// @brief Constructs a new Location object.
                /// 
                /// @param relative_loc - The relative path of a file or directory.
                ///
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

                ///
                /// @brief Constructs by copy a new Location object.
                /// 
                /// @param other - The Location object to copy.
                ///
                inline Location(const Location& other)
                    : _relative{other._relative},
                      _absolute{other._absolute}
                {}

                ///
                /// @brief Constructs by move a new Location object.
                /// 
                /// @param other - The Location object to move.
                ///
                inline Location(Location&& other)
                    : _relative{move(other._relative)},
                      _absolute{move(other._absolute)}
                {}

                ///
                /// @brief Assigns by copy a new Location object.
                /// 
                /// @param rhs - The Location object to copy.
                /// @return Location& - A reference to the assigned Location object.
                ///
                inline Location& operator=(const Location& rhs)
                {
                    _relative = rhs._relative;
                    _absolute = rhs._absolute;
                    return *this;
                }

                ///
                /// @brief Assigns by move a new Location object.dd
                /// 
                /// @param rhs - The Location object to move.
                /// @return Location& - A reference to the assigned Location object.
                ///
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