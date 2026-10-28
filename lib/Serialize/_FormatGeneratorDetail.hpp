#pragma once

/// @file _FormatGeneratorDetail.hpp
/// @brief Internal compile-time format style and escape generation.
/// @details Converts formatting metadata into character sequences consumed by
/// the public format generator and printer implementation.

#include "LiteralParser.hpp"

///
/// @brief Used in identifying the classes
/// and functions in the standard library. 
///
namespace hsd
{
    ///
    /// @brief Get the foreground string value out of the style info.
    /// 
    /// @tparam CharT - The character type (char or wchar).
    /// @tparam info - The style info with its fields.
    /// @return A compile time string with the foreground color escape sequence.
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
    /// @brief Get the background string value out of the style info.
    /// 
    /// @tparam CharT - The character type (char or wchar).
    /// @tparam info - The style info with its fields.
    /// @return A compile time string with the background color escape sequence.
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
    /// of the style info if there is any.
    /// 
    /// @tparam CharT - The character type (char or wchar).
    /// @tparam info - The style info with its fields.
    /// @return A compile time string with the reset escape sequence.
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
    /// @brief Get the style string value out of the style info.
    /// 
    /// @tparam CharT - The character type (char or wchar).
    /// @tparam tag - mask of the style.
    /// @tparam info - The style info with its fields.
    /// @return A compile time string with the style value.
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
    /// @brief Get the format attribute string escape sequence out of the style info.
    /// 
    /// @tparam CharT - The character type (char or wchar).
    /// @tparam info - The style info with its fields.
    /// @return A compile time string with the format attribute escape sequence.
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
} // namespace hsd