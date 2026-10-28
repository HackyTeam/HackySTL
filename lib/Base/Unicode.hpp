#pragma once

/// @file Unicode.hpp
/// @brief Unicode length and encoding conversion between UTF-8, UTF-16, UTF-32
/// and wide character buffers.
/// @details Conversions between types of the same width copy elements
/// unchanged. Conversions between widths decode each code point and reject
/// malformed input. A destination size counts its terminator, and a destination
/// that is too small is reported as an error without writing past its end.

#include "Result.hpp"

namespace hsd::unicode
{
    /// @brief Character types the conversions accept: 1, 2 and 4 byte units.
    template <typename CharT>
    concept UnicodeUnit = sizeof(CharT) == 1 || sizeof(CharT) == 2 || sizeof(CharT) == 4;

    namespace unicode_detail
    {
        /// @brief Whether a code point lies in the UTF-16 surrogate range.
        constexpr bool is_surrogate(char32 code_point)
        {
            return code_point >= 0xD800 && code_point <= 0xDFFF;
        }

        /// @brief Number of `CharT` code units that encode `code_point`.
        /// @tparam CharT Destination unit type.
        /// @param code_point Code point to measure.
        /// @return The unit count, between 1 and 4.
        template <UnicodeUnit CharT>
        constexpr usize encoded_units(char32 code_point)
        {
            if constexpr (sizeof(CharT) == 1)
            {
                if (code_point < 0x80)
                {
                    return 1;
                }
                if (code_point < 0x800)
                {
                    return 2;
                }
                if (code_point < 0x10000)
                {
                    return 3;
                }
                return 4;
            }
            else if constexpr (sizeof(CharT) == 2)
            {
                return code_point < 0x10000 ? 1 : 2;
            }
            else
            {
                return 1;
            }
        }

        /// @brief Writes the code units of `code_point` starting at `dest`.
        /// @tparam CharT Destination unit type.
        /// @param code_point Valid code point to encode.
        /// @param dest Buffer with room for `encoded_units<CharT>(code_point)` units.
        template <UnicodeUnit CharT>
        constexpr void encode(char32 code_point, CharT* dest)
        {
            if constexpr (sizeof(CharT) == 1)
            {
                switch (encoded_units<CharT>(code_point))
                {
                    case 1:
                        dest[0] = static_cast<CharT>(code_point);
                        break;
                    case 2:
                        dest[0] = static_cast<CharT>(0xC0 | (code_point >> 6));
                        dest[1] = static_cast<CharT>(0x80 | (code_point & 0x3F));
                        break;
                    case 3:
                        dest[0] = static_cast<CharT>(0xE0 | (code_point >> 12));
                        dest[1] = static_cast<CharT>(0x80 | ((code_point >> 6) & 0x3F));
                        dest[2] = static_cast<CharT>(0x80 | (code_point & 0x3F));
                        break;
                    default:
                        dest[0] = static_cast<CharT>(0xF0 | (code_point >> 18));
                        dest[1] = static_cast<CharT>(0x80 | ((code_point >> 12) & 0x3F));
                        dest[2] = static_cast<CharT>(0x80 | ((code_point >> 6) & 0x3F));
                        dest[3] = static_cast<CharT>(0x80 | (code_point & 0x3F));
                        break;
                }
            }
            else if constexpr (sizeof(CharT) == 2)
            {
                if (code_point < 0x10000)
                {
                    dest[0] = static_cast<CharT>(code_point);
                }
                else
                {
                    const char32 offset = code_point - 0x10000;
                    dest[0] = static_cast<CharT>(0xD800 | (offset >> 10));
                    dest[1] = static_cast<CharT>(0xDC00 | (offset & 0x3FF));
                }
            }
            else
            {
                dest[0] = static_cast<CharT>(code_point);
            }
        }

