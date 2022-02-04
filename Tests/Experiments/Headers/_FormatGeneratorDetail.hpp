#pragma once

#include <Tuple.hpp>
#include "LiteralParser.hpp"

namespace hsd
{
    template <typename CharT, fmt_common<CharT> info>
    static consteval auto get_foreground_format()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag & info.fg)
        {
            usize _index = 0;
            basic_string_literal<CharT, 9 + info.foreground.length> _str;
            
            for (_index = 0; _index < 7; ++_index)
            {
                _str.data[_index] = info.fg_fmt[_index];
            }

            for (_index = 0; _index < info.foreground.length; ++_index)
            {
                _str.data[_index + 7] = info.foreground.value[_index];
            }

            _str.data[_index + 7] = 'm';
            _str.data[_index + 8] = '\0';
            return _str;
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    template <typename CharT, fmt_common<CharT> info>
    static consteval auto get_background_format()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag & info.bg)
        {
            usize _index = 0;
            basic_string_literal<CharT, 9 + info.background.length> _str;
            
            for (_index = 0; _index < 7; ++_index)
            {
                _str.data[_index] = info.bg_fmt[_index];
            }

            for (_index = 0; _index < info.background.length; ++_index)
            {
                _str.data[_index + 7] = info.background.value[_index];
            }

            _str.data[_index + 7] = 'm';
            _str.data[_index + 8] = '\0';
            return _str;
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    template <typename CharT, fmt_common<CharT> info>
    static consteval auto get_reset_format()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag & info.fg || info.tag & info.bg)
        {
            return basic_string_literal<CharT, 5>{info.reset_fmt};
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    struct formatter_overloads
    {
        template <typename T>
        static constexpr bool is_i8 = is_same<decay_t<T>, i8>::value;

        template <typename T>
        static constexpr bool is_u8 = is_same<decay_t<T>, u8>::value;

        template <typename T>
        static constexpr bool is_i16 = is_same<decay_t<T>, i16>::value;

        template <typename T>
        static constexpr bool is_u16 = is_same<decay_t<T>, u16>::value;

        template <typename T>
        static constexpr bool is_i32 = is_same<decay_t<T>, i32>::value;

        template <typename T>
        static constexpr bool is_u32 = is_same<decay_t<T>, u32>::value;

        template <typename T>
        static constexpr bool is_i64 = is_same<decay_t<T>, i64>::value;

        template <typename T>
        static constexpr bool is_u64 = is_same<decay_t<T>, u64>::value;

        template <typename T>
        static constexpr bool is_ilong = is_same<decay_t<T>, ilong>::value;

        template <typename T>
        static constexpr bool is_ulong = is_same<decay_t<T>, ulong>::value;

        template <typename T>
        static constexpr bool is_f32 = is_same<decay_t<T>, f32>::value;

        template <typename T>
        static constexpr bool is_f64 = is_same<decay_t<T>, f64>::value;

        template <typename T>
        static constexpr bool is_f128 = is_same<decay_t<T>, f128>::value;

        template <typename T>
        static constexpr bool is_char_ptr = requires(T t)
        {
            static_cast<const char*>(t);
        };

        template <typename T>
        static constexpr bool is_wchar_ptr = requires(T t)
        {
            static_cast<const wchar_t*>(t);
        };
    };

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_i8<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<7>{"0x%hhx"};
            }
            else
            {
                return string_literal<5>{"%hhd"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<7>{L"0x%hhx"};
            }
            else
            {
                return wstring_literal<5>{L"%hhd"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_u8<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<7>{"0x%hhx"};
            }
            else
            {
                return string_literal<5>{"%hhu"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<7>{L"0x%hhx"};
            }
            else
            {
                return wstring_literal<5>{L"%hhu"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_i16<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<6>{"0x%hx"};
            }
            else
            {
                return string_literal<4>{"%hd"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<6>{L"0x%hx"};
            }
            else
            {
                return wstring_literal<4>{L"%hd"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_u16<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<6>{"0x%hx"};
            }
            else
            {
                return string_literal<4>{"%hu"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<6>{L"0x%hx"};
            }
            else
            {
                return wstring_literal<4>{L"%hu"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_i32<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<5>{"0x%x"};
            }
            else
            {
                return string_literal<3>{"%d"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<5>{L"0x%x"};
            }
            else
            {
                return wstring_literal<3>{L"%d"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_u32<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );
        
        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<5>{"0x%x"};
            }
            else
            {
                return string_literal<3>{"%u"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<5>{L"0x%x"};
            }
            else
            {
                return wstring_literal<3>{L"%u"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_i64<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<7>{"0x%llx"};
            }
            else
            {
                return string_literal<5>{"%lld"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<7>{L"0x%llx"};
            }
            else
            {
                return wstring_literal<5>{L"%lld"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_u64<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<7>{"0x%llx"};
            }
            else
            {
                return string_literal<5>{"%llu"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<7>{L"0x%llx"};
            }
            else
            {
                return wstring_literal<5>{L"%llu"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_ilong<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<6>{"0x%lx"};
            }
            else
            {
                return string_literal<4>{"%ld"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<6>{L"0x%lx"};
            }
            else
            {
                return wstring_literal<4>{L"%ld"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_ulong<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return string_literal<6>{"0x%lx"};
            }
            else
            {
                return string_literal<4>{"%lu"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.hex)
            {
                return wstring_literal<6>{L"0x%lx"};
            }
            else
            {
                return wstring_literal<4>{L"%lu"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_f32<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return string_literal<3>{"%e"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return string_literal<3>{"%a"};
            }
            else
            {
                return string_literal<3>{"%f"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return wstring_literal<3>{L"%e"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return wstring_literal<3>{L"%a"};
            }
            else
            {
                return wstring_literal<3>{L"%f"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_f64<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return string_literal<4>{"%le"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return string_literal<4>{"%la"};
            }
            else
            {
                return string_literal<4>{"%lf"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return wstring_literal<4>{L"%le"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return wstring_literal<4>{L"%la"};
            }
            else
            {
                return wstring_literal<4>{L"%lf"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_f128<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.ovr),
            "Invalid format tag"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return string_literal<4>{"%Le"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return string_literal<4>{"%La"};
            }
            else
            {
                return string_literal<4>{"%Lf"};
            }
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            if constexpr (info.tag & info.exp)
            {
                return wstring_literal<4>{L"%Le"};
            }
            else if constexpr (info.tag & info.hex)
            {
                return wstring_literal<4>{L"%La"};
            }
            else
            {
                return wstring_literal<4>{L"%Lf"};
            }
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_char_ptr<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.hex) && 
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr), 
            "hex, exp, and ovr flags"
            " are mutually exclusive"
            " for char*"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            return string_literal<3>{"%s"};
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            return wstring_literal<3>{L"%s"};
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_overloads::is_wchar_ptr<T>)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value,
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.hex) && 
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr), 
            "hex, exp, and ovr flags"
            " are mutually exclusive"
            " for wchar*"
        );

        return wstring_literal<4>{L"%ls"};
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (is_same<T, tuple<usize, const char*>>::value)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.hex) && 
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr), 
            "hex, exp, and ovr flags"
            " are mutually exclusive"
            " for tuple<usize, const char*>"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            return string_literal<5>{"%.*s"};
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            return wstring_literal<5>{L"%.*s"};
        }
    }

    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (is_same<T, tuple<usize, const wchar*>>::value)
    static consteval auto get_value_format_string()
    {
        static_assert(
            is_same<CharT, wchar>::value,
            "Unsupported character type"
        );

        static_assert(
            !(info.tag & info.hex) && 
            !(info.tag & info.exp) &&
            !(info.tag & info.ovr), 
            "hex, exp, and ovr flags"
            " are mutually exclusive"
            " for tuple<usize, const wchar*>"
        );

        return wstring_literal<6>{L"%.*ls"};
    }
} // namespace hsd