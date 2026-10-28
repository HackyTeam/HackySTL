#pragma once

/// @file Hash.hpp
/// @brief Hash functions and hash customization points.
/// @details Provides deterministic hashing for integral, pointer, character,
/// and project string-like values used by unordered_map and related features.

#include "Concepts.hpp"

namespace hsd
{
    /// @brief Hash customization point parameterized by result and key types.
    /// @tparam HashType Integer type returned by get_hash.
    /// @tparam T Key type selected by a specialization.
    template < typename HashType, typename T = void >
    struct hash
    {
        /// @brief Hash result type exposed by the customization point.
        using ResultType = HashType;
    };

    /// @brief FNV-1a hash specialization for iterable character-like ranges.
    /// @tparam HashType Integer type used for the accumulator and result.
    /// @tparam T Iterator or range endpoint type.
    template < typename HashType, typename T >
    requires (ForwardIterable<T> || ReverseIterable<T>)
    struct hash<HashType, T>
    {
        /// @brief Hash result type exposed by the customization point.
        using ResultType = HashType;

        /// @brief Hashes a null-terminated sequence with FNV-1a.
        /// @param begin Pointer or iterator to the first element.
        /// @return FNV-1a hash accumulated until a null element is reached.
        static constexpr ResultType get_hash(T begin)
        {
            HashType offset_basis = 0;
            HashType prime = 0;

            if constexpr (sizeof(HashType) == sizeof(u64))
            {
                offset_basis = 14'695'981'039'346'656'037u;
                prime = 1'099'511'628'211u;
            }
            else if constexpr (sizeof(HashType) == sizeof(u32)) 
            {
                offset_basis = 2'166'136'261u;
                prime = 16'777'619u;
            }

            HashType hash = offset_basis;

            while (*begin != '\0') 
            {
                hash = (hash ^ static_cast<HashType>(*begin)) * prime;
                begin++;
            }

            return hash;
        }

        /// @brief Hashes a bounded forward range with FNV-1a.
        /// @param begin Iterator to the first element.
        /// @param end Iterator one past the final element.
        /// @return FNV-1a hash accumulated over `[begin, end)`.
        static constexpr ResultType get_hash(T begin, T end)
        requires (ForwardIterable<T>)
        {
            HashType offset_basis = 0;
            HashType prime = 0;

            if constexpr (sizeof(HashType) == sizeof(u64))
            {
                offset_basis = 14'695'981'039'346'656'037u;
                prime = 1'099'511'628'211u;
            }
            else if constexpr (sizeof(HashType) == sizeof(u32)) 
            {
                offset_basis = 2'166'136'261u;
                prime = 16'777'619u;
            }

            HashType hash = offset_basis;

            while (begin != end) 
            {
                hash = (hash ^ static_cast<HashType>(*begin)) * prime;
                begin++;
            }

            return hash;
        }
    };

    /// @brief Integral hash specialization using value conversion.
    /// @tparam HashType Integer type returned by get_hash.
    /// @tparam T Integral key type.
    template < typename HashType, typename T >
    requires (is_integral<T>::value)
    struct hash<HashType, T>
    {
        /// @brief Hash result type exposed by the customization point.
        using ResultType = HashType;

        /// @brief Converts an integral key to the requested hash type.
        /// @param number Integral key to hash.
        /// @return `number` converted to `HashType`.
        static constexpr ResultType get_hash(T number)
        {
            return static_cast<HashType>(number);
        }
    };
} // namespace hsd