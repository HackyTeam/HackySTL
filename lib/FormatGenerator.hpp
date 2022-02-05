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
                !(lit.base.tag & lit.base.fg) && 
                !(lit.base.tag & lit.base.bg),
                "Format literal cannot have both "
                "foreground and background color "
                "tags for overloading format"
            );

            static_assert(
                requires (T _t) {{_t.pretty_args()} -> IsTuple;},
                "Object must have a pretty_args() method which returns a tuple."    
            );

            using arg_type = remove_cvref_t<T>;
            using type_tup = type_tuple<decltype(declval<T>().pretty_args())>;
            using char_type = typename decltype(lit)::char_type;

            constexpr usize _ovr_arg_size = type_tup::size;            
            constexpr auto _ovr_fmt_buf = 
                parse_literal<arg_type::template pretty_fmt<char_type>(), _ovr_arg_size + 1>().unwrap();

            constexpr auto _ovr_last = 
                basic_string_literal<char_type, _ovr_fmt_buf[_ovr_arg_size].length + 1>
                {
                    _ovr_fmt_buf[_ovr_arg_size].format, 
                    _ovr_fmt_buf[_ovr_arg_size].length
                };

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

                return (
                    lit.format + 
                    get_background_format<char_type, lit.base>() +
                    get_foreground_format<char_type, lit.base>() +
                    get_value_format_string<char_type, lit.base, tup_val_type>() +
                    get_reset_format<char_type, lit.base>()
                );
            }
            else
            {
                using char_type = typename decltype(lit)::char_type;

                return (
                    lit.format + 
                    get_background_format<char_type, lit.base>() +
                    get_foreground_format<char_type, lit.base>() +
                    get_value_format_string<char_type, lit.base, decay_t<T>>() +
                    get_reset_format<char_type, lit.base>()
                );
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
