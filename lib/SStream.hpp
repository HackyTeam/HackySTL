#pragma once

#include "_SStreamDetail.hpp"
#include "Tuple.hpp"

namespace hsd
{
    template <typename CharT>
    class basic_sstream : private vector<CharT>
    {
    private:
        static constexpr CharT _default_seps[] = {' ', '\t', '\n', '\r'};
        const CharT* _separators = _default_seps;

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        
        inline basic_sstream() = default;
        inline basic_sstream(const basic_sstream&) = delete;
        inline basic_sstream& operator=(const basic_sstream&) = delete;

        inline void write_raw_data(const CharT* raw_data)
        {
            for (auto* it = raw_data; *it != '\0'; ++it)
            {
                this->back() = *it;
                this->emplace_back('\0');
            }
        }

        template <typename... Args>
        inline Result<void, runtime_error> set_data(Args&... args)
        {
            auto _data_set = sstream_detail::
                split_data<sizeof...(Args)>(data(), _separators);

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
            tuple _args_tup = {forward<Args&>(args)...};

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
                            .base = _fmt_buf[I].base
                        };

                    sstream_detail::stream_writer<_curr_fmt> _writer = {
                        data(), capacity() - size(), size()
                    };

                    _write_impl(_writer, forward<arg_type>(_arg)).unwrap();
                    this->_size += _writer.index();
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
                    .base = _last.base
                };

            sstream_detail::stream_writer<_curr_fmt> _writer = {
                data(), capacity() - size(), size()
            };

            _write_impl(_writer).unwrap();
            this->_size += _writer.index();
        }

        inline void pop_back()
        {
            vector<CharT>::pop_back();
            this->back() = '\0';
        }

        inline void push_back(CharT c)
        {
            if (this->size() != 0)
            {
                this->back() = c;
                this->emplace_back('\0');
            }
            else
            {
                this->emplace_back(c);
                this->emplace_back('\0');
            }
        }

        inline void set_separators(const CharT* separators)
        {
            _separators = separators;
        }

        inline const CharT* get_separators() const
        {
            return _separators;
        }

        inline void clear()
        {
            vector<CharT>::clear();
            this->emplace_back('\0');
        }

        using vector<CharT>::resize;
        using vector<CharT>::reserve;
        using vector<CharT>::capacity;
        using vector<CharT>::size;
        using vector<CharT>::data;
        using vector<CharT>::begin;
        using vector<CharT>::end;
        using vector<CharT>::cbegin;
        using vector<CharT>::cend;

        inline const_iterator c_str() const
        {
            return this->begin();
        }
    };
    
    using sstream = basic_sstream<char>;
    using wsstream = basic_sstream<wchar>;
    using u8sstream = basic_sstream<char8>;
    using u16sstream = basic_sstream<char16>;
    using u32sstream = basic_sstream<char32>;
} // namespace hsd
