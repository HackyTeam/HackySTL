#pragma once

/// @file LiteralParser.hpp
/// @brief Compile-time parsing of format string literals.
/// @details Splits literal text into format segments, validates tags, and
/// supplies the metadata consumed by FormatGenerator and SStream.

#include "StringLiteral.hpp"
#include "../Containers/Vector.hpp"
#include "../Base/CString.hpp"
#include "../Base/Switch.hpp"

namespace hsd
{
    /// @brief Per-tag ANSI SGR (Select Graphic Rendition) style state accumulated
    /// while parsing `bold`, `fg`, `bg`, ... style tokens inside a `{...}` tag.
    /// @tparam CharT Character type of the literal being parsed (char or wchar).
    template <typename CharT>
    struct fmt_style
    {
        /// @brief Fixed-capacity compile-time string alias used for the escape
        /// sequence fragments below.
        /// @tparam N Capacity of the string literal, including the terminating null.
        template <usize N>
        using lit_type = basic_string_literal<CharT, N>;

        /// @brief Escape prefix (`ESC [`) that starts every SGR attribute sequence.
        static constexpr lit_type<3> attr_base = {
            {'\x1b', '[', '\0'}
        };

        /// @brief Separator (`;`) placed between chained SGR attribute codes.
        static constexpr lit_type<2> attr_sep = {
            {';', '\0'}
        };

        /// @brief Terminator (`m`) that closes an SGR attribute sequence.
        static constexpr lit_type<2> attr_end = {
            {'m', '\0'}
        };

        /// @brief Prefix (`38;5;`) selecting an indexed 256-color foreground.
        static constexpr lit_type<6> fg_base = {
            {'3', '8', ';', '5', ';', '\0'}
        };

        /// @brief Prefix (`48;5;`) selecting an indexed 256-color background.
        static constexpr lit_type<6> bg_base = {
            {'4', '8', ';', '5', ';', '\0'}
        };

        /// @brief Full escape sequence (`ESC [0m`) that resets all SGR attributes.
        static constexpr lit_type<5> reset_attr = {
            {'\x1b', '[', '0', 'm', '\0'}
        };

        /// @brief Decimal text of an indexed color code (0-255) parsed from a
        /// `fg=` or `bg=` token.
        struct fmt_color
        {
            /// @brief Number of significant digits stored in #value (1 to 3).
            usize length = 0;
            /// @brief Digit characters of the color code, most significant first.
            CharT value[4] = {};
        };

        static constexpr usize bold   = 1;   ///< `bold` style tag bit.
        static constexpr usize dim    = 2;   ///< `dim` style tag bit.
        static constexpr usize italic = 4;   ///< `italic` style tag bit.
        static constexpr usize undrln = 8;   ///< `undrln` (underline) style tag bit.
        static constexpr usize blink  = 16;  ///< `blink` style tag bit.
        static constexpr usize revrse = 32;  ///< `rev` (reverse video) style tag bit.
        static constexpr usize hidden = 64;  ///< `hid` (hidden) style tag bit.
        static constexpr usize strike = 128; ///< `strike` (strikethrough) style tag bit.
        static constexpr usize foregr = 256; ///< `fg` (foreground color) style tag bit.
        static constexpr usize backgr = 512; ///< `bg` (background color) style tag bit.
        static constexpr usize size   = 10;  ///< Number of distinct style tag bits declared above.

        /// @brief Bitwise OR of the style tag bits (#bold, #foregr, ...) enabled
        /// for this format token.
        usize tag = 0;
        /// @brief Parsed foreground color; meaningful only when #tag has #foregr set.
        fmt_color foreground = {};
        /// @brief Parsed background color; meaningful only when #tag has #backgr set.
        fmt_color background = {};
    };

