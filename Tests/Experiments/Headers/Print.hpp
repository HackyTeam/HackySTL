#pragma once

#include <Concepts.hpp>
#include <unistd.h>

namespace hsd_test
{
    template <typename CharT>
    struct fmt_common
    {
        static constexpr const CharT fg_fmt[8] = {
            '\x1b', '[', '3', '8', ';', '5', ';', 'm'
        };
        static constexpr const CharT bg_fmt[8] = {
            '\x1b', '[', '4', '8', ';', '5', ';', 'm'
        };
        static constexpr const CharT reset_fmt[4] = {
            '\x1b', '[', '0', 'm'
        };

        static constexpr hsd::uchar none = 0; 
        static constexpr hsd::uchar hex  = 1; // hexadecimal
        static constexpr hsd::uchar exp  = 2; // exponent
        static constexpr hsd::uchar fg   = 4; // foreground color
        static constexpr hsd::uchar bg   = 8; // background color

        hsd::uchar tag;
        hsd::uchar foreground;
        hsd::uchar background;
    };

    template <typename CharT, hsd::usize N>
    struct fmt_storage
    {
        CharT data[N];
        hsd::usize len;

        constexpr const CharT* get_cstr() const
        {
            return data + N - len;
        }

        constexpr hsd::usize get_size() const
        {
            return N;
        }
    };
    

    template <typename CharT, fmt_common<CharT> fmt>
    requires (!(fmt.tag & fmt.hex) && !(fmt.tag & fmt.exp))
    static constexpr auto format(auto value)
    requires (hsd::is_integral<decltype(value)>::value)
    {
        fmt_storage<CharT, 20> _buffer = {};
        bool _neg = (value < 0) ? (value = -value, true) : false;
        
        do
        {
            _buffer.data[_buffer.get_size() - ++_buffer.len] = 
                static_cast<CharT>('0' + (value % 10));
            
            value /= 10;
        } while (value != 0);
        
        if (_neg == true)
        {
            _buffer.data[_buffer.get_size() - ++_buffer.len] = 
                static_cast<CharT>('-');
        }
        
        return _buffer;
    }
} // namespace hsd_test
