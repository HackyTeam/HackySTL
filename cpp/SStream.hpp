#pragma once

#include "_SStreamDetail.hpp"

namespace hsd
{
    
    template<typename CharT>
    class sstream
    {
    private:
        CharT* _data = nullptr;
        usize _size = 0;
		usize _current_pos = 0;

    public:
		using iterator = CharT*;
        using const_iterator = const CharT*;
        sstream(const sstream& other) = delete;

        sstream(usize size)
        {
            _data = new CharT[size + 1];
            _data[size] = '\0';
            _size = size;
        }

        ~sstream()
        {
            delete[] _data;
        }

		template<typename... Args>
		void set_data(Args&... args)
		{
            auto _data_set = sstream_detail::split(_data, _size);

            if(sizeof...(Args) > _data_set.size())
            {
                throw std::runtime_error("Input too small");
            }
            else if(sizeof...(Args) < _data_set.size())
            {
                fprintf(stderr, "Warning");
            }
            else
            {
                [&]<usize... Ints>(hsd::index_sequence<Ints...>)
                {
                    (sstream_detail::_parse(_data_set[Ints], args), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }
		}

        template<typename T>
        T parse_custom()
        {
            T _value{};
            auto _data_set = sstream_detail::split(_data, _size);
            static_assert(
                std::is_base_of<sstream_detail::readable, T>::value,
                "You have to inherit from sstream_detail::readable"
            );
            
            if constexpr(is_same<CharT, char>::value)
            {
                _value.read(_data_set);
            }
            else if constexpr(is_same<CharT, wchar>::value)
            {
                _value.wread(_data_set);
            }

            return _value;
        }

        template<typename T>
        T parse()
        {
            T _value{};
            hsd::string<CharT> _str = move(to_string());
            sstream_detail::_parse(_str, _value);
            return _value;
        }

        hsd::string<CharT> to_string()
        {
            return hsd::string<CharT>(_data);
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
    
	using wsstream = hsd::sstream<wchar>;
    using u8sstream = hsd::sstream<char>;
    using u16sstream = hsd::sstream<char16>;
    using u32sstream = hsd::sstream<char32>;
} // namespace hsd