    /// @brief Per-token category flags and payload shared by every `{...}` format
    /// tag, independent of character type.
    /// @tparam CharT Character type of the literal being parsed (char or wchar).
    template <typename CharT>
    struct fmt_common
    {
        static constexpr usize none  = 0;  ///< No tags set; the token has no modifiers.
        static constexpr usize hex   = 1;  ///< `hex` tag: print the value in hexadecimal.
        static constexpr usize exp   = 2;  ///< `exp` tag: print the value in exponential notation.
        static constexpr usize ovr   = 4;  ///< `ovr` tag: defer to the value's own pretty-printing.
        static constexpr usize style = 8;  ///< Set when any ANSI style tag (`bold`, `fg`, ...) is present.
        static constexpr usize prec  = 16; ///< `prec` tag: a precision value was supplied.

        /// @brief Bitwise OR of the #none/#hex/#exp/#ovr/#style/#prec tags present
        /// on this token.
        usize tag;
        /// @brief Precision value parsed from a `prec=` token; meaningful only
        /// when #tag has #prec set.
        usize prec_val;
        /// @brief ANSI style state parsed from `bold`/`fg`/`bg`/... tokens;
        /// meaningful only when #tag has #style set.
        fmt_style<CharT> style_val;
    };

    /// @brief Runtime view over one parsed segment of a format literal: the
    /// plain text preceding a tag plus the tag's parsed metadata.
    /// @tparam CharT Character type of the literal being parsed (char or wchar).
    template <typename CharT>
    struct format_info
    {
        /// @brief Pointer into the original literal where this segment's plain
        /// text begins.
        const CharT* format;
        /// @brief Number of characters of plain text starting at #format.
        usize length;

        /// @brief Tags and values parsed from the `{...}` token following the
        /// plain text (zero-initialized for the trailing segment).
        fmt_common<CharT> base;
    };

    /// @brief Compile-time, self-owning counterpart of #format_info: holds its
    /// plain text in a fixed-size #basic_string_literal instead of pointing
    /// into the original literal.
    /// @tparam CharT Character type of the literal.
    /// @tparam N Capacity of the owned string literal, including the
    /// terminating null.
    template <typename CharT, usize N>
    struct format_literal
    {
        /// @brief Character type of the literal, exposed for template deduction.
        using char_type = CharT;
        /// @brief Owned copy of this segment's plain text.
        basic_string_literal<CharT, N> format;

        /// @brief Tags and values parsed from the segment's `{...}` token.
        fmt_common<CharT> base;
    };

    /// @brief Implementation details of #parse_literal: the tag-name packing,
    /// scanning, and per-tag value parsing used while walking a format literal.
    namespace parse_lit_detail
    {
        /// @brief Packed representation of a tag name, used to compare tokens
        /// (`hex`, `fg`, `bold`, ...) against `switch_cases` without per-character
        /// comparisons. Characters at even source indices are packed into
        /// value[0] and characters at odd indices into value[1], three
        /// characters per word, so tag names up to six characters are supported.
        struct switchable
        {
            /// @brief Packed characters of the tag name; see #switchable for the
            /// packing scheme.
            u64 value[2] = {};

            /// @brief Compares two packed tag representations for equality.
            /// @param other Other packed value to compare against.
            /// @return true if both packed words are equal.
            constexpr bool operator==(const switchable& other) const
            {
                return value[0] == other.value[0] && value[1] == other.value[1];
            }
        };

        /// @brief Aborts compilation with a diagnostic if @p duplicate is
        /// already set in @p tag, rejecting a tag applied twice to one segment.
        /// @param tag Accumulated tag bits parsed so far for the current segment.
        /// @param duplicate Bit of the tag about to be applied.
        static constexpr void _check_duplicate_tag(
            usize tag, usize duplicate);

        /// @brief Finds the first occurrence of @p letter in a null-terminated
        /// string.
        /// @tparam CharT Character type of the string being scanned.
        /// @param str Null-terminated string to search.
        /// @param letter Character to search for.
        /// @return Index of the first match, or `static_cast<usize>(-1)` if
        /// @p letter does not occur before the terminating null.
        template <typename CharT>
        static constexpr auto _find(const CharT* str, CharT letter);

