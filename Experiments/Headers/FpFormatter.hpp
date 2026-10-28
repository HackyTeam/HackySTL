#pragma once

#include <Containers/StackArray.hpp>
#include <Base/Math.hpp>

namespace test
{
    template <hsd::usize Sz>
    struct fp_rep
    {
        hsd::usize end_index = 0;
        hsd::stack_array<char, Sz> data = {};
    };

    template <typename>
    struct fp_max_digits;

    template <>
    struct fp_max_digits<hsd::f128>
    {
        static constexpr hsd::usize val = 4933;
    };

    template <>
    struct fp_max_digits<hsd::f64>
    {
        static constexpr hsd::usize val = 309;
    };

    template <>
    struct fp_max_digits<hsd::f32>
    {
        static constexpr hsd::usize val = 39;
    };

    static constexpr auto round_to_string(hsd::IsFloat auto val)
    {
        using max_digits = fp_max_digits<decltype(val)>;
        fp_rep<max_digits::val> _str;

        if (val == 0)
        {
            _str.data[_str.end_index++] = '0';
            return _str;
        }

        using FpType = decltype(val);
        hsd::usize _exp = 0;
        constexpr FpType _ten = 10;

        {
            auto _copy_val = val;

            while (_copy_val != 0)
            {
                ++_exp;
                _copy_val = hsd::math::trunc(_copy_val / _ten);
            }

            _exp -= (_exp != 0) ? 1 : 0;
        }

        auto _current_decimal = (_exp != 0) ? _ten : _ten / _ten;

        while (_exp > 1)
        {
            _current_decimal *= _ten;
            --_exp;
        }

        while (val != 0)
        {
            auto _digit = static_cast<hsd::i8>(val / _current_decimal);
            _str.data[_str.end_index++] = '0' + _digit;
            val -= _current_decimal * _digit;
            _current_decimal = hsd::math::trunc(_current_decimal / _ten);
        }

        return _str;
    }

    template <hsd::usize Dec>
    static constexpr auto point_to_string(hsd::IsFloat auto val)
    {
        fp_rep<Dec> _str;

        while (_str.end_index != Dec)
        {
            auto _new_val = val * static_cast<decltype(val)>(10);
            auto _floor_val = hsd::math::floor(_new_val);
            _str.data[_str.end_index++] = static_cast<hsd::i8>(_floor_val) + '0';
            val = _new_val - _floor_val;
        }

        return _str;
    }

    template <hsd::usize DecPrecision = 6>
    static constexpr auto to_string(hsd::IsFloat auto val)
    {
        bool _neg = val < 0.;
        val = _neg ? -val : val;

        auto _floor_val = hsd::math::floor(val);
        auto _round_string = round_to_string(_floor_val);
        auto _point_string = point_to_string<DecPrecision>(val - _floor_val);

        fp_rep<_round_string.data.size() + _point_string.data.size() + 3> _str;
        _str.end_index = _neg ? 1 : 0;

        if (_neg == true)
        {
            _str.data[0] = '-';
        }

        for (; _str.end_index != _round_string.end_index; ++_str.end_index)
        {
            _str.data[_str.end_index] = _round_string.data[_str.end_index];
        }

        _str.data[_str.end_index++] = '.';

        for (hsd::usize _index = 0; _index != _point_string.end_index; ++_index)
        {
            _str.data[_str.end_index] = _point_string.data[_index];
            ++_str.end_index;
        }

        _str.data[_str.end_index] = '\0';
        return _str;
    }
} // namespace test