#pragma once

#include "_SStreamDetail.hpp"
#include "Tuple.hpp"

namespace hsd
{
    template <typename CharT, usize Size>
    class basic_sstream
    {
    private:
        CharT _data[Size] = {};
        usize _size = 0;

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        
        constexpr basic_sstream() = default;
        constexpr basic_sstream(const basic_sstream&) = delete;
        constexpr basic_sstream& operator=(const basic_sstream&) = delete;

        constexpr void write_raw_data(const CharT* raw_data)
        {
            for (auto* it = raw_data; *it != '\0'; ++it)
            {
                _data[_size++] = *it;

                if (_size == Size)
                {
                    hsd_panic("static_basic_sstream: buffer overflow");
                }
            }
        }

        template <typename... Args>
        inline Result<void, runtime_error> set_data(Args&... args)
        {
            auto _data_set = sstream_detail::split_data<sizeof...(Args)>(_data);

            if (sizeof...(Args) > _data_set.size())
            {
                return runtime_error{"Input too small to parse"};
            }
            else
            {
                sstream_detail::stream_parser<CharT> _parser = 
                    pair{_data_set.data(), _data_set.size()};
                
                ((_parse_impl(_parser, args).unwrap()), ...);
            }

            return {};
        }

        template < basic_string_literal fmt, typename... Args >
        inline void write_data(Args&&... args)
        {
            using char_type = typename decltype(fmt)::char_type;
            const tuple<Args&...> _args_tup = {args...};
            constexpr auto _fmt_buf = sstream_detail::
                parse_literal<fmt, sizeof...(Args) + 1>().unwrap();
            
            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "Arguments don\'t match"
            );

            if constexpr (sizeof...(Args) != 0)
            {
                auto _forward_write = [&]<usize I>()
                {
                    using arg_type = decltype(_args_tup.template get<I>());
                    arg_type _arg = _args_tup.template get<I>();

                    constexpr auto _curr_fmt = 
                        format_literal<char_type, _fmt_buf[I].length + 1>
                        {
                            .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                            .tag = _fmt_buf[I].tag,
                            .foreground = _fmt_buf[I].foreground,
                            .background = _fmt_buf[I].background
                        };

                    sstream_detail::stream_writer<_curr_fmt> _writer = {
                        _data, Size - _size, _size
                    };

                    _write_impl(_writer, forward<arg_type>(_arg));
                    _size += _writer.index();
                };

                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    ((_forward_write.template operator()<Ints>()), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }

            constexpr auto _last = _fmt_buf[sizeof...(Args)];
            constexpr auto _curr_fmt = 
                format_literal<char_type, _last.length + 1>
                {
                    .format = {_last.format, _last.length},
                    .tag = _last.tag,
                    .foreground = _last.foreground,
                    .background = _last.background
                };

            sstream_detail::stream_writer<_curr_fmt> _writer = {
                _data, Size - _size, _size
            };

            _write_impl(_writer).unwrap();
            _size += _writer.index();
        }

        constexpr void pop_back()
        {
            _data[--_size] = '\0';
        }

        constexpr void push_back(CharT c)
        {
            _data[_size++] = c;
        }

        constexpr void clear()
        {
            _size = 0;
            _data[0] = '\0';
        }

        constexpr usize capacity() const
        {
            return Size;
        }

        constexpr usize size() const
        {
            return _size;
        }

        constexpr iterator data()
        {
            return _data;
        }

        constexpr const_iterator c_str() const
        {
            return _data;
        }
    };
    
    template <usize Size>
    using sstream = basic_sstream<char, Size>;
    template <usize Size>
    using wsstream = basic_sstream<wchar, Size>;
    template <usize Size>
    using u8sstream = basic_sstream<char8, Size>;
    template <usize Size>
    using u16sstream = basic_sstream<char16, Size>;
    template <usize Size>
    using u32sstream = basic_sstream<char32, Size>;
} // namespace hsd
