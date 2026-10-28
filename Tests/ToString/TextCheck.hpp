#pragma once

/// @file TextCheck.hpp
/// @brief Comparison helpers shared by the ToString tests.

#include <Base/Types.hpp>

/// @brief Largest text the tests format, sized for the widest fixed-point long double.
inline constexpr hsd::usize max_text_length = 5200;

// Compares every character of a conversion buffer with a null-terminated expected
// text. Usable in constant expressions, so one check serves compile time and run time.
template <typename Buf, typename CharT>
static constexpr bool equals(const Buf& buf, const CharT* expected)
{
    hsd::usize _idx = 0;

    for (; expected[_idx] != static_cast<CharT>(0); ++_idx)
    {
        if (_idx >= buf.size() || buf.data()[_idx] != expected[_idx])
        {
            return false;
        }
    }

    return _idx == buf.size();
}
