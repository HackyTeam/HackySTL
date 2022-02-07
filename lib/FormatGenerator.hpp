#pragma once

#include "_FormatGeneratorDetail.hpp"

namespace hsd
{
    template <format_literal lit, typename T>
    static constexpr auto format()
    {
        if constexpr (lit.base.tag & lit.base.ovr)
        {
            static_assert(
                !(lit.base.tag & lit.base.style),
                "Format literal cannot have both"
                " ovr and style tags"
            );

            static_assert(
                requires (T _t) {{_t.pretty_args()} -> IsTuple;},
                "Object must have a pretty_args() method which returns a tuple."    
            );

            using arg_type = remove_cvref_t<T>;
            using type_tup = type_tuple<decltype(declval<T>().pretty_args())>;
            using char_type = typename decltype(lit)::char_type;

            constexpr usize _ovr_arg_size = type_tup::size;            
            constexpr auto _ovr_fmt_buf = parse_literal<
                    arg_type::template pretty_fmt<char_type>(), 
                    _ovr_arg_size + 1
                >().unwrap();

            constexpr auto _ovr_last = 
                basic_string_literal<
                    char_type, _ovr_fmt_buf[_ovr_arg_size].length + 1
                > {
                    _ovr_fmt_buf[_ovr_arg_size].format, 
                    _ovr_fmt_buf[_ovr_arg_size].length
                };

            constexpr auto _sz = basic_cstring<char_type>::length(_ovr_last.data);
            static_assert(_ovr_last.size() == _sz + 1);

            return lit.format + [&]<usize... I>(index_sequence<I...>) {
                return (
                    format<
                        format_literal<char_type, _ovr_fmt_buf[I].length + 1>
                        {
                            .format = {_ovr_fmt_buf[I].format, _ovr_fmt_buf[I].length},
                            .base = _ovr_fmt_buf[I].base
                        },
                        decay_t<typename type_tup::template type_at<I>>
                    >() + ...
                );
            }(make_index_sequence<_ovr_arg_size>{}) + _ovr_last;
        }
        else
        {
            if constexpr (requires(T _t) {_t.pretty_args();})
            {
                using tup_val_type = decltype(declval<T>().pretty_args());
                using char_type = typename decltype(lit)::char_type;
                using cstring_type = basic_cstring<char_type>;

                constexpr auto _fmt_style_attr = get_format_style_attr<char_type, lit.base.style_val>();
                constexpr auto _fmt_val_str = get_value_format_string<char_type, lit.base, tup_val_type>();
                constexpr auto _fmt_reset_attr = get_reset_attr<char_type, lit.base.style_val>();
                
                static_assert(_fmt_style_attr.size() - 1 == cstring_type::length(_fmt_style_attr.data));
                static_assert(_fmt_reset_attr.size() - 1 == cstring_type::length(_fmt_reset_attr.data));
                static_assert(_fmt_val_str.size() - 1 == cstring_type::length(_fmt_val_str.data));

                return lit.format + _fmt_style_attr + _fmt_val_str + _fmt_reset_attr;
            }
            else
            {
                using char_type = typename decltype(lit)::char_type;
                using cstring_type = basic_cstring<char_type>;

                constexpr auto _fmt_style_attr = get_format_style_attr<char_type, lit.base.style_val>();
                constexpr auto _fmt_val_str = get_value_format_string<char_type, lit.base, decay_t<T>>();
                constexpr auto _fmt_reset_attr = get_reset_attr<char_type, lit.base.style_val>();
                
                static_assert(_fmt_style_attr.size() - 1 == cstring_type::length(_fmt_style_attr.data));
                static_assert(_fmt_reset_attr.size() - 1 == cstring_type::length(_fmt_reset_attr.data));
                static_assert(_fmt_val_str.size() - 1 == cstring_type::length(_fmt_val_str.data));

                return lit.format + _fmt_style_attr + _fmt_val_str + _fmt_reset_attr;
            }
        }
    }

    template <typename T>
    static constexpr auto make_args(T&& arg)
    {
        if constexpr (requires{{arg.pretty_args()} -> IsTuple;})
        {
            return arg.pretty_args();
        }
        else
        {
            return tuple<T>{arg};
        }
    }
} // namespace hsd
