#pragma once

#include "_SStreamDetail.hpp"

namespace hsd
{
    
    template <typename CharT>
    class basic_sstream
    {
    private:
        CharT* _data = nullptr;
        usize _size = 0;
		usize _current_pos = 0;

    public:
		using iterator = CharT*;
        using const_iterator = const CharT*;
        basic_sstream(const basic_sstream& other) = delete;

        basic_sstream(usize size)
        {
            _data = new CharT[size + 1];
            _data[size] = '\0';
            _size = size;
        }

        ~basic_sstream()
        {
            delete[] _data;
        }

		template <typename... Args>
		void set_data(Args&... args)
		{
            auto _data_set = sstream_detail::split_data(_data, _size);

            if(sizeof...(Args) > _data_set.size())
            {
                hsd_fputs_check(stderr, "Input too small to parse");
                abort();
            }
            else if(sizeof...(Args) < _data_set.size())
            {
                hsd_fputs_check(stderr, "Warning: Possible Undefined Behavior");
            }
            else
            {
                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (sstream_detail::_parse(_data_set[Ints], args), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }
		}

        template <typename T>
        T parse()
        {
            T _value{};
            using sstream_detail::_parse;
            auto _str = pair{c_str(), size()};
            _parse(_str, _value);
            return _value;
        }

        template < basic_string_literal fmt, typename... Args >
		void write_data(Args&&... args)
		{
            using char_type = typename decltype(fmt)::char_type;
            using sstream_detail::_write;
            constexpr auto _fmt_buf = sstream_detail::split_literal<fmt, sizeof...(Args) + 1>().unwrap();
            static_assert(_fmt_buf.size() == sizeof...(Args) + 1, "Arguments don\'t match");
            usize _data_len = _size;

            constexpr auto _len = _fmt_buf[sizeof...(Args)].second;
            constexpr basic_string_literal<char_type, _len + 1> _last{
                _fmt_buf[sizeof...(Args)].first, _len
            };

            [&]<usize... Ints>(index_sequence<Ints...>)
            {
                (
                    (sstream_detail::_sub_from(_data_len, _write<
                    basic_string_literal< char_type, _fmt_buf[Ints].second + 1 >{
                        _fmt_buf[Ints].first, _fmt_buf[Ints].second
                    }>(args, {_data + (_size - _data_len), _data_len})), ...)
                );
            }(make_index_sequence<sizeof...(Args)>{});

            _write<_last>({_data + (_size - _data_len), _data_len});
		}

        basic_string<CharT> to_string()
        {
            return basic_string<CharT>(_data);
        }

		void pop_back()
        {
            _data[--_size] = '\0';
        }

        void clear()
        {
            if(_data != nullptr)
                delete[] _data;
    
            _data = new CharT[1];
            _data[0] = '\0';
            _size = 0;
        }

        void reset_data()
        {
            _data[0] = '\0';
            _current_pos = 0;
        }

        usize size()
        {
            return _size;
        }

        usize size() const
        {
            return _size;
        }

        iterator data()
        {
            return _data;
        }

        iterator data() const
        {
            return _data;
        }

        const_iterator c_str() const
        {
            return _data;
        }
    
        iterator begin()
        {
            return data();
        }

        iterator begin() const
        {
            return data();
        }

        iterator end()
        {
            return begin() + size();
        }

        iterator end() const
        {
            return begin() + size();
        }

        const_iterator cbegin()
        {
            return begin();
        }

        const_iterator cbegin() const
        {
            return begin();
        }

        const_iterator cend()
        {
            return end();
        }

        const_iterator cend() const
        {
            return end();
        }
    };
    
    using sstream = basic_sstream<char>;
	using wsstream = basic_sstream<wchar>;
    using u8sstream = basic_sstream<char8>;
    using u16sstream = basic_sstream<char16>;
    using u32sstream = basic_sstream<char32>;
} // namespace hsd
