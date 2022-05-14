#pragma once

#include "FormatGenerator.hpp"

namespace hsd
{
    template <basic_string_literal fmt, typename... Args>
    static inline option_err<runtime_error> print(Args &&...args)
    {
        using char_type = typename decltype(fmt)::char_type;
        using tup_type = type_tuple<Args...>;

        static constexpr auto _fmt_buf =
            parse_literal<fmt, sizeof...(Args) + 1>().unwrap();

        constexpr auto _last = _fmt_buf[tup_type::size];
        constexpr auto _last_fmt = 
            format_literal<char_type, _last.length + 1>
            {
                .format = {_last.format, _last.length},
                .base = _last.base
            };

        static_assert(
            is_same<char_type, char>::value ||
            is_same<char_type, wchar>::value,
            "char_type must be char or wchar."
        );

        if constexpr (sizeof...(Args) != 0)
        {
            constexpr auto _print_fmt =
                []<usize... Ints>(index_sequence<Ints...>) {
                    
                    constexpr auto _gen_fmt = []<usize I>()
                    {
                        constexpr auto _curr_fmt = 
                            format_literal<char_type, _fmt_buf[I].length + 1>
                            {
                                .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                                .base = _fmt_buf[I].base
                            };

                        return format<_curr_fmt, typename tup_type::template type_at<I>>();
                    };

                    return ((_gen_fmt.template operator()<Ints>()) + ...);
                }(make_index_sequence<tup_type::size>{}) + _last_fmt.format;

            const tuple _args = (make_args(args) + ...);

            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                if constexpr (is_same<char_type, char>::value)
                {
                    printf(_print_fmt.data, _args.template get<Ints>()...);
                }
                else
                {
                    wprintf(_print_fmt.data, _args.template get<Ints>()...);
                }
            }(make_index_sequence<decltype(_args)::size()>{});
        }
        else
        {
            if constexpr (is_same<char_type, char>::value)
            {
                fputs(_last_fmt.format.data, stdout);
            }
            else
            {
                fputws(_last_fmt.format.data, stdout);
            }
        }

        return {};
    }
} // namespace hsd
