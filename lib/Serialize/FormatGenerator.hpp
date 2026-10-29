#pragma once

/// @file FormatGenerator.hpp
/// @brief Compile-time format literal and formatting-tag definitions.
/// @details Represents format text, styles, numeric presentation, and output
/// character types used by SStream and the project printer.

#include "_FormatGeneratorDetail.hpp"

namespace hsd
{
    ///
    /// @brief Resolved format text, style escapes, and numeric tag for a value.
    ///
    /// @tparam CharT - Character type of the stored literals.
    /// @tparam N1 - Stored element count of `format`.
    /// @tparam N2 - Stored element count of `style`.
    /// @tparam N3 - Stored element count of `reset`.
    ///
    template <typename CharT, usize N1, usize N2, usize N3>
    struct format_output
    {
        /// @brief Alias for a `basic_string_literal` of `CharT` and size `N`.
        template <usize N>
        using lit_type = basic_string_literal<CharT, N>;

        /// @brief User-supplied literal text surrounding the formatted value.
        lit_type<N1> format;
        /// @brief Style escape sequence applied before the value.
        lit_type<N2> style;
        /// @brief Reset escape sequence applied after the value.
        lit_type<N3> reset;

        /// @brief Tag bit meaning no special numeric presentation is requested.
        static constexpr usize none  = 0; 
        /// @brief Tag bit requesting hexadecimal presentation.
        static constexpr usize hex   = 1;  // hexadecimal
        /// @brief Tag bit requesting exponential presentation.
        static constexpr usize exp   = 2;  // exponent
        /// @brief Tag bit requesting an explicit precision value.
        static constexpr usize prec  = 16; // precision

        /// @brief Combination of presentation tag bits selected for this value.
        usize tag;
        /// @brief Precision value used when the `prec` tag bit is set.
        usize prec_val;
    };

    ///
    /// @brief Builds the resolved `format_output` for a styled literal and value type.
    ///
    /// @tparam lit - Parsed format literal describing style, color, and tags.
    /// @tparam T - Value type the format is being resolved for.
    /// @return A `format_output` holding the literal text, style/reset escapes,
    /// and the numeric presentation tag extracted from `lit`.
    ///
    template <format_literal lit, typename T>
    requires (!(lit.base.tag & lit.base.ovr))
    static constexpr auto format()
    {
        using char_type = typename decltype(lit)::char_type;
        using cstring_type = basic_cstring<char_type>;

        constexpr auto _fmt_style_attr = get_format_style_attr<char_type, lit.base.style_val>();
        constexpr auto _fmt_reset_attr = get_reset_attr<char_type, lit.base.style_val>();

        static_assert(_fmt_style_attr.size() - 1 == cstring_type::length(_fmt_style_attr.data));
        static_assert(_fmt_reset_attr.size() - 1 == cstring_type::length(_fmt_reset_attr.data));

        return format_output<char_type, lit.format.size(), _fmt_style_attr.size(), _fmt_reset_attr.size()>{
            .format = lit.format,
            .style = _fmt_style_attr,
            .reset = _fmt_reset_attr,
            .tag = lit.base.tag & ~lit.base.style,
            .prec_val = lit.base.prec_val
        };
    }

    ///
    /// @brief Overload of `format()` for literals with no associated value type.
    ///
    /// @tparam lit - Parsed format literal describing style, color, and tags.
    /// @return A `format_output` holding the literal text, style/reset escapes,
    /// and the numeric presentation tag extracted from `lit`.
    ///
    template <format_literal lit>
    requires (!(lit.base.tag & lit.base.ovr))
    static constexpr auto format()
    {
        using char_type = typename decltype(lit)::char_type;
        using cstring_type = basic_cstring<char_type>;

        constexpr auto _fmt_style_attr = get_format_style_attr<char_type, lit.base.style_val>();
        constexpr auto _fmt_reset_attr = get_reset_attr<char_type, lit.base.style_val>();

        static_assert(_fmt_style_attr.size() - 1 == cstring_type::length(_fmt_style_attr.data));
        static_assert(_fmt_reset_attr.size() - 1 == cstring_type::length(_fmt_reset_attr.data));

        return format_output<char_type, lit.format.size(), _fmt_style_attr.size(), _fmt_reset_attr.size()>{
            .format = lit.format,
            .style = _fmt_style_attr,
            .reset = _fmt_reset_attr,
            .tag = lit.base.tag & ~lit.base.style,
            .prec_val = lit.base.prec_val
        };
    }
} // namespace hsd
