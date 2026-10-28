#pragma once

/// @file _ToStringBase.hpp
/// @brief Internal fixed-buffer storage for string conversion algorithms.
/// @details Provides bounded forward and reverse character buffers used by the
/// public numeric conversion functions.

#include "../Base/Types.hpp"

namespace hsd::to_string_detail
{
    /// @brief Fixed-size character buffer filled from the back towards the front.
    /// @tparam CharT Character type stored in the buffer.
    /// @tparam N Total capacity of the backing array.
    /// @details Used to build digit strings least-significant-digit first
    /// (integer and hex conversions) without any reversal step afterwards.
    template <typename CharT, usize N>
    class to_string_value_rev
    {
    private:
        /// @brief Index of the first written character; decreases as `push` is called.
        usize _begin_idx = N;
        /// @brief Backing storage; only `[_begin_idx, N)` holds initialized data.
        CharT _arr[N]{};

    public:
        /// @brief Writes `val` immediately before the previously written characters.
        /// @param val Character to prepend to the buffer.
        /// @details No bounds check is performed; `N` must be large enough for the
        /// maximum digit count of the value being converted.
        constexpr void push(CharT val)
        {
            _arr[--_begin_idx] = val;
        }

        /// @brief Returns a pointer to the first written character.
        /// @return Same as `begin()`.
        constexpr const auto* data() const
        {
            return begin();
        }

        /// @brief Returns the number of characters written so far.
        /// @return `N - _begin_idx`.
        constexpr usize size() const
        {
            return N - _begin_idx;
        }

        /// @brief Returns an iterator to the first written character.
        /// @return Pointer into the backing array at `_begin_idx`.
        constexpr const auto* begin() const
        {
            return _arr + _begin_idx;
        }

        /// @brief Returns an iterator one past the last written character.
        /// @return Pointer to the fixed end of the backing array.
        constexpr const auto* end() const
        {
            return _arr + N;
        }
    };

    /// @brief Fixed-size character buffer filled from the front towards the back.
    /// @tparam CharT Character type stored in the buffer.
    /// @tparam N Total capacity of the backing array.
    /// @details Used to build strings in natural left-to-right order (e.g. the
    /// digits emitted by `write_fixed_point`).
    template <typename CharT, usize N>
    class to_string_value_for
    {
    private:
        /// @brief Number of characters written so far; also the next write index.
        usize _end_idx = 0;
        /// @brief Backing storage; only `[0, _end_idx)` holds initialized data.
        CharT _arr[N]{};
    
    public:
        /// @brief Appends `val` after the previously written characters.
        /// @param val Character to append to the buffer.
        /// @details No bounds check is performed; `N` must be large enough for the
        /// maximum digit count of the value being converted.
        constexpr void push(CharT val)
        {
            _arr[_end_idx++] = val;
        }
    
        /// @brief Returns a pointer to the first written character.
        /// @return Same as `begin()`.
        constexpr const auto* data() const
        {
            return begin();
        }
    
        /// @brief Returns the number of characters written so far.
        /// @return Current value of `_end_idx`.
        constexpr usize size() const
        {
            return _end_idx;
        }
    
        /// @brief Returns an iterator to the first written character.
        /// @return Pointer to the start of the backing array.
        constexpr const auto* begin() const
        {
            return _arr;
        }
    
        /// @brief Returns an iterator one past the last written character.
        /// @return Pointer to the backing array at `_end_idx`.
        constexpr const auto* end() const
        {
            return _arr + _end_idx;
        }
    };
} // namespace hsd:: to_string_detail