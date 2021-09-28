#pragma once

#include "_SStreamDetail.hpp"
#include "Tuple.hpp"

namespace hsd
{
    template <typename CharT>
    class basic_sstream
    {
    private:
        CharT* _data = nullptr;
        usize _capacity = 0;
        usize _size = 0;

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        basic_sstream(const basic_sstream&) = delete;
        basic_sstream& operator=(const basic_sstream&) = delete;

        inline basic_sstream(usize size)
        {
            _data = mallocator::allocate_multiple<
                CharT>(size + 1).unwrap();

            _data[size] = '\0';
            _capacity = size;
        }

        inline ~basic_sstream()
        {
            mallocator::deallocate(_data);
        }

        inline void add_raw_data(const CharT* raw_data)
        {
            _size += static_cast<usize>(
                sstream_detail::_write<"">(raw_data, {_data + _size, _capacity - _size})
            );
            _data[_size++] = static_cast<CharT>(' ');
            _data[_size] = static_cast<CharT>('\0');
        }

        inline Result<void, runtime_error> update_size()
        {
            if (_data == nullptr)
                return runtime_error{"Invalid data"};

            _size = basic_cstring<CharT>::length(_data);
            return {};
        }

        template <typename... Args>
        inline Result<void, runtime_error> set_data(Args&... args)
        {
            using sstream_detail::_parse;
            auto _data_set = sstream_detail::split_data<sizeof...(Args)>(_data);

            if (sizeof...(Args) > _data_set.size())
            {
                return runtime_error{"Input too small to parse"};
            }
            else
            {
                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (_parse(_data_set[Ints], args), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }

            return {};
        }

        template <typename T>
        inline T parse()
        {
            T _value{};
            pair _parse_data = {
                c_str(), basic_cstring<CharT>::length(_data)
            };

            using sstream_detail::_parse;
            _parse(_parse_data, _value);
            return _value;
        }

        template < basic_string_literal fmt, typename... Args >
        inline void write_data(Args&&... args)
        {
            _size = _capacity;
            
            using sstream_detail::_write;
            using sstream_detail::add_const_data_t;
            using char_type = typename decltype(fmt)::char_type;
            const tuple _args_tup = {forward<add_const_data_t<decay_t<Args>>>(args)...};
            constexpr auto _fmt_buf = sstream_detail::
                parse_literal<fmt, sizeof...(Args) + 1>().unwrap();
            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "Arguments don\'t match"
            );

            auto _forward_print = [&_args_tup, &_fmt_buf, this]<usize I>()
            {
                using arg_type = decltype(_args_tup.template get<I>());
                const auto& _arg = _args_tup.template get<I>();

                sstream_detail::_sub_from(
                    _size, _write<
                        format_literal<char_type, _fmt_buf[I].length + 1> {
                            .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                            .tag = _fmt_buf[I].tag,
                            .foreground = _fmt_buf[I].foreground,
                            .background = _fmt_buf[I].background
                        }
                    >(forward<arg_type>(_arg), {_data + (_capacity - _size), _size})
                ).unwrap();
            };

            constexpr auto _last = _fmt_buf[sizeof...(Args)];
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                // This crashes clang on windows
                ((_forward_print.template operator()<Ints>()), ...);
            }(make_index_sequence<sizeof...(Args)>{});

            usize _last_len = static_cast<usize>(
                _write<
                    format_literal<char_type, _last.length + 1> {
                        .format = {_last.format, _last.length},
                        .tag = _last.tag,
                        .foreground = _last.foreground,
                        .background = _last.background
                    }
                >(
                    {_data + (_capacity - _size), _size}
                )
            );
            _size = _capacity - _size + _last_len;
        }

        inline void pop_back()
        {
            _data[--_size] = '\0';
        }

        inline void clear()
        {
            _data[0] = '\0';
            _size = 0;
        }

        inline usize capacity() const
        {
            return _capacity;
        }

        inline usize size() const
        {
            return _size;
        }

        inline iterator data()
        {
            return _data;
        }