        /// @brief Decodes the code point that starts at `src`.
        /// @tparam CharT Source unit type.
        /// @param src First source unit; must not be null when `avail` is not zero.
        /// @param avail Number of source units that may be read.
        /// @param code_point Receives the decoded code point on success.
        /// @return Source units consumed, or 0 if the sequence is truncated or malformed.
        template <UnicodeUnit CharT>
        constexpr usize decode(const CharT* src, usize avail, char32& code_point)
        {
            using unsigned_t = add_unsigned_t<CharT>;

            if (avail == 0)
            {
                return 0;
            }

            const auto first = static_cast<unsigned_t>(src[0]);

            if constexpr (sizeof(CharT) == 1)
            {
                if (first < 0x80)
                {
                    code_point = static_cast<char32>(first);
                    return 1;
                }

                usize units = 0;
                char32 minimum = 0;
                if ((first & 0xE0) == 0xC0)
                {
                    units = 2;
                    minimum = 0x80;
                    code_point = static_cast<char32>(first & 0x1F);
                }
                else if ((first & 0xF0) == 0xE0)
                {
                    units = 3;
                    minimum = 0x800;
                    code_point = static_cast<char32>(first & 0x0F);
                }
                else if ((first & 0xF8) == 0xF0)
                {
                    units = 4;
                    minimum = 0x10000;
                    code_point = static_cast<char32>(first & 0x07);
                }
                else
                {
                    return 0;
                }

                if (avail < units)
                {
                    return 0;
                }

                for (usize index = 1; index < units; ++index)
                {
                    const auto next = static_cast<unsigned_t>(src[index]);
                    if ((next & 0xC0) != 0x80)
                    {
                        return 0;
                    }
                    code_point = (code_point << 6) | static_cast<char32>(next & 0x3F);
                }

                if (code_point < minimum || code_point > 0x10FFFF || is_surrogate(code_point))
                {
                    return 0;
                }
                return units;
            }
            else if constexpr (sizeof(CharT) == 2)
            {
                if ((first & 0xFC00) == 0xDC00)
                {
                    return 0;
                }
                if ((first & 0xFC00) != 0xD800)
                {
                    code_point = static_cast<char32>(first);
                    return 1;
                }
                if (avail < 2)
                {
                    return 0;
                }

                const auto second = static_cast<unsigned_t>(src[1]);
                if ((second & 0xFC00) != 0xDC00)
                {
                    return 0;
                }

                code_point = 0x10000 + ((static_cast<char32>(first & 0x3FF) << 10) | static_cast<char32>(second & 0x3FF));
                return 2;
            }
            else
            {
                if (first > 0x10FFFF || is_surrogate(first))
                {
                    return 0;
                }
                code_point = static_cast<char32>(first);
                return 1;
            }
        }
    } // namespace unicode_detail

    /// @brief Computes the number of `CharT1` units a source string transcodes to.
    /// @tparam CharT1 Destination unit type used for the measurement.
    /// @tparam CharT2 Source unit type.
    /// @param src Source buffer; may be null when `src_len` is zero.
    /// @param src_len Maximum number of source units to scan (defaults to unbounded).
    /// @return Units the transcoded text occupies, excluding the terminator. A
    /// malformed source unit counts as one unit here; `convert_to` reports it as an error.
    /// @details Scanning stops at the first null unit or after `src_len` units,
    /// and never reads past either limit.
    template <UnicodeUnit CharT1, UnicodeUnit CharT2>
    static constexpr usize length(const CharT2* src, usize src_len = -1)
    {
        if (src == nullptr)
        {
            return 0;
        }

        usize count = 0;
        usize in = 0;
        while (in < src_len && src[in] != static_cast<CharT2>(0))
        {
            if constexpr (sizeof(CharT1) == sizeof(CharT2))
            {
                ++count;
                ++in;
            }
            else
            {
                char32 code_point = 0;
                const usize used = unicode_detail::decode(src + in, src_len - in, code_point);
                if (used == 0)
                {
                    ++count;
                    ++in;
                    continue;
                }
                count += unicode_detail::encoded_units<CharT1>(code_point);
                in += used;
            }
        }
        return count;
    }

    /// @brief Transcodes a source string into a destination buffer.
    /// @tparam CharT1 Destination unit type.
    /// @tparam CharT2 Source unit type.
    /// @param dest Destination buffer; receives the converted text and a null terminator.
    /// @param size Capacity of `dest` in `CharT1` units, including the terminator.
    /// @param src Source buffer; may be null when `src_len` is zero.
    /// @param src_len Maximum number of source units to consume (defaults to unbounded).
    /// @return Units left in `dest` after the text and its terminator, or a
    /// `runtime_error` if `dest` is too small or the source is malformed.
    /// @details The source is read up to its first null unit or `src_len` units,
    /// whichever comes first. Nothing is written at or past `dest + size`.
    template <UnicodeUnit CharT1, UnicodeUnit CharT2>
    static constexpr result<usize, runtime_error> convert_to(
        CharT1* dest, usize size, const CharT2* src, usize src_len = -1)
    {
        if (size == 0)
        {
            return runtime_error{"Destination too small for conversion"};
        }
        if (src == nullptr)
        {
            src_len = 0;
        }

        usize out = 0;
        usize in = 0;
        while (in < src_len && src[in] != static_cast<CharT2>(0))
        {
            if constexpr (sizeof(CharT1) == sizeof(CharT2))
            {
                if (out + 1 >= size)
                {
                    return runtime_error{"Destination too small for conversion"};
                }
                dest[out++] = static_cast<CharT1>(src[in++]);
            }
            else
            {
                char32 code_point = 0;
                const usize used = unicode_detail::decode(src + in, src_len - in, code_point);
                if (used == 0)
                {
                    return runtime_error{"Malformed source sequence"};
                }

                const usize units = unicode_detail::encoded_units<CharT1>(code_point);
                if (out + units + 1 > size)
                {
                    return runtime_error{"Destination too small for conversion"};
                }

                unicode_detail::encode(code_point, dest + out);
                out += units;
                in += used;
            }
        }

        dest[out] = static_cast<CharT1>(0);
        return size - out - 1;
    }
} // namespace hsd::unicode
