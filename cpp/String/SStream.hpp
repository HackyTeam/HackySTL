#pragma once

#include "String.hpp"
#include "../Utility/Utility.hpp"

namespace hsd
{
    namespace sstream_detail
    {
		template<typename CharT>
		static constexpr bool _is_number(CharT letter)
		{
			return letter >= '0' && letter <= '9';
		}

		template<typename CharT>
        static hsd::pair<int, const CharT*> _parse_i(const CharT* str)
		{
			int _num = 0;
			bool _negative = false;

			for(; *str != '\0' && !_is_number(*str); str++);
			_negative = *(str - 1) == '-';
			
			for(; *str != '\0' && _is_number(*str); str++)
			{
				_num *= 10;
				_num += *str - '0';
			}
			
			return {_negative ? -_num : _num, str};
		}

		template<typename CharT>
        static hsd::pair<size_t, const CharT*> _parse_us(const CharT* str)
		{
			size_t _num = 0;

			for(; *str != '\0' && !_is_number(*str); str++);
			for(; *str != '\0' && _is_number(*str); str++)
			{
				_num *= 10;
				_num += *str - '0';
			}

			return {_num, str};
		}

		template<typename CharT>
		static hsd::pair<float, const CharT*> _parse_f(const CharT* str)
		{
		    float _round_num = 0;
			float _point_num = 0;
			bool _negative = false;
			size_t _num_len = 0;

			for(; *str != '\0' && !_is_number(*str); str++)
			{
				_negative = (*str == '-');
			}

			if(*str == '\0')
				return {0.f, str};

		    for(; *str != '\0' && *str != '.' && _is_number(*str); str++)
		    {
		        _round_num *= 10;
		        _round_num += *str - '0';
		    }

			if(*str == '\0')
			{	
				return {
					_negative ? -_round_num : 
					_round_num, str
				};
			}

		    for(str += 1; *str != '\0' && _is_number(*str); str++, _num_len++)
		    {
		        _point_num *= 10;
		        _point_num += *str - '0';
		    }
		    for(; _num_len != 0; _num_len--, _point_num *= 0.1);
			
			return {
				_negative ? 
				-(_round_num + _point_num) : 
				_round_num + _point_num, str
			};
		}
    } // namespace sstream_detail
    
    template<typename CharT>
    class sstream
    {
    private:
        using size_t = unsigned long int;
        CharT* _data = nullptr;
        size_t _size = 0;
		size_t _current_pos = 0;

        template<typename T>
        void _parse(T& val)
        {
			val = parse<T>();
        }

    public:
		using iterator = CharT*;
        using const_iterator = const CharT*;

        sstream(size_t size)
        {
            _data = new CharT[size + 1];
            _data[size] = '\0';
            _size = size;

        }

        sstream(const sstream& other) = delete;

        ~sstream()
        {
            delete[] _data;
        }

		template<typename... Args>
		void set_data(Args&... args)
		{
			(_parse(args), ...);
		}

        template<typename T>
        T parse()
        {
            if(_data == nullptr)
            {
                throw std::runtime_error("Cannot parse an empty stream");
            }
            if constexpr(std::is_integral_v<T>)
            {
                if constexpr(std::is_unsigned_v<T>)
                {
                    auto [_ret, _pos_ptr] = sstream_detail::_parse_us(&_data[_current_pos]);
					_current_pos = _pos_ptr - _data;
					return _ret;
                }
                else
                {
                    auto [_ret, _pos_ptr] = sstream_detail::_parse_i(&_data[_current_pos]);
					_current_pos = _pos_ptr - _data;
					return _ret;
                }
            }
            else if constexpr(std::is_floating_point_v<T>)
            {
                auto [_ret, _pos_ptr] = sstream_detail::_parse_f(&_data[_current_pos]);
				_current_pos = _pos_ptr - _data;
				return _ret;
            }
        }

        hsd::string<CharT> to_string()
        {
            return hsd::string<CharT>(_data, _size);
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
        }

        size_t size()
        {
            return _size;
        }

        iterator data()
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
    
        iterator end()
        {
            return begin() + size();
        }
    
        const_iterator cbegin()
        {
            return data();
        }
    
        const_iterator cend()
        {
            return cbegin() + size();
        }
    };
    
	using wsstream = hsd::sstream<wchar_t>;
    using u8sstream = hsd::sstream<char>;
    using u16sstream = hsd::sstream<char16_t>;
    using u32sstream = hsd::sstream<char32_t>;
} // namespace hsd
