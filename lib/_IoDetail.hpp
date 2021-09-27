#pragma once

#include <locale.h>

#include "SStream.hpp"

namespace hsd
{
    namespace io_detail
    {
        class bufferable
        {
        protected:
            static inline sstream _io_buf{4096};
            static inline wsstream _wio_buf{4096};
        };

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp))
        )
        static inline void _print(FILE* file_buf = stdout)
        {
            fputs(fmt.format.data, file_buf);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp))
        )
        static inline void _print(bool val, FILE* file_buf = stdout)
        {
            if (val == true)
            {
                HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", file_buf, "true");
            }
            else
            {
                HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", file_buf, "false");
            }
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(char val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hhx", "%c", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(char8 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hhx", "%c", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i16 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hx", "%hd", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u16 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hx", "%hu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i32 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%x", "%d", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u32 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%x", "%u", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i64 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%llx", "%lld", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u64 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%llx", "%llu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(long val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%lx", "%ld", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(ulong val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%lx", "%lu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f32 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.exp, "%e", "%f", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f64 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.exp, "%le", "%lf", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f128 val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT(fmt, fmt.exp, "%Le", "%Lf", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp))
        )
        static inline void _print(const char* val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<char, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.hex))
        )
        static inline void _print(const char8* val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.hex))
        )
        static inline void _print(FILE* file_buf = stdout)
        {
            fputws(fmt.format.data, file_buf);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.hex))
        )
        static inline void _print(bool val, FILE* file_buf = stdout)
        {
            if (val == true)
            {
                HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", file_buf, "true");
            }
            else
            {
                HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", file_buf, "false");
            }
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp || fmt.tag & fmt.hex)
        )
        static inline void _print(wchar val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%lc", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(char val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%c", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(char8 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%c", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i16 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hd", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u16 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i32 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%x", L"%d", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u32 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%x", L"%u", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(i64 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%lld", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(u64 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%llu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(long val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%ld", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.exp)
        )
        static inline void _print(ulong val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%lu", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f32 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%e", L"%f", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f64 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%le", L"%lf", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !(fmt.tag & fmt.hex)
        )
        static inline void _print(f128 val, FILE* file_buf = stdout)
        {
            HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%Le", L"%Lf", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.hex))
        )
        static inline void _print(const char* val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT_UNTAGGED(fmt, L"%s", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.exp))
        )
        static inline void _print(const char8* val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT_UNTAGGED(fmt, L"%s", file_buf, val);
        }

        template <format_literal fmt>
        requires (
            IsSame<wchar, typename decltype(fmt)::char_type> &&
            !((fmt.tag & fmt.exp) || (fmt.tag & fmt.hex))
        )
        static inline void _print(const wchar* val, FILE* file_buf = stdout)
        {
            HSD_PRINT_USE_FMT_UNTAGGED(fmt, L"%ls", file_buf, val);
        }
    }
} // namespace hsd