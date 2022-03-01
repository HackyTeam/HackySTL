#pragma once

#include "Tuple.hpp"
#include "LiteralParser.hpp"

namespace hsd
{
    ///
    /// @brief Get the foreground string value out of the style info
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The style info with its fields
    /// @return A compile time string with the foreground color escape sequence
    ///
    template <typename CharT, fmt_style<CharT> info>
    static consteval auto get_foreground_attr()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag & info.foregr)
        {
            usize _index = 0;
            basic_string_literal<CharT, 6 + info.foreground.length> _str;
            
            for (_index = 0; _index < 5; ++_index)
            {
                _str.data[_index] = info.fg_base.data[_index];
            }

            for (_index = 0; _index < info.foreground.length; _index++)
            {
                _str.data[_index + 5] = info.foreground.value[_index];
            }

            _str.data[_index + 5] = '\0';
            return _str;
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    ///
    /// @brief Get the background string value out of the style info
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The style info with its fields
    /// @return A compile time string with the background color escape sequence
    ///
    template <typename CharT, fmt_style<CharT> info>
    static consteval auto get_background_attr()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag & info.backgr)
        {
            usize _index = 0;
            basic_string_literal<CharT, 6 + info.background.length> _str;
            
            for (_index = 0; _index < 5; ++_index)
            {
                _str.data[_index] = info.bg_base.data[_index];
            }

            for (_index = 0; _index < info.background.length; _index++)
            {
                _str.data[_index + 5] = info.background.value[_index];
            }

            _str.data[_index + 5] = '\0';
            return _str;
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    ///
    /// @brief Get the reset string value out
    /// of the style info if there is any
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The style info with its fields
    /// @return A compile time string with the reset escape sequence
    ///
    template <typename CharT, fmt_style<CharT> info>
    static consteval auto get_reset_attr()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr (info.tag != 0)
        {
            return basic_string_literal<CharT, 5>{info.reset_attr};
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    ///
    /// @brief Get the style string value out of the style info
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam tag - mask of the style
    /// @tparam info - The style info with its fields
    /// @return A compile time string with the style value
    ///
    template <typename CharT, usize tag, fmt_style<CharT> info>
    static consteval auto get_style_attr_impl()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        if constexpr ((info.tag & tag) == info.bold)
        {
            return basic_string_literal<CharT, 2>{{'1', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.dim)
        {
            return basic_string_literal<CharT, 2>{{'2', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.italic)
        {
            return basic_string_literal<CharT, 2>{{'3', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.undrln)
        {
            return basic_string_literal<CharT, 2>{{'4', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.blink)
        {
            return basic_string_literal<CharT, 2>{{'5', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.revrse)
        {
            return basic_string_literal<CharT, 2>{{'7', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.hidden)
        {
            return basic_string_literal<CharT, 2>{{'8', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.strike)
        {
            return basic_string_literal<CharT, 2>{{'9', '\0'}};
        }
        else if constexpr ((info.tag & tag) == info.foregr)
        {
            return get_foreground_attr<CharT, info>();
        }
        else if constexpr ((info.tag & tag) == info.backgr)
        {
            return get_background_attr<CharT, info>();
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    ///
    /// @brief Get the format attribute string escape sequence out of the style info
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The style info with its fields
    /// @return A compile time string with the format attribute escape sequence
    ///
    template <typename CharT, fmt_style<CharT> info>
    static consteval auto get_format_style_attr()
    {
        static_assert(
            is_same<CharT, wchar>::value || 
            is_same<CharT, char>::value, 
            "Unsupported character type"
        );

        constexpr auto _res = [&]<usize... Ints>(index_sequence<Ints...>)
        {
            constexpr auto _get_attr = [&]<usize I>()
            {
                constexpr auto _attr = get_style_attr_impl<CharT, 1 << I, info>();
    
                if constexpr (_attr.size() - 1 != 0)
                {
                    return (info.attr_sep + _attr);
                }
                else
                {
                    return basic_string_literal<CharT, 1>{{'\0'}};
                }
            };

            return ((_get_attr.template operator()<Ints>()) + ...);
        }(make_index_sequence<info.size>{});

        if constexpr (_res.size() - 1 != 0)
        {
            return (info.attr_base + _res + info.attr_end);
        }
        else
        {
            return basic_string_literal<CharT, 1>{{'\0'}};
        }
    }

    ///
    /// @brief Structure used as a namespace
    /// for the format traits
    ///
    struct formatter_traits
    {
        ///
        /// @brief Checks if T is a character type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_char = is_same<decay_t<T>, char>::value;

        ///
        /// @brief Checks if T is a 8-bit integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_i8 = is_same<decay_t<T>, i8>::value;

        ///
        /// @brief Checks if T is a 8-bit unsigned integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_u8 = is_same<decay_t<T>, u8>::value;

        ///
        /// @brief Checks if T is a 16-bit integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_i16 = is_same<decay_t<T>, i16>::value;

        ///
        /// @brief Checks if T is a 16-bit unsigned integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_u16 = is_same<decay_t<T>, u16>::value;

        ///
        /// @brief Checks if T is a 32-bit integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_i32 = is_same<decay_t<T>, i32>::value;

        ///
        /// @brief Checks if T is a 32-bit unsigned integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_u32 = is_same<decay_t<T>, u32>::value;

        ///
        /// @brief Checks if T is a 64-bit integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_i64 = is_same<decay_t<T>, i64>::value;

        ///
        /// @brief Checks if T is a 64-bit unsigned integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_u64 = is_same<decay_t<T>, u64>::value;

        ///
        /// @brief Checks if T is a 32/64-bit (depends on the OS) integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_ilong = is_same<decay_t<T>, ilong>::value;

        ///
        /// @brief Checks if T is a 32/64-bit (depends on the OS) unsigned integer type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_ulong = is_same<decay_t<T>, ulong>::value;

        ///
        /// @brief Checks if T is a 32-bit floating point type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_f32 = is_same<decay_t<T>, f32>::value;

        ///
        /// @brief Checks if T is a 64-bit floating point type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_f64 = is_same<decay_t<T>, f64>::value;

        ///
        /// @brief Checks if T is a +80-bit (depends on the CPU arch) floating point type
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_f128 = is_same<decay_t<T>, f128>::value;

        ///
        /// @brief Checks if T is convertible to a character pointer
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_char_ptr = requires(T t)
        {
            static_cast<const char*>(t);
        };

        ///
        /// @brief Checks if T is convertible to a wide character pointer
        /// 
        /// @tparam T - The type to check
        ///
        template <typename T>
        static constexpr bool is_wchar_ptr = requires(T t)
        {
            static_cast<const wchar*>(t);
        };
    };

    ///
    /// @brief Gets the coresponding `printf` format (char overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// in this case all possible options are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_char<T>)
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
            " for char"
        );

        if constexpr (is_same<CharT, char>::value)
        {
            return string_literal<3>{"%c"};
        }
        else if constexpr (is_same<CharT, wchar>::value)
        {
            return wstring_literal<3>{L"%c"};
        }
    }

    ///
    /// @brief Gets the coresponding `printf` format (i8 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_i8<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (u8 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_u8<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (i16 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_i16<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (u16 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_u16<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (i32 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_i32<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (u32 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_u32<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (i64 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_i64<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (u64 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_u64<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (ilong overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_ilong<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (ulong overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal format option is supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_ulong<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (f32 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal (0xh.hhhhp+/-d) and exponential (e.g. d.dddde+d)
    /// format options are supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_f32<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (f64 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal (0xh.hhhhp+/-d) and exponential (e.g. d.dddde+d)
    /// format options are supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_f64<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (f128 overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// hexadecimal (0xh.hhhhp+/-d) and exponential (e.g. d.dddde+d)
    /// format options are supported, the rest are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_f128<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (const char* overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// in this case all possible options are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_char_ptr<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (const wchar* overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// in this case all possible options are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
    template <typename CharT, fmt_common<CharT> info, typename T>
    requires (formatter_traits::is_wchar_ptr<T>)
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

    ///
    /// @brief Gets the coresponding `printf` format (string-view-like overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// in this case all possible options are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
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

    ///
    /// @brief Gets the coresponding `printf` format (wstring-view-like overload)
    /// 
    /// @tparam CharT - The character type (char or wchar)
    /// @tparam info - The format info with specific formatting options
    /// in this case all possible options are invalid
    /// @tparam T - The type to format
    /// @return The coresponding `printf` format string
    ///
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