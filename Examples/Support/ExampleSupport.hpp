#pragma once

/// @file ExampleSupport.hpp
/// @brief Small helpers shared by the example programs.

#include <Containers/String.hpp>

namespace example_support
{
    /// @brief Returns a copy of `path` with every backslash replaced by a slash.
    /// @details Windows reports paths with backslashes. Printing slashes keeps the
    /// output of the examples the same on every platform.
    /// @param path Path text as the library reports it.
    /// @return The same path, using `/` as its separator.
    inline hsd::string with_forward_slashes(const hsd::string& path)
    {
        hsd::string _result = path;

        for (hsd::usize _idx = 0; _idx < _result.length(); ++_idx)
        {
            if (_result[_idx] == '\\')
            {
                _result[_idx] = '/';
            }
        }

        return _result;
    }
} // namespace example_support