        /// @brief Packs the first @p sz characters of @p str into a #switchable
        /// so a tag name can be compared with `switch_cases`.
        /// @tparam CharT Character type of the token text.
        /// @param str Token text (need not be null-terminated).
        /// @param sz Number of characters of @p str to pack.
        /// @return Packed representation of the token.
        template <typename CharT>
        static constexpr auto _to_switchable(const CharT* str, usize sz)
            -> switchable;

        /// @brief Packs a compile-time string literal token into a #switchable.
        /// @tparam CharT Character type of the token text.
        /// @tparam N Length of @p str, including the terminating null.
        /// @param str Null-terminated string literal to pack.
        /// @return Packed representation of the token.
        template <typename CharT, usize N>
        static constexpr auto _to_switchable(const CharT (&str)[N])
            -> switchable;

        /// @brief Ensures a parsed tag is immediately followed by the tag
        /// separator `,` or the closing `}`, aborting compilation otherwise.
        /// @tparam CharT Character type of the format literal.
        /// @param fmt Format literal being parsed.
        /// @param pos_current Index just past the tag that was parsed.
        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT* fmt, usize pos_current
        );

        /// @brief Parses the `=NNN` payload of an `fg=`/`bg=` tag into an
        /// indexed color code.
        /// @tparam CharT Character type of the format literal.
        /// @param color_fmt Format literal being parsed.
        /// @param pos_current In/out index positioned at the `=` on entry;
        /// advanced past the parsed digits on success.
        /// @return The parsed color on success, or a `runtime_error` describing
        /// why the color could not be parsed (missing `=` or missing digits).
        template <typename CharT>
        static constexpr auto _parse_color(
            const CharT* color_fmt, usize& pos_current
        ) -> result<typename fmt_style<CharT>::fmt_color, runtime_error>;

        /// @brief Parses the `=NNN` payload of a `prec=` tag into a precision
        /// value.
        /// @tparam CharT Character type of the format literal.
        /// @param prec_fmt Format literal being parsed.
        /// @param pos_current In/out index positioned at the `=` on entry;
        /// advanced past the parsed digits on success.
        /// @return The parsed precision on success, or a `runtime_error`
        /// describing why the precision could not be parsed (missing `=` or
        /// missing digits).
        template <typename CharT>
        static constexpr auto _parse_precision(
            const CharT* prec_fmt, usize& pos_current
        ) -> result<usize, runtime_error>;

        template <typename CharT>
        static constexpr void _add_hex_attr(const CharT* fmt_ptr, usize pos, usize& tag)
        {
            using tag_type = fmt_common<CharT>;

            if ((tag & tag_type::exp) == tag_type::exp)
            {
                hsd::panic("Hexadecimal and exponential formats are not allowed to mix with eachother");
            }
            parse_lit_detail::
                _check_duplicate_tag(tag, tag_type::hex);
            
            tag |= tag_type::hex;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_exp_attr(
            const CharT* fmt_ptr, usize pos, usize& tag)
        {
            using tag_type = fmt_common<CharT>;

            if ((tag & tag_type::hex) == tag_type::hex)
            {
                hsd::panic("Hexadecimal and exponential formats are not allowed to mix with eachother");
            }
            parse_lit_detail::
                _check_duplicate_tag(tag, tag_type::exp);
            
            tag |= tag_type::exp;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_ovr_attr(
            const CharT* fmt_ptr, usize pos, usize& tag)
        {
            using tag_type = fmt_common<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(tag, tag_type::ovr);

            tag |= tag_type::ovr;

            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_prec_attr(
            const CharT* fmt_ptr, usize& pos, 
            usize& prec, usize& tag)
        {
            using tag_type = fmt_common<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(tag, tag_type::prec);
            
            tag |= tag_type::prec;
            
            auto _res = parse_lit_detail::
                _parse_precision(fmt_ptr, pos);

            if (_res.is_ok() == false)
            {
                hsd::panic("Failed to parse precision value.");
            }

            prec = _res.unwrap();
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_foreground_attr(
            const CharT* fmt_ptr, usize& pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::foregr);
            
            tag |= tag_type::style;
            style.tag |= style_type::foregr;
            
            auto _res = parse_lit_detail::
                _parse_color(fmt_ptr, pos);

            if (_res.is_ok() == false)
            {
                hsd::panic("Failed to parse foreground value, only valid 8-bit values are allowed.");
            }

            style.foreground = _res.unwrap();
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_background_attr(
            const CharT* fmt_ptr, usize& pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::backgr);
            
            tag |= tag_type::style;
            style.tag |= style_type::backgr;
            
            auto _res = parse_lit_detail::
                _parse_color(fmt_ptr, pos);

            if (_res.is_ok() == false)
            {
                hsd::panic("Failed to parse background value, only valid 8-bit values are allowed.");
            }
            
            style.background = _res.unwrap();

            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_bold_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::bold);
            
            tag |= tag_type::style;
            style.tag |= style_type::bold;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_dim_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::dim);
            
            tag |= tag_type::style;
            style.tag |= style_type::dim;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_itlaic_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::italic);
            
            tag |= tag_type::style;
            style.tag |= style_type::italic;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_underline_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::undrln);
            
