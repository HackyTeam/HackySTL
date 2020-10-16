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
                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (sstream_detail::_parse(_data_set[Ints], args), ...);
                }(make_index_sequence<sizeof...(Args)>{});
            }
		}

        template<typename T>
        T parse()
        {
            T _value{};
            using sstream_detail::_parse;
            string<CharT> _str = move(to_string());
            _parse(_str, _value);
            return _value;
        }

        string<CharT> to_string()
        {
            return string<CharT>(_data);
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
    
	using wsstream = sstream<wchar>;
    using u8sstream = sstream<char>;
    using u16sstream = sstream<char16>;
    using u32sstream = sstream<char32>;
} // namespace hsd
