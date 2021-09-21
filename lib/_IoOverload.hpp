#pragma once

#include <stdio.h>
#include <wchar.h>

#include "Pair.hpp"
#include "StringLiteral.hpp"
#include "Concepts.hpp"

#define HSD_PRINT_USE_FMT(fmt, fmt_tag, tag_fmt, val_fmt, file_buf, val)\
do\
{\
    if constexpr (fmt.tag & fmt_tag)\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            fprintf(\
                file_buf, (fmt.format + tag_fmt).data, val\
            );\
        }\
    }\
    else\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            fprintf(\
                file_buf, (fmt.format + val_fmt).data, val\
            );\
        }\
    }\
} while (0)

#define HSD_PRINT_USE_FMT_UNTAGGED(fmt, val_fmt, file_buf, val)\
do\
{\
    if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, fmt.background, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.fg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.bg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.background, val\
        );\
    }\
    else\
    {\
        fprintf(\
            file_buf, (fmt.format + val_fmt).data, val\
        );\
    }\
} while (0)

#define HSD_WPRINT_USE_FMT(fmt, fmt_tag, tag_fmt, val_fmt, file_buf, val)\
do\
{\
    if constexpr (fmt.tag & fmt_tag)\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            fwprintf(\
                file_buf, (fmt.format + tag_fmt).data, val\
            );\
        }\
    }\
    else\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            fwprintf(\
                file_buf, (fmt.format + val_fmt).data, val\
            );\
        }\
    }\
} while (0)

#define HSD_WPRINT_USE_FMT_UNTAGGED(fmt, val_fmt, file_buf, val)\
do\
{\
    if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, fmt.background, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.fg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.bg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.background, val\
        );\
    }\
    else\
    {\
        fwprintf(\
            file_buf, (fmt.format + val_fmt).data, val\
        );\
    }\
} while (0)

#define HSD_SSTREAM_USE_FMT(fmt, fmt_tag, tag_fmt, val_fmt, dest, val)\
do\
{\
    if constexpr (fmt.tag & fmt_tag)\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            return snprintf(\
                dest.first, dest.second, (fmt.format + tag_fmt).data, val\
            );\
        }\
    }\
    else\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            return snprintf(\
                dest.first, dest.second, (fmt.format + val_fmt).data, val\
            );\
        }\
    }\
} while (0)

#define HSD_SSTREAM_USE_FMT_UNTAGGED(fmt, val_fmt, dest, val)\
do\
{\
    if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, fmt.background, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.fg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.bg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.background, val\
        );\
    }\
    else\
    {\
        return snprintf(\
            dest.first, dest.second, (fmt.format + val_fmt).data, val\
        );\
    }\
} while (0)

#define HSD_WSSTREAM_USE_FMT(fmt, fmt_tag, tag_fmt, val_fmt, dest, val)\
do\
{\
    if constexpr (fmt.tag & fmt_tag)\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.bg_fmt + tag_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            return swprintf(\
                dest.first, dest.second, (fmt.format + tag_fmt).data, val\
            );\
        }\
    }\
    else\
    {\
        if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, fmt.background, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.fg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.foreground, val\
            );\
        }\
        else if constexpr (fmt.tag & fmt.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
                fmt.background, val\
            );\
        }\
        else\
        {\
            return swprintf(\
                dest.first, dest.second, (fmt.format + val_fmt).data, val\
            );\
        }\
    }\
} while (0)

#define HSD_WSSTREAM_USE_FMT_UNTAGGED(fmt, val_fmt, dest, val)\
do\
{\
    if constexpr (fmt.tag & fmt.fg && fmt.tag & fmt.bg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.fg_fmt + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, fmt.background, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.fg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.fg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.foreground, val\
        );\
    }\
    else if constexpr (fmt.tag & fmt.bg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.bg_fmt + val_fmt + fmt.reset_fmt).data,\
            fmt.background, val\
        );\
    }\
    else\
    {\
        return swprintf(\
            dest.first, dest.second, (fmt.format + val_fmt).data, val\
        );\
    }\
} while (0)
namespace hsd
{
    template <typename CharT>
    struct format_info
    {
        const CharT* format;
        usize length;

        static constexpr uchar none = 0; 
        static constexpr uchar hex  = 1; // hexadecimal
        static constexpr uchar exp  = 2; // exponent
        static constexpr uchar fg   = 4; // foreground color
        static constexpr uchar bg   = 8; // background color

        uchar tag;
        uchar foreground;
        uchar background;
    };

    template <typename CharT, usize N>
    struct format_literal
    {
        using char_type = CharT;
        basic_string_literal<CharT, N> format;

        static constexpr const CharT fg_fmt[] = {
            '\x1b', '[', '3', '8', ';', '5', ';', '%', 'h', 'h', 'u', 'm', '\0'
        };
        static constexpr const CharT bg_fmt[] = {
            '\x1b', '[', '4', '8', ';', '5', ';', '%', 'h', 'h', 'u', 'm', '\0'
        };
        static constexpr const CharT reset_fmt[] = {
            '\x1b', '[', '0', 'm', '\0'
        };

        static constexpr uchar none = 0; 
        static constexpr uchar hex  = 1; // hexadecimal
        static constexpr uchar exp  = 2; // exponent
        static constexpr uchar fg   = 4; // foreground color
        static constexpr uchar bg   = 8; // background color

        uchar tag;
        uchar foreground;
        uchar background;
    };

    namespace sstream_detail
    {
        template <typename CharT>
        inline void _parse(pair<const CharT*, usize>&, auto&) = delete;

        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline i32 _write(auto, pair<char*, usize>) = delete;

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline i32 _write(auto, pair<wchar*, usize>) = delete;
    } // namespace sstream_detail

    namespace io_detail
    {
        template <format_literal fmt>
        requires (IsSame<char, typename decltype(fmt)::char_type>)
        inline void _print(auto, FILE*) = delete;

        template <format_literal fmt>
        requires (IsSame<wchar, typename decltype(fmt)::char_type>)
        inline void _print(auto, FILE*) = delete;
    } // namespace io_detail
} // namespace hsd