            tag |= tag_type::style;
            style.tag |= style_type::undrln;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_blink_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::blink);
            
            tag |= tag_type::style;
            style.tag |= style_type::blink;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_reverse_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::revrse);
            
            tag |= tag_type::style;
            style.tag |= style_type::revrse;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_hidden_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::hidden);
            
            tag |= tag_type::style;
            style.tag |= style_type::hidden;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }

        template <typename CharT>
        static constexpr void _add_strikethrough_attr(
            const CharT* fmt_ptr, usize pos, 
            fmt_style<CharT>& style, usize& tag)
        {
            using tag_type = fmt_common<CharT>;
            using style_type = fmt_style<CharT>;

            parse_lit_detail::
                _check_duplicate_tag(style.tag, style_type::strike);
            
            tag |= tag_type::style;
            style.tag |= style_type::strike;
            
            parse_lit_detail::
                _check_following_fmt(fmt_ptr, pos);
        }
    } // namespace parse_lit_detail

    /// @brief Parses a compile-time format string literal into per-segment
    /// metadata consumed by FormatGenerator and SStream.
    ///
    /// @details
    /// Grammar (informal): the literal is a sequence of plain-text segments
    /// interleaved with tags of the form `{tag1,tag2=value,...}`; an empty
    /// `{}` denotes a segment with no tags. Recognized tag names are:
    ///  - `hex`, `exp`, `ovr`: flag tags, no value.
    ///  - `prec=N`: sets fmt_common::prec_val to the decimal integer `N`.
    ///  - `fg=N`, `bg=N`: set the foreground/background color to the indexed
    ///    color `N` (0-255).
    ///  - `bold`, `dim`, `italic`, `undrln`, `blink`, `rev`, `hid`, `strike`:
    ///    ANSI style flag tags, no value.
    ///
    /// Each tag must be immediately followed by `,` (to chain another tag) or
    /// `}` (to close the token); applying the same tag twice within one token,
    /// an empty token right after `,`, or an unrecognized tag name each abort
    /// compilation (via `hsd::panic` or `.unwrap()` on an intermediate error
    /// result). One #format_info entry is emitted per plain-text segment,
    /// including a final entry for the text following the last tag.
    ///
    /// @tparam fmt Compile-time string literal to parse.
    /// @tparam N Maximum number of segments the returned buffer can hold; must
    /// be at least the number of tags in @p fmt plus one.
    /// @return On success, a `result` holding a `static_vector` of #format_info
    /// describing each segment; on failure (an empty token immediately after
    /// `,`), a `result` holding a descriptive `runtime_error`.
    template <basic_string_literal fmt, usize N>
    static constexpr auto parse_literal()
    {
        using char_type = typename decltype(fmt)::char_type;
        using cstring_type = basic_cstring<char_type>;
        using info_type = format_info<char_type>;
        using buf_type = static_vector<info_type, N>;
        using res_type = result<buf_type, runtime_error>;
        using style_type = fmt_style<char_type>;

        constexpr const char_type *_fmt_end = fmt.data + fmt.size() - 1;
        constexpr auto _npos = static_cast<usize>(-1);

        buf_type _buf;
        usize _tag = 0;
        usize _prec = 0;

        style_type _style = {};
        const char_type* _fmt_ptr = fmt.data;

        usize _pos_current = parse_lit_detail::
            _find<char_type>(_fmt_ptr, '{');
        
        usize _curly_pos = _pos_current;

        auto _add_hex_func       = [&] { parse_lit_detail::_add_hex_attr(_fmt_ptr, _pos_current, _tag); };
        auto _add_exp_func       = [&] { parse_lit_detail::_add_exp_attr(_fmt_ptr, _pos_current, _tag); };
        auto _add_ovr_func       = [&] { parse_lit_detail::_add_ovr_attr(_fmt_ptr, _pos_current, _tag); };
        auto _add_prec_func      = [&] { parse_lit_detail::_add_prec_attr(_fmt_ptr, _pos_current, _prec, _tag); };
        auto _add_fg_func        = [&] { parse_lit_detail::_add_foreground_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_bg_func        = [&] { parse_lit_detail::_add_background_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_bold_func      = [&] { parse_lit_detail::_add_bold_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_dim_func       = [&] { parse_lit_detail::_add_dim_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_itlaic_func    = [&] { parse_lit_detail::_add_itlaic_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_underline_func = [&] { parse_lit_detail::_add_underline_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_blink_func     = [&] { parse_lit_detail::_add_blink_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_rev_func       = [&] { parse_lit_detail::_add_reverse_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_hid_func       = [&] { parse_lit_detail::_add_hidden_attr(_fmt_ptr, _pos_current, _style, _tag); };
        auto _add_strike_func    = [&] { parse_lit_detail::_add_strikethrough_attr(_fmt_ptr, _pos_current, _style, _tag); };

        while (_pos_current != _npos && _fmt_ptr[_pos_current] != '\0')
        {
            if (_pos_current != _npos && _fmt_ptr[_pos_current] != '}')
            {
                if (_fmt_ptr[_pos_current] == ',' && _fmt_ptr[_pos_current + 1] == '}')
                {
                    return res_type {
                        runtime_error {
                            "Empty token after ','"
                        }
                    };
                }

                _pos_current++;
                usize _pos_prev = _pos_current;

                while (cstring_type::is_letter(_fmt_ptr[_pos_current]))
                {
                    _pos_current++;
                }

                if (_pos_current == _pos_prev && _fmt_ptr[_pos_current] != '}')
                {
                    return res_type {
                        runtime_error {
                            "Invalid token"
                        }
                    };
                }

                auto _switch_token = parse_lit_detail::_to_switchable(
                    _fmt_ptr + _pos_prev, _pos_current - _pos_prev);

                switch_cases(
                    _switch_token, equal_case {
                        parse_lit_detail::_to_switchable(""), []{}
                    }, equal_case {
                        parse_lit_detail::_to_switchable("hex"), _add_hex_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("exp"), _add_exp_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("ovr"), _add_ovr_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("prec"), _add_prec_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("fg"), _add_fg_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("bg"), _add_bg_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("bold"), _add_bold_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("dim"), _add_dim_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("italic"), _add_itlaic_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("undrln"), _add_underline_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("blink"), _add_blink_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("rev"), _add_rev_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("hid"), _add_hid_func
                    }, equal_case {
                        parse_lit_detail::_to_switchable("strike"), _add_strike_func
                    }, default_case {[]{ hsd::panic("Invalid format token"); }}
                );
            }
            else
            {
                _buf.emplace_back(
                    info_type {
                        .format = _fmt_ptr,
                        .length = _curly_pos,
                        .base = {
                            .tag = _tag,
                            .prec_val = _prec,
                            .style_val = _style,
                        }
                    }
                );

                _fmt_ptr += _pos_current + 1;
                _pos_current = _curly_pos = parse_lit_detail::
                    _find<char_type>(_fmt_ptr, '{');
                
                _tag = 0;
                _style = style_type{};
                _prec = 0;
            }
        }

        _buf.emplace_back(
            info_type {
                .format = _fmt_ptr,
                .length = static_cast<usize>(_fmt_end - _fmt_ptr),
                .base = {
                    .tag = _tag,
                    .prec_val = _prec,
                    .style_val = _style,
                }
            }
        );

        return res_type{_buf};
    }

    namespace parse_lit_detail
    {
        static constexpr void _check_duplicate_tag(
            usize tag, usize duplicate)
        {
            if (tag & duplicate) hsd::panic("Duplicate tag");
        };

        template <typename CharT>
        static constexpr auto _find(const CharT *str, CharT letter)
        {
            const CharT *_start_ptr = str;

            for (; *str != '\0'; str++)
            {
                if (*str == letter)
                    return static_cast<usize>(str - _start_ptr);
            }

            return static_cast<usize>(-1);
        };

        template <typename CharT>
        static constexpr auto _to_switchable(const CharT *str, usize sz)
            -> switchable
        {
            switchable _result = {};

            for (usize _index = 0; _index < sz; _index++)
            {
                _result.value[_index % 2] |=
                    static_cast<u64>(str[_index]) << ((_index >> 1) * 8);
            }

            return _result;
        };

        template <typename CharT, usize N>
        static constexpr auto _to_switchable(const CharT (&str)[N])
            -> switchable
        {
            return _to_switchable(str, N - 1);
        };

        template <typename CharT>
        static constexpr void _check_following_fmt(
            const CharT *fmt, usize pos_current)
        {
            if (fmt[pos_current] != '}' && fmt[pos_current] != ',')
            {
                hsd::panic("Invalid format literal: must be followed by '}' or ','");
            }
        }

        template <typename CharT>
        static constexpr auto _parse_precision(
            const CharT* prec_fmt, usize& pos_current) 
            -> result<usize, runtime_error>
        {
            if (prec_fmt[pos_current++] != '=')
            {
                return runtime_error{"Expected '=' after precision format"};
            }

            constexpr auto _is_number = [](CharT letter)
            {
                return letter >= '0' && letter <= '9';
            };

            if (prec_fmt[pos_current] <= '9' && prec_fmt[pos_current] >= '0')
            {
                usize _num = 0;

                for (; prec_fmt[pos_current] != '\0' && _is_number(prec_fmt[pos_current]); ++pos_current)
                {
                    _num *= 10;
                    _num += prec_fmt[pos_current] - '0';
                }

                return _num;
            }
            else
            {
                return runtime_error{"Expected precision value after '='"};
            }
        }

        template <typename CharT>
        static constexpr auto _parse_color(
            const CharT *color_fmt, usize &pos_current)
            -> result<typename fmt_style<CharT>::fmt_color, runtime_error>
        {
            using color_type = typename fmt_style<CharT>::fmt_color;

            if (color_fmt[pos_current++] != '=')
            {
                return runtime_error{"Expected '=' after color format"};
            }

            if (color_fmt[pos_current] <= '9' && color_fmt[pos_current] >= '0')
            {
                auto _color = basic_cstring<CharT>::template 
                    parse<uchar>(color_fmt + pos_current);

                color_type _res = {
                    // Ternary operator always loves you :^)
                    .length = _color > 99 ? 3u : _color > 9 ? 2u : 1u,
                    .value = {}
                };

                for (usize _index = 0; _index < _res.length; _index++)
                {
                    _res.value[_res.length - _index - 1] = _color % 10 + '0';
                    _color /= 10;
                }

                pos_current += _res.length;

                return _res;
            }
            else
            {
                return runtime_error{"Expected color value after '='"};
            }
        }
    } // namespace parse_lit_detail
} // namespace hsd