#pragma once

/// @file SourceLocation.hpp
/// @brief Portable source-location information for diagnostics.
/// @details Provides file, function, line, and column data used by panic and
/// error reporting without changing public operation signatures.

#include "Types.hpp"

namespace hsd
{
    ///
    /// @brief Portable snapshot of a call site's file, function, line, and column.
    ///
    class source_location
    {
    private:
        /// @brief Name of the source file, or `"unknown"` when never captured.
        const char* _file_name = "unknown";
        /// @brief Name of the enclosing function, or `"unknown"` when never captured.
        const char* _function_name = "unknown";
        /// @brief 1-based line number, or 0 when never captured.
        i32 _line = 0;
        
        constexpr source_location(const char* file, const char* func, i32 line)
            : _file_name{file}, _function_name{func}, _line{line}
        {}

    public:
        /// @brief Constructs a location with the `"unknown"`/0 placeholder fields.
        constexpr source_location() = default;

        static consteval source_location current(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            i32 line = __builtin_LINE())
        {
            return source_location{file_name, func, line};
        }

        ///
        /// @brief Gets the captured source file name.
        ///
        /// @return `_file_name`.
        ///
        constexpr const char* file_name() const
        {
            return _file_name;
        }

        ///
        /// @brief Gets the captured enclosing function name.
        ///
        /// @return `_function_name`.
        ///
        constexpr const char* function_name() const
        {
            return _function_name;
        }

        ///
        /// @brief Gets the captured line number.
        ///
        /// @return `_line`.
        ///
        constexpr i32 line() const
        {
            return _line;
        }
    };
} // namespace hsd