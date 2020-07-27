#pragma once

#include "../Utility/Utility.hpp"
#include <math.h>

namespace hsd
{
	using size_t = unsigned long int;
	
	template<typename CharT>
    class cstring
    {
    private:
        static constexpr bool _compare(const CharT* str, const CharT* substr)
        {
        	for(; *str && *substr; str++, substr++)
        	{
        		if(*str != *substr)
        			return false;
        	}
        	return (*substr == '\0');
        }

		template<typename T>
		static constexpr size_t _num_len(T num)
		{
			size_t _len = 0;
			while(num)
			{
				_len++;
				num /= 10;
			}
			return _len;
		}

		static constexpr bool _is_number(CharT letter)
		{
			return letter >= '0' && letter <= '9';
		}
    public:

        static constexpr const CharT* find(const CharT* str, const CharT* substr)
        {
        	for(; *str != '\0'; str++)
        	{
        		if ((*str == *substr) && _compare(str, substr))
        			return str;
        	}

        	return nullptr;
        }

        static constexpr const CharT* find(const CharT* str, CharT letter)
        {
        	for(; *str != '\0'; str++)
        	{
        		if(*str == letter)
        			return str;
        	}

        	return nullptr;
        }

		static constexpr const CharT* find_rev(const CharT* str, const CharT* substr)
        {
			const CharT* rez = nullptr;

        	for(; *str != '\0'; str++)
        	{
        		if ((*str == *substr) && _compare(str, substr))
        			rez = str;
        	}

        	return rez;
        }

        static constexpr const CharT* find_rev(const CharT* str, CharT letter)
        {
        	const CharT* rez = nullptr;

        	for(; *str != '\0'; str++)
        	{
        		if(*str == letter)
        			rez = str;
        	}

        	return rez;
        }

		static constexpr const CharT* find_rev(const CharT* str, const CharT* substr, size_t pos)
        {
        	for(; pos != 0; pos--)
        	{
        		if ((str[pos] == *substr) && _compare(&str[pos], substr))
        			return &str[pos];
        	}

        	return nullptr;
        }

        static constexpr const CharT* find_rev(const CharT* str, CharT letter, size_t pos)
        {
        	for(; pos != 0; pos--)
        	{
        		if(str[pos] == letter)
        			return &str[pos];
        	}

        	return nullptr;
        }

        static constexpr size_t length(const CharT* str)
        {
        	size_t _iter;

        	for(_iter = 0; str[_iter] != '\0'; _iter++);

        	return _iter;
        }

        static constexpr const CharT* upper(CharT* str)
        {
        	size_t _iter;

        	for(_iter = 0; str[_iter] != '\0'; _iter++);
        	{
        		if(str[_iter] >= 'a' && str[_iter] <= 'z')
        			str[_iter] -= 32;
        	}

        	return str;
        }

        static constexpr const CharT* lower(CharT* str)
        {
        	size_t _iter;

        	for(_iter = 0; str[_iter] != '\0'; _iter++);
        	{
        		if(str[_iter] >= 'a' && str[_iter] <= 'z')
        			str[_iter] += 32;
        	}

        	return str;
        }

	    static constexpr const CharT* reverse(const CharT* str, size_t size = 0)
	    {
	    	size_t _begin = 0;
	        size_t _end = size - 1;
			CharT* _buf = nullptr;

	        if(size == 0)
	        {
				_end = length(str);
				if(_end != 0){ _end--; }
			}
			
			_buf = new CharT[_end + 2];
			hsd::copy(str, str + _end + 1, _buf);

	    	for(; _begin <= _end; _begin++, _end--)
	    	{
	    		hsd::swap(_buf[_begin], _buf[_end]);
	    	}

	    	return _buf;
	    }

		static constexpr void reverse(CharT*& str, size_t size = 0)
	    {
	    	size_t _begin = 0;
	        size_t _end = size - 1;

	        if(size == 0)
	        {
				_end = length(str);
				if(_end != 0){ _end--; }
			}
	    	for(; _begin <= _end; _begin++, _end--)
	    	{
	    		hsd::swap(str[_begin], str[_end]);
	    	}
	    }

		static constexpr const CharT* to_string(int num)
		{
			bool _negative = (num < 0);
			size_t _len = _num_len(num);
			CharT* _buf = nullptr;

			if(_negative)
			{
				_len += 1;
				_buf = new CharT[_len + 1];
				_buf[_len] = '\0';
				_buf[0] = '-';
				num = -num;
			}
			else
			{
				_buf = new CharT[_len + 1];
				_buf[_len] = '\0';
			}

			while(num)
			{
				_buf[--_len] = '0' + (num % 10);
				num /= 10;
			}

			return _buf;
		}

		static constexpr const CharT* to_string(size_t num)
		{
			size_t _len = _num_len(num);
			CharT* _buf = new CharT[_len + 1];
			_buf[_len] = '\0';

			for(; num != 0; num /= 10)
			{
				_buf[--_len] = '0' + (num % 10);
			}

			return _buf;
		}

