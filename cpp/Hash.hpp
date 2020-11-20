#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename HashType>
    struct fnv1a
    {
        template <typename T>
        static constexpr HashType get_hash(T begin)
        {
            HashType offset_basis = 0;
            HashType prime = 0;

            if constexpr(sizeof(HashType) == sizeof(u64))
            {
                offset_basis = 14'695'981'039'346'656'037u;
                prime = 1'099'511'628'211u;
            }
            else if constexpr(sizeof(HashType) == sizeof(u32)) 
            {
                offset_basis = 2'166'136'261u;
                prime = 16'777'619u;
            }

            HashType hash = offset_basis;

            while(*begin != '\0') 
            {
                hash = (hash ^ static_cast<HashType>(*begin)) * prime;
                begin++;
            }

            return hash;
        }

        template <typename T> requires (is_integral<T>::value)
        static constexpr HashType get_hash(T number)
        {
            return static_cast<HashType>(number);
        }
    };
} // namespace hsd