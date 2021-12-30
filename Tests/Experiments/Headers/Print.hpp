#pragma once

#include <Pair.hpp>

namespace hsd_test
{
    template <typename CharT, hsd::uchar tag>
    //requires (!(tag & hsd_fmt_tag_hex) && !(hsd_fmt_tag_exp))
    static constexpr auto format(hsd::i8 value)
        -> hsd::pair<const CharT*, hsd::usize>
    {
        static CharT _buffer[4] = {};
        hsd::usize _len = 0;
        bool neg = (value < 0) ? (val = -val, true) : false;

        do
        {
            _buffer[3 - _len++] = static_cast<CharT>('0' + (val % 10));
            val /= 10;
        } while (val);

        if (neg)
        {
            _buffer[3 - _len++] = static_cast<CharT>('-');
        }

        return {_buffer + 4 - _len, _len};
    }
} // namespace hsd_test