        inline const_iterator c_str() const
        {
            return _data;
        }
    };

    template <typename CharT, usize Size>
    class static_basic_sstream
    {
    private:
        CharT _data[Size] = {};
        usize _size = 0;

    public:
        using iterator = CharT*;
        using const_iterator = const CharT*;
        static_basic_sstream(const static_basic_sstream&) = delete;
        static_basic_sstream& operator=(const static_basic_sstream&) = delete;

        constexpr static_basic_sstream() = default;

        constexpr void add_raw_data(const CharT* raw_data)
        {
            basic_cstring<CharT>::add(_data, raw_data, _size);
            _size += basic_cstring<CharT>::length(raw_data);

            _data[_size++] = static_cast<CharT>(' ');
            _data[_size] = static_cast<CharT>('\0');
        }

        constexpr void update_size()
        {
            _size = basic_cstring<CharT>::length(_data);
        }

        template <typename... Args>
        inline Result<void, runtime_error> set_data(Args&... args)
        {
            using sstream_detail::_parse;
            auto _data_set = sstream_detail::split_data<sizeof...(Args)>(_data);

            if (sizeof...(Args) > _data_set.size())
            {
                return runtime_error{"Input too small to parse"};
            }
            else
            {
                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (_parse(_data_set[Ints], args), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }

            return {};
        }

        template <typename T>
        inline T parse()
        {
            T _value{};
            set_data(_value).unwrap();
            return _value;
        }

        template < basic_string_literal fmt, typename... Args >
        inline void write_data(Args&&... args)
        {
            _size = capacity();

            using sstream_detail::_write;
            using sstream_detail::add_const_data_t;
            using char_type = typename decltype(fmt)::char_type;
            const tuple _args_tup = {forward<add_const_data_t<decay_t<Args>>>(args)...};
            constexpr auto _fmt_buf = sstream_detail::
                parse_literal<fmt, sizeof...(Args) + 1>().unwrap();
            static_assert(
                _fmt_buf.size() == sizeof...(Args) + 1, 
                "Arguments don\'t match"
            );

            auto _forward_print = [&_args_tup, &_fmt_buf, this]<usize I>()
            {
                using arg_type = decltype(_args_tup.template get<I>());
                const auto& _arg = _args_tup.template get<I>();

                sstream_detail::_sub_from(
                    _size, _write<
                        format_literal<char_type, _fmt_buf[I].length + 1> {
                            .format = {_fmt_buf[I].format, _fmt_buf[I].length},
                            .tag = _fmt_buf[I].tag,
                            .foreground = _fmt_buf[I].foreground,
                            .background = _fmt_buf[I].background
                        }
                    >(forward<arg_type>(_arg), {_data + (capacity() - _size), _size})
                ).unwrap();
            };

            constexpr auto _last = _fmt_buf[sizeof...(Args)];
            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                // This crashes clang on windows
                ((_forward_print.template operator()<Ints>()), ...);
            }(make_index_sequence<sizeof...(Args)>{});

            usize _last_len = static_cast<usize>(
                _write<
                    format_literal<char_type, _last.length + 1> {
                        .format = {_last.format, _last.length},
                        .tag = _last.tag,
                        .foreground = _last.foreground,
                        .background = _last.background
                    }
                >(
                    {_data + (capacity() - _size), _size}
                )
            );

            _size = capacity() - _size + _last_len;
        }

        constexpr void pop_back()
        {
            _data[--_size] = '\0';
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
    
    using sstream = basic_sstream<char>;
    using wsstream = basic_sstream<wchar>;
    using u8sstream = basic_sstream<char8>;
    using u16sstream = basic_sstream<char16>;
    using u32sstream = basic_sstream<char32>;

    template <usize Size>
    using static_sstream = static_basic_sstream<char, Size>;
    template <usize Size>
    using static_wsstream = static_basic_sstream<wchar, Size>;
    template <usize Size>
    using static_u8sstream = static_basic_sstream<char8, Size>;
    template <usize Size>
    using static_u16sstream = static_basic_sstream<char16, Size>;
    template <usize Size>
    using static_u32sstream = static_basic_sstream<char32, Size>;
} // namespace hsd
