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
            static inline sstream<4096> _io_buf{};
            static inline wsstream<4096> _wio_buf{};
        };

        template <auto>
        class printer;

        template <hsd::format_literal fmt>
        requires (hsd::IsSame<char, typename decltype(fmt)::char_type>)
        class printer<fmt>
        {
        private:
            FILE* _file;

        public:
            printer(FILE* file)
                : _file{file}
            {}

            friend void _print_impl(printer& p)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                fputs(fmt.format.data, p._file);
            }

            friend void _print_impl(printer& p, bool val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                if (val == true)
                {
                    HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", p._file, "true");
                }
                else
                {
                    HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", p._file, "false");
                }
            }

            friend void _print_impl(printer& p, char val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hhx", "%c", p._file, val);
            }

            friend void _print_impl(printer& p, char8 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hhx", "%c", p._file, val);
            }

            friend void _print_impl(printer& p, i16 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hx", "%hd", p._file, val);
            }

            friend void _print_impl(printer& p, u16 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%hx", "%hu", p._file, val);
            }

            friend void _print_impl(printer& p, i32 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%x", "%d", p._file, val);
            }

            friend void _print_impl(printer& p, u32 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%x", "%u", p._file, val);
            }

            friend void _print_impl(printer& p, i64 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%llx", "%lld", p._file, val);
            }

            friend void _print_impl(printer& p, u64 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%llx", "%llu", p._file, val);
            }

            friend void _print_impl(printer& p, long val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%lx", "%ld", p._file, val);
            }

            friend void _print_impl(printer& p, ulong val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.hex, "0x%lx", "%lu", p._file, val);
            }

            friend void _print_impl(printer& p, f32 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.exp, "%e", "%f", p._file, val);
            }

            friend void _print_impl(printer& p, f64 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.exp, "%le", "%lf", p._file, val);
            }

            friend void _print_impl(printer& p, f128 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_PRINT_USE_FMT(fmt, fmt.exp, "%Le", "%Lf", p._file, val);
            }

            friend void _print_impl(printer& p, const char* val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", p._file, val);
            }

            friend void _print_impl(printer& p, const char8* val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                HSD_PRINT_USE_FMT_UNTAGGED(fmt, "%s", p._file, val);
            }
        };

        template <hsd::format_literal fmt>
        requires (hsd::IsSame<wchar, typename decltype(fmt)::char_type>)
        class printer<fmt>
        {
        private:
            FILE* _file;

        public:
            inline printer(FILE* file)
                : _file{file}
            {}

            auto* file()
            {
                return _file;
            }

            friend void _print_impl(printer& p)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                fputws(fmt.format.data, p._file);
            }

            friend void _print_impl(printer& p, bool val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                if (val == true)
                {
                    HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", p._file, "true");
                }
                else
                {
                    HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", p._file, "false");
                }
            }

            friend void _print_impl(printer& p, char val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%c", p._file, val);
            }

            friend void _print_impl(printer& p, char8 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hhx", L"%c", p._file, val);
            }

            friend void _print_impl(printer& p, i16 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hd", p._file, val);
            }

            friend void _print_impl(printer& p, u16 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%hx", L"%hu", p._file, val);
            }

            friend void _print_impl(printer& p, i32 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%x", L"%d", p._file, val);
            }

            friend void _print_impl(printer& p, u32 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%x", L"%u", p._file, val);
            }

            friend void _print_impl(printer& p, i64 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%lld", p._file, val);
            }

            friend void _print_impl(printer& p, u64 val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%llx", L"%llu", p._file, val);
            }

            friend void _print_impl(printer& p, long val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%ld", p._file, val);
            }

            friend void _print_impl(printer& p, ulong val)
            requires (!(fmt.tag & fmt.exp))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.hex, L"0x%lx", L"%lu", p._file, val);
            }

            friend void _print_impl(printer& p, f32 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%e", L"%f", p._file, val);
            }

            friend void _print_impl(printer& p, f64 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%le", L"%lf", p._file, val);
            }

            friend void _print_impl(printer& p, f128 val)
            requires (!(fmt.tag & fmt.hex))
            {
                HSD_WPRINT_USE_FMT(fmt, fmt.exp, L"%Le", L"%Lf", p._file, val);
            }

            friend void _print_impl(printer& p, const char* val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", p._file, val);
            }

            friend void _print_impl(printer& p, const char8* val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                HSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%s", p._file, val);
            }

            friend void _print_impl(printer& p, const wchar* val)
            requires (!((fmt.tag & fmt.hex) || (fmt.tag & fmt.exp)))
            {
                fHSD_WPRINT_USE_FMT_UNTAGGED(fmt, L"%ls", p._file, val);
            }
        };
    } // namespace io_detail
} // namespace hsd