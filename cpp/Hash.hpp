#pragma once

#include "Utility.hpp"

namespace hsd
{
    template <typename hash_t>
    struct fnv1a
    {
        template<typename T>
        static constexpr hash_t get_hash(T begin)
        {
            hash_t offset_basis = 0;
            hash_t prime = 0;

            if constexpr(sizeof(hash_t) == sizeof(uint64_t))
            {
                offset_basis = 14'695'981'039'346'656'037u;
                prime = 1'099'511'628'211u;
            }
            else if constexpr(sizeof(hash_t) == sizeof(uint32_t)) 
            {
                offset_basis = 2'166'136'261u;
                prime = 16'777'619u;
            }

            hash_t hash = offset_basis;

            if constexpr(is_char_pointer<T>::value)
            {
                while(*begin != '\0') 
                {
                    hash = (hash ^ static_cast<hash_t>(*begin)) * prime;
                    begin++;
                }
            }
            else
            {
                hash = (hash ^ static_cast<hash_t>(begin)) * prime;
            }

            return hash;
        }
    };
} // namespace hsd