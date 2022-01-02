#pragma once

#include <stdio.h>
#include <wchar.h>

#include "Pair.hpp"
#include "StringLiteral.hpp"

#define HSD_PRINT_USE_FMT(fmt, fmt_tag, tag_fmt, val_fmt, file_buf, val)\
do\
{\
    if constexpr (fmt.base.tag & fmt_tag)\
    {\
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            fprintf(\
                file_buf,\
                (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, fmt.base.background, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.fg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.bg)\
    {\
        fprintf(\
            file_buf,\
            (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt_tag)\
    {\
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            fwprintf(\
                file_buf,\
                (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, fmt.base.background, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.fg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.bg)\
    {\
        fwprintf(\
            file_buf,\
            (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt_tag)\
    {\
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            return snprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, fmt.base.background, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.fg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.bg)\
    {\
        return snprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt_tag)\
    {\
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.bg_fmt + tag_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
        if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, fmt.base.background, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.fg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.foreground, val\
            );\
        }\
        else if constexpr (fmt.base.tag & fmt.base.bg)\
        {\
            return swprintf(\
                dest.first, dest.second,\
                (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
                fmt.base.background, val\
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
    if constexpr (fmt.base.tag & fmt.base.fg && fmt.base.tag & fmt.base.bg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.fg_fmt + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, fmt.base.background, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.fg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.fg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.foreground, val\
        );\
    }\
    else if constexpr (fmt.base.tag & fmt.base.bg)\
    {\
        return swprintf(\
            dest.first, dest.second,\
            (fmt.format + fmt.base.bg_fmt + val_fmt + fmt.base.reset_fmt).data,\
            fmt.base.background, val\
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
    struct fmt_common
    {
        static constexpr const CharT fg_fmt[13] = {
            '\x1b', '[', '3', '8', ';', '5', ';', '%', 'h', 'h', 'u', 'm', '\0'
        };
        static constexpr const CharT bg_fmt[13] = {
            '\x1b', '[', '4', '8', ';', '5', ';', '%', 'h', 'h', 'u', 'm', '\0'
        };
        static constexpr const CharT reset_fmt[5] = {
            '\x1b', '[', '0', 'm', '\0'
        };

        static constexpr usize none = 0; 
        static constexpr usize hex  = 1; // hexadecimal
        static constexpr usize exp  = 2; // exponent
        static constexpr usize fg   = 4; // foreground color
        static constexpr usize bg   = 8; // background color

        usize tag;
        uchar foreground;
        uchar background;
    };

    template <typename CharT>
    struct format_info
    {
        const CharT* format;
        usize length;

        fmt_common<CharT> base;
    };

    template <typename CharT, usize N>
    struct format_literal
    {
        using char_type = CharT;
        basic_string_literal<CharT, N> format;

        fmt_common<CharT> base;
    };
} // namespace hsd
