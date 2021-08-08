#include "Types.hpp"

namespace hsd
{
    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 1 && sizeof(CharT2) == 1)
    static constexpr void to_utf8(CharT1* dest, const CharT2* src)
    {
        while (*src != static_cast<CharT2>('\0'))
        {
            *dest++ = static_cast<CharT1>(*src++);
        }

        *dest = static_cast<CharT1>('\0');
    }

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 1 && sizeof(CharT2) == 2)
    static constexpr void to_utf8(CharT1* dest, const CharT2* src)
    {
        char32 _expr = 0;

        while (*src != static_cast<CharT2>('\0'))
        {
            if (
                (static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xFC00) == 0xD800 && 
                (static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xFC00) == 0xDC00
            ) {
                _expr = static_cast<char32>(
                    (
                        ((static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x3FF) << 10) | 
                        (static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3FF)
                    ) + 0x10000
                );
            }
            else
            {
                _expr = static_cast<char32>(src[0]);
            }

            if (_expr < 0x10000)
            {
                if (_expr < 0x80)
                {
                    // ASCII
                    dest[0] = static_cast<CharT1>(_expr);
                    
                    dest += 1; src += 1;
                }
                else if (_expr < 0x800)
                {
                    // 11-bit
                    dest[0] = static_cast<CharT1>(0xC0 | ((_expr >> 6) & 0x1F));
                    dest[1] = static_cast<CharT1>(0x80 | (_expr & 0x3F));
                    
                    dest += 2; src += 1;
                }
                else
                {
                    // 16-bit
                    dest[0] = static_cast<CharT1>(0xE0 | ((_expr >> 12) & 0x0F));
                    dest[1] = static_cast<CharT1>(0x80 | ((_expr >> 6) & 0x3F));
                    dest[2] = static_cast<CharT1>(0x80 | (_expr & 0x3F));
                    
                    dest += 3; src += 1;
                }
            }
            else
            {
                // 21-bit
                dest[0] = static_cast<CharT1>(0xF0 | ((_expr - 0x10000) >> 18));
                dest[1] = static_cast<CharT1>(0x80 | ((_expr >> 12) & 0x3F));
                dest[2] = static_cast<CharT1>(0x80 | ((_expr - 0x10000) >> 6 & 0x3F));
                dest[3] = static_cast<CharT1>(0x80 | ((_expr - 0x10000) & 0x3F));
                
                dest += 4; src += 2;
            }
        }

        *dest = static_cast<CharT1>('\0');
    }

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 1 && sizeof(CharT2) == 4)
    static constexpr void to_utf8(CharT1* dest, const CharT2* src)
    {
        while (*src != static_cast<CharT2>('\0'))
        {
            if (*src < 0x7FFFF)
            {
                dest[0] = static_cast<CharT1>(0xF0 | (
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 18
                    )
                );

                dest[1] = static_cast<CharT1>(0x80 | ((
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 12
                    ) & 0x3F)
                );
                dest[2] = static_cast<CharT1>(0x80 | ((
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 6
                    ) & 0x3F)
                );
                
                dest[3] = static_cast<CharT1>(0x80 | (
                    static_cast<add_unsigned_t<CharT2>>(*src) & 0x3F
                    )
                );
                
                dest += 4; src += 1;
            }
            else if (*src < 0x1FFF)
            {
                dest[0] = static_cast<CharT1>(0xE0 | (
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 12
                    )
                );

                dest[1] = static_cast<CharT1>(0x80 | ((
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 6
                    ) & 0x3F)
                );

                dest[2] = static_cast<CharT1>(0x80 | (
                        static_cast<add_unsigned_t<CharT2>>(*src) & 0x3F
                    )
                );

                dest += 3; src += 1;
            }
            else if (*src < 0x7FF)
            {
                dest[0] = static_cast<CharT1>(0xC0 | (
                        static_cast<add_unsigned_t<CharT2>>(*src) >> 6
                    )
                );

                dest[1] = static_cast<CharT1>(0x80 | (
                        static_cast<add_unsigned_t<CharT2>>(*src) & 0x3F
                    )
                );
                
                dest += 2; src += 1;
            }
            else
            {
                dest[0] = static_cast<CharT1>(*src);
                
                dest += 1; src += 1;
            }
        }
        
        *dest = static_cast<CharT1>('\0');
    }

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 2 && sizeof(CharT2) == 1)
    static constexpr void to_utf16(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT2>('\0'))
		{
        	if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xF8) == 0xF0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[3]) & 0xC0) == 0x80
			) {
				// UTF-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        	    const auto expr = static_cast<char32>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x07) << 18 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F) << 12 | 
					(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0x3F) << 6  | 
					(static_cast<add_unsigned_t<CharT2>>(src[3]) & 0x3F)
				);

        	    dest[0] = static_cast<CharT1>(0xD800 | ((expr - 0x10000) >> 10));
        	    dest[1] = static_cast<CharT1>(0xDC00 | ((expr - 0x10000) & 0x3FF));
				
				dest += 2; src += 4;
        	}
        	else if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xF0) == 0xE0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0xC0) == 0x80
			) {
				// UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
        	    const auto expr = static_cast<char32>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x0F) << 12 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F) << 6  | 
					(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0x3F)
				);
	
        	    if (expr < 0x10000)
        	    {
        	        dest[0] = static_cast<CharT1>(expr);
        	        
					dest += 1; src += 3;
        	    }
        	    else
        	    {
        	        dest[0] = static_cast<CharT1>(0xD800 | ((expr - 0x10000) >> 10));
        	        dest[1] = static_cast<CharT1>(0xDC00 | ((expr - 0x10000) & 0x3FF));
        	        
					dest += 2; src += 3;
        	    }
        	}
        	else if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xE0) == 0xC0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80
			) {
				// UTF-8: 110xxxxx 10xxxxxx
        	    dest[0] = static_cast<CharT1>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x1F) << 6 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F)
				);

        	    dest += 1; src += 2;
        	}
        	else
        	{
				// UTF-8: 0xxxxxxx
        	    dest[0] = static_cast<CharT1>(src[0]);
        	    
				dest += 1; src += 1;
        	}
		}

        *dest = static_cast<CharT1>('\0');
	}

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 2 && sizeof(CharT2) == 2)
	static constexpr void to_utf16(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT1>('\0'))
        {
            *dest++ = static_cast<CharT1>(*src++);
        }

        *dest = static_cast<CharT1>('\0');
	}

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 2 && sizeof(CharT2) == 4)
	static constexpr void to_utf16(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT2>('\0'))
		{
			if (static_cast<add_unsigned_t<CharT2>>(*src) < 0x10000)
			{
        	    dest[0] = static_cast<CharT1>(*src);
        	    
				dest += 1; src += 1;
        	}
        	else
        	{
        	    dest[0] = static_cast<CharT1>(
					0xD800 | ((
                        static_cast<add_unsigned_t<CharT2>>(*src) - 0x10000
                    ) >> 10)
				);
        	    dest[1] = static_cast<CharT1>( 
					0xDC00 | ((
                        static_cast<add_unsigned_t<CharT2>>(*src) - 0x10000
                    ) & 0x3FF)
				);
        	    
				dest += 2; src += 1;
        	}
		}

        *dest = static_cast<CharT1>('\0');
	}

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 4 && sizeof(CharT2) == 1)
	static constexpr void to_utf32(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT2>('\0'))
		{
			if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xF8) == 0xF0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[3]) & 0xC0) == 0x80
			) {
				// UTF-8: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        	    *dest = static_cast<CharT1>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x07) << 18 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F) << 12 | 
					(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0x3F) << 6  | 
					(static_cast<add_unsigned_t<CharT2>>(src[3]) & 0x3F)
				);

    			src += 4;
        	}
        	else if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xF0) == 0xE0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80 && 
				(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0xC0) == 0x80
			) {
				// UTF-8: 1110xxxx 10xxxxxx 10xxxxxx
				*dest = static_cast<char32>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x0F) << 12 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F) << 6  | 
					(static_cast<add_unsigned_t<CharT2>>(src[2]) & 0x3F)
				);
				
				src += 3;
        	}
        	else if (
				(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xE0) == 0xC0 && 
				(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xC0) == 0x80
			) {
				// UTF-8: 110xxxxx 10xxxxxx
				*dest = static_cast<char32>(
					(static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x1F) << 6 | 
					(static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x3F)
				);

				src += 2;
        	}
        	else
        	{
				// ASCII or invalid UTF-8
				*dest = static_cast<char32>(src[0]);
				src += 1;
        	}

			dest++;
		}

        *dest = static_cast<CharT1>('\0');
	}

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 4 && sizeof(CharT2) == 2)
	static constexpr void to_utf32(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT2>('\0'))
		{
			if (
                (static_cast<add_unsigned_t<CharT2>>(src[0]) & 0xFC00) == 0xD800 && 
                (static_cast<add_unsigned_t<CharT2>>(src[1]) & 0xFC00) == 0xDC00
            )
			{
				// UTF-16: 110110?? 10xxxxxx 10xxxxxx
				*dest = static_cast<CharT1>(
					((static_cast<add_unsigned_t<CharT2>>(src[0]) & 0x03FF) << 10 | 
                    (static_cast<add_unsigned_t<CharT2>>(src[1]) & 0x03FF)) + 0x10000
				);

				src += 2;
			}
			else
			{
				// ASCII or invalid UTF-16
				*dest = static_cast<CharT1>(src[0]);
				src += 1;
			}

			dest++;
		}

        *dest = static_cast<CharT1>('\0');
	}

    template <typename CharT1, typename CharT2>
    requires (sizeof(CharT1) == 4 && sizeof(CharT2) == 4)
	static constexpr void to_utf32(CharT1* dest, const CharT2* src)
	{
		while (*src != static_cast<CharT2>('\0'))
        {
            *dest++ = static_cast<CharT1>(*src++);
        }

        *dest = static_cast<CharT1>('\0');
	}
} // namespace hsd