		static constexpr const CharT* to_string(float num)
		{
			size_t _len = 0;
			int _round_num = static_cast<int>(num);
			bool _negative = (_round_num < 0);
			size_t _point_num = fabs(num - _round_num) * 10000 + 1;
			
			if(_negative)
				_len = _num_len(_round_num) + 6;
			else
				_len = _num_len(_round_num) + 5;

			CharT* _buf = new CharT[_len + 1];
			_buf[_len] = '\0';

			for(; _point_num != 0; _point_num /= 10)
			{
				_buf[--_len] = '0' + (_point_num % 10);
			}
			
			_buf[--_len] = '.';
			const CharT* _round_buf = to_string(_round_num);
			hsd::move(_round_buf, _round_buf + _len, _buf);
			return _buf;
		}

		static constexpr const CharT* to_string(double num)
		{
			size_t _len = 0;
			int _round_num = static_cast<int>(num);
			bool _negative = (_round_num < 0);
			size_t _point_num = fabs(num - _round_num) * 1000000 + 1;
			
			if(_negative)
				_len = _num_len(_round_num) + 8;
			else
				_len = _num_len(_round_num) + 7;
			
			CharT* _buf = new CharT[_len + 1];
			_buf[_len] = '\0';

			for(; _point_num != 0; _point_num /= 10)
			{
				_buf[--_len] = '0' + (_point_num % 10);
			}
			
			_buf[--_len] = '.';
			const CharT* _round_buf = to_string(_round_num);
			hsd::move(_round_buf, _round_buf + _len, _buf);
			delete[] _round_buf;
			return _buf;
		}

		static constexpr int parse_i(const CharT* str)
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
			
			return _negative ? -_num : _num;
		}

		static constexpr size_t parse_us(const CharT* str)
		{
			size_t _num = 0;

			for(; *str != '\0' && !_is_number(*str); str++);
			for(; *str != '\0' && _is_number(*str); str++)
			{
				_num *= 10;
				_num += *str - '0';
			}

			return _num;
		}

		static constexpr float parse_f(const CharT* str)
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
				return 0.f;

		    for(; *str != '\0' && *str != '.' && _is_number(*str); str++)
		    {
		        _round_num *= 10;
		        _round_num += *str - '0';
		    }

			if(*str == '\0')
				return _round_num;

		    for(str += 1; *str != '\0'; str++, _num_len++)
		    {
		        _point_num *= 10;
		        _point_num += *str - '0';
		    }
		    for(; _num_len != 0; _num_len--, _point_num *= 0.1);
			
			return _negative ? -(_round_num + _point_num) : _round_num + _point_num;
		}

		static constexpr int compare(const CharT* lhs, const CharT* rhs)
		{
			size_t _index = 0;

			for(; lhs[_index] && rhs[_index]; _index++)
			{
				if(lhs[_index] < rhs[_index])
					return -1;
				else if(lhs[_index] > rhs[_index])
					return 1;
			}
			
			if(lhs[_index] < rhs[_index])
				return -1;
			else if(lhs[_index] > rhs[_index])
				return 1;
			else
				return 0;
		}

		static constexpr int compare(const CharT* lhs, const CharT* rhs, size_t len)
		{
			size_t _index = 0;

			for(; _index < len && lhs[_index] && rhs[_index]; _index++)
			{
				if(lhs[_index] < rhs[_index])
					return -1;
				else if(lhs[_index] > rhs[_index])
					return 1;
			}
			
			if(lhs[_index] < rhs[_index])
				return -1;
			else if(lhs[_index] > rhs[_index])
				return 1;
			else
				return 0;
		}

		static constexpr CharT* copy(CharT* dest, const CharT* src, size_t len)
		{
			if (dest == nullptr)
				return nullptr;

			for(; *src && len != 0; len--, dest++, src++)
			{
				*dest = *src;
			}
			return dest;
		}

		static constexpr CharT* copy(CharT* dest, const CharT* src)
		{
			if (dest == nullptr)
				return nullptr;

			char* ptr = dest;

			for(; *src; dest++, src++)
			{
				*dest = *src;
			}
			return ptr;
		}

		static constexpr CharT* add(CharT* dest, const CharT* src)
		{
		    size_t _index = length(dest);
			size_t _index_helper = 0;

		    for (; src[_index_helper] != '\0'; _index_helper++)
		        dest[_index + _index_helper] = src[_index_helper];

			dest[_index + _index_helper] = '\0';
		    return dest;
		}

		static constexpr CharT* add(CharT* dest, const CharT* src, size_t len)
		{
			size_t _index = len;
			size_t _index_helper = 0;

		    for (; src[_index_helper] != '\0'; _index_helper++)
		        dest[_index + _index_helper] = src[_index_helper];

			dest[_index + _index_helper] = '\0';
		    return dest;
		}
    };

	using wcstring = hsd::cstring<wchar_t>;
	using u8cstring = hsd::cstring<char>;
	using u16cstring = hsd::cstring<char16_t>;
	using u32cstring = hsd::cstring<char32_t>;
}