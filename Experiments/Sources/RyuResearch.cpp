#include "Base/Limits.hpp"
#include "Base/Math.hpp"
#include "Base/Result.hpp"
#include "Base/Types.hpp"
#include <Base/Concepts.hpp>
#include <Wrappers/Pair.hpp>
#include <Containers/Integer.hpp>
#include <Containers/String.hpp>

namespace hsd
{
    namespace ryu_detail
    {
        template <FloatingPointType T>
        struct fp_rep 
        {
            using IntType = conditional_t<
                sizeof(T) == 16, u128, conditional_t<
                    sizeof(T) == 8, u64, u32
                >
            >;

            IntType mantissa : limits<T>::mantissa;
            IntType exponent : limits<T>::exponent;
            IntType sign : 1;
        };
    
        template <FloatingPointType T>
        static consteval auto get_log_table()
        {
            constexpr usize _sz = 
                limits<T>::max_exponent - 
                limits<T>::min_exponent_denorm;
    
            stack_array<T, _sz> _arr{};
            auto _val = limits<T>::denorm_min;
    
            for (auto _it = _arr.begin(); _it != _arr.end() - 1; ++_it)
            {
                *_it = _val;
                _val *= 2;
            }
    
            _arr[_sz - 1] = _val;
            return _arr;
        }
    
        template <bool Floating = true>
        static constexpr auto get_base_2_exp(auto val)
        {
            using FpType = decltype(val);
            
            constexpr auto _min_exp = limits<FpType>::min_exponent_denorm;
            constexpr auto _max_exp = limits<FpType>::max_exponent;
            constexpr auto _array = get_log_table<FpType>();
            
            i32 _left = 0;
            i32 _right = _max_exp - _min_exp - 1;
            
            while (_left <= _right)
            {
                const auto _middle = (_left + _right) / 2;
    
                if (_array[_middle] < val)
                {
                    _left = _middle + 1;
                }
                else if (_array[_middle] > val)
                {
                    _right = _middle - 1;
                }
                else
                {
                    return pair{_middle + _min_exp, _array[_middle]};
                }
            }
    
            if constexpr (Floating == true)
            {    
                // small enough number to not be bothered
                auto _div_val = math::log2(val / _array[_right]);
    
                return pair{static_cast<i32>(_right + _min_exp + _div_val), _array[_right]};
            }
            else
            {
                return pair{_right + _min_exp, _array[_right]};
            }
        }
    
        template <FloatingPointType T>
        static constexpr auto get_subnormal_exp()
        {
            i32 _exp = 0;
            constexpr T _until = limits<T>::min / 2;
    
            for (T _val = 1; _val != _until; _val /= 2)
            {
                ++_exp;
            }
    
            return _exp;
        }
    
        template <FloatingPointType T>
        static constexpr auto generate_mantissa(T val)
        {
            u64 _mantissa = 0;
            T _mantissa_mask = 1.5;
    
            for (; _mantissa_mask != 1; _mantissa_mask *= .5, _mantissa_mask += 1)
            {
                _mantissa <<= 1;
    
                if (val >= _mantissa_mask)
                {
                    _mantissa |= 1;
                    val -= (_mantissa_mask - 1);
                }
    
                _mantissa_mask -= 1;
            }
    
            return _mantissa;
        }
    
        template <FloatingPointType T>
        static constexpr auto generate_subnorm_rep(T val)
        {
            constexpr auto _exp = -get_subnormal_exp<T>();
            constexpr auto _subnorm_max = limits<T>::min / 2;
            u64 _mant = 0;
    
            bool _negative = val < 0;

            for (T _mantissa_mask = _subnorm_max; _mantissa_mask != 0; _mantissa_mask *= .5)
            {
                _mant <<= 1;
    
                if (val >= _mantissa_mask)
                {
                    _mant |= 1;
                    val -= _mantissa_mask;
                }
            }
    
            return fp_rep<T> { 
                .mantissa = _mant,
                .exponent = _exp + (limits<T>::max_exponent + 1), 
                .sign = _negative
            };
        }
    
        template <FloatingPointType T>
        static constexpr auto get_rep(T val)
        {
            if (math::abs(val) == 0)
            {
                return fp_rep<T>{};
            }
            if (val < limits<T>::min)
            {
                return generate_subnorm_rep(val);
            }
    
            auto [_exp, _pow] = get_base_2_exp<false>(val);
            auto _mant = generate_mantissa(val / _pow);
    
            return fp_rep<T> {
                .mantissa = _mant,
                .exponent = static_cast<u64>(_exp + (limits<T>::max_exponent + 1)),
                .sign = val < 0
            };
        }
            
        template <FloatingPointType T>
        static constexpr auto get_rep_experimental(T val)
        {
            auto _res = bit_cast<fp_rep<T>>(val);
    
            if constexpr (sizeof(T) == 16 && IsSame<f128, T>)
            {
                // x87 moronic math
                _res.mantissa <<= 1; 
                _res.mantissa >>= 1;
            }

            return _res;
        }

        static constexpr usize log10_2_denom = 1'000'000'000ULL;
        static constexpr usize log10_2_num = log10_2_denom * math::log10(2.);

        static constexpr usize log10_5_denom = 1'000'000'000ULL;
        static constexpr usize log10_5_num = log10_5_denom * math::log10(5.);

        static constexpr usize log2_5_denom = 1'000'000'000ULL;
        static constexpr usize log2_5_num = log2_5_denom * math::log2(5.);

        template <typename T>
        static constexpr T pow_5_bits(T exp)
        {
            return (exp * log2_5_num + log2_5_denom - 1) / log2_5_denom;
        }

        static constexpr usize pow_5_factor(usize value)
        {
            usize count = 0;
            
            while (value > 0)
            {
                if (value % 5 != 0)
                {
                    return count;
                }

                value /= 5;
                count++;
            }

            panic("Invalid value");
        }

        static constexpr auto generate_mask(usize num_bits)
        {
            u64 _mask = 1;
            _mask <<= num_bits;

            return _mask - 1;
        }

        template <typename T>
        static constexpr usize decimal_factor(T value)
        {
            constexpr T _one_million = 1'000'000;
            usize _count = 0;

            while (value > _one_million)
            {
                value /= _one_million;
                _count += 6;
            }

            while (value > 0)
            {
                value /= 10;
                _count++;
            }

            return _count;
        }
    }

    template <FloatingPointType T>
    struct ryu_tables
    {
    private:
        static constexpr usize _num_entries = limits<T>::max_exponent10 - limits<T>::min_exponent10 + 1;
        static constexpr usize _pos_table_size = -limits<T>::min_exponent10_denorm + 2;
        static constexpr usize _inv_table_size = _num_entries - _pos_table_size;
        static constexpr usize _bitcount = sizeof(T) * 16;

        stack_array<integer<_bitcount>, _pos_table_size> _pow5_table;
        stack_array<integer<_bitcount>, _inv_table_size> _inv_pow5_table;

    public:
        constexpr ryu_tables()
        {
            using WideInteger = integer<_bitcount * 2 + _bitcount / 2>;

            integer<_bitcount> _pow5 = 1;
            WideInteger _one = 1;

            usize _max_iters = (_pos_table_size > _inv_table_size) ? _pos_table_size : _inv_table_size;


            usize _pow_5_len = 1;
            _pow5_table[0] = _pow5;
            _inv_pow5_table[0] = static_cast<integer<_bitcount>>(static_cast<WideInteger>(_one << (_bitcount)) / _pow5 + _one);

            for (usize _idx = 1; _idx < _max_iters; ++_idx)
            {
                _pow5 *= 5;
                _pow_5_len = ryu_detail::pow_5_bits(_idx);

                if (_idx < _pos_table_size)
                {
                    _pow5_table[_idx] = _pow5;
                }

                if (_idx < _inv_table_size)
                {
                    _inv_pow5_table[_idx] = static_cast<integer<_bitcount>>(static_cast<WideInteger>(_one << (_bitcount + _pow_5_len - 1)) / _pow5 + _one);
                }
            }
        }

        constexpr auto get_pow5(usize idx) const
        {
            return _pow5_table[idx];
        }

        constexpr auto get_inv_pow5(usize idx) const
        {
            return _inv_pow5_table[idx];
        }

        static constexpr auto get_bitcount()
        {
            return _bitcount;
        }
    };

    template <FloatingPointType T>
    static constexpr auto parse_floating(T value)
    {
        constexpr auto _bitcount = ryu_tables<T>::get_bitcount() / 2;
        constexpr auto _mantissa_bits = limits<T>::mantissa;
        constexpr auto _mantissa_mask = ryu_detail::generate_mask(_mantissa_bits);

        constexpr auto _exponent_bits = limits<T>::exponent;
        constexpr auto _exponent_bias = ryu_detail::generate_mask(_exponent_bits - 1);

        using IntegerType = integer<_bitcount * 2>;

        constexpr IntegerType _one = 1;

        constexpr IntegerType _mant_mask = []{
            IntegerType _mask = 1;
            _mask <<= _mantissa_bits;
            return _mask;
        }();

        const ryu_tables<T> _tables = {};
        ryu_detail::fp_rep<T> _rep = {};

        if (is_constant_evaluated() == true)
        {
            _rep = ryu_detail::get_rep(value);
        }
        else
        {
            _rep = ryu_detail::get_rep_experimental(value);
        }

        isize _exp_base2;
        IntegerType _mantissa_base2;

        if constexpr (sizeof(T) > 8)
        {
            _mantissa_base2 = IntegerType{static_cast<u64>(_rep.mantissa >> 64)};
            _mantissa_base2 <<= 64;
            _mantissa_base2 |= _rep.mantissa;
        }

        if (_rep.exponent == 0)
        {
            _exp_base2 = 1;
            _exp_base2 -= _exponent_bias;
            _exp_base2 -= _mantissa_bits;
        }
        else
        {
            _exp_base2 = _rep.exponent;
            _exp_base2 -= _exponent_bias;
            _exp_base2 -= _mantissa_bits;
            _mantissa_base2 = IntegerType{_rep.mantissa};
            _mantissa_base2 |= _mant_mask;
        }

        bool _is_mant_even = (_mantissa_base2 & _one) == 0;

        const auto _mean_current = _mantissa_base2 * 4;
        const auto _mean_next = _mantissa_base2 * 4 + 2;
        const auto _mean_prev = _mantissa_base2 * 4 - ((_mantissa_base2 != _mantissa_mask || _rep.exponent <= 1) ? 2 : 1);

        _exp_base2 -= 2;

        IntegerType _decimal_current, _decimal_next, _decimal_prev;
        isize _exp_base10;

        bool _dec_current_trailing = false;
        bool _dec_next_trailing = false;
        bool _dec_prev_trailing = false;
        u32 _last_digit = 0;

        if (_exp_base2 >= 0)
        {
            auto _jump_factor = _exp_base2 * ryu_detail::log10_2_num / ryu_detail::log10_2_denom;
            auto _2_pow_jump = _tables.get_bitcount() + ryu_detail::pow_5_bits(_jump_factor) - 1;
            auto _idx = _jump_factor + _2_pow_jump - _exp_base2;

            _decimal_current = _mean_current * _tables.get_inv_pow5(_jump_factor) >> _idx;
            _decimal_next = _mean_next * _tables.get_inv_pow5(_jump_factor) >> _idx;
            _decimal_prev = _mean_prev * _tables.get_inv_pow5(_jump_factor) >> _idx;

            if (_jump_factor != 0 && (_decimal_next - 1) / 10 <= _decimal_prev / 10)
            {
                auto _last_removed_digit_shift = _tables.get_bitcount() + ryu_detail::pow_5_bits(_jump_factor - 1) - 1;

                _last_digit = (static_cast<IntegerType>(
                    _mean_current * _tables.get_pow5(_jump_factor - 1) >> 
                    (_last_removed_digit_shift + _jump_factor - 1 - _exp_base2)
                ) % 10).get_last_byte();
            }

            _exp_base10 = _jump_factor;

            _dec_current_trailing = ryu_detail::pow_5_bits(_mean_current) >= _jump_factor;
            _dec_next_trailing = ryu_detail::pow_5_bits(_mean_next) >= _jump_factor;
            _dec_prev_trailing = ryu_detail::pow_5_bits(_mean_prev) >= _jump_factor;
        }
        else
        {
            auto _jump_factor = -_exp_base2 * ryu_detail::log10_5_num / ryu_detail::log10_5_denom;
            auto _5_pow_jump = -_exp_base2 - _jump_factor;
            auto _idx = _jump_factor + _tables.get_bitcount() - ryu_detail::pow_5_bits(_5_pow_jump);

            _decimal_current = _mean_current * _tables.get_pow5(_5_pow_jump) >> _idx;
            _decimal_next = _mean_next * _tables.get_pow5(_5_pow_jump) >> _idx;
            _decimal_prev = _mean_prev * _tables.get_pow5(_5_pow_jump) >> _idx;

            if (_jump_factor != 0 && (_decimal_next - 1) / 10 <= _decimal_prev / 10)
            {
                auto _last_removed_digit_shift = _jump_factor + _tables.get_bitcount() - ryu_detail::pow_5_bits(_5_pow_jump + 1) - 1;

                _last_digit = (static_cast<IntegerType>(
                    _mean_current * _tables.get_pow5(_5_pow_jump + 1) >> _last_removed_digit_shift
                ) % 10).get_last_byte();
            }

            _exp_base10 = _jump_factor + _exp_base2;
            //_exp_base10 /= 2;

            _dec_current_trailing = (_jump_factor < _mantissa_bits) && (_mean_current & (static_cast<IntegerType>(_one << (_jump_factor - 1)) - _one)) == 0;
            _dec_next_trailing = _jump_factor <= 1;
        }

        if constexpr (sizeof(T) == 4)
        {
            struct ryu_result
            {
                u32 decimal_current;
                u32 decimal_next;
                u32 decimal_prev;
                u32 last_digit;
                isize exp_base10;
                bool dec_current_trailing;
                bool dec_next_trailing;
                bool dec_prev_trailing;
                bool is_mant_even;
                bool is_negative;
            };

            return ryu_result {
                .decimal_current = static_cast<u32>(_decimal_current),
                .decimal_next = static_cast<u32>(_decimal_next),
                .decimal_prev = static_cast<u32>(_decimal_prev),
                .last_digit = _last_digit,
                .exp_base10 = _exp_base10,
                .dec_current_trailing = _dec_current_trailing,
                .dec_next_trailing = _dec_next_trailing,
                .dec_prev_trailing = _dec_prev_trailing,
                .is_mant_even = _is_mant_even,
                .is_negative = _rep.sign
            };
        }
        else if constexpr (sizeof(T) == 8)
        {
            struct ryu_result
            {
                u64 decimal_current;
                u64 decimal_next;
                u64 decimal_prev;
                u32 last_digit;
                isize exp_base10;
                bool dec_current_trailing;
                bool dec_next_trailing;
                bool dec_prev_trailing;
                bool is_mant_even;
                bool is_negative;
            };

            auto _res = ryu_result {
                .decimal_current = _decimal_current.get_underlying_array()[0],
                .decimal_next = _decimal_next.get_underlying_array()[0],
                .decimal_prev = _decimal_prev.get_underlying_array()[0],
                .last_digit = _last_digit,
                .exp_base10 = _exp_base10,
                .dec_current_trailing = _dec_current_trailing,
                .dec_next_trailing = _dec_next_trailing,
                .dec_prev_trailing = _dec_prev_trailing,
                .is_mant_even = _is_mant_even,
                .is_negative = _rep.sign == 1
            };

            _res.decimal_current <<= 32;
            _res.decimal_current += _decimal_current.get_underlying_array()[1];

            _res.decimal_next <<= 32;
            _res.decimal_next += _decimal_next.get_underlying_array()[1];

            _res.decimal_prev <<= 32;
            _res.decimal_prev += _decimal_prev.get_underlying_array()[1];

            return _res;
        }
        else
        {
            struct ryu_result
            {
                integer<_bitcount> decimal_current;
                integer<_bitcount> decimal_next;
                integer<_bitcount> decimal_prev;
                u32 last_digit;
                isize exp_base10;
                bool dec_current_trailing;
                bool dec_next_trailing;
                bool dec_prev_trailing;
                bool is_mant_even;
                bool is_negative;
            };

            auto _res = ryu_result {
                .decimal_current = 0,
                .decimal_next = 0,
                .decimal_prev = 0,
                .last_digit = _last_digit,
                .exp_base10 = _exp_base10,
                .dec_current_trailing = _dec_current_trailing,
                .dec_next_trailing = _dec_next_trailing,
                .dec_prev_trailing = _dec_prev_trailing,
                .is_mant_even = _is_mant_even,
                .is_negative = _rep.sign
            };

            const auto _arr_size = _bitcount / 16;

            for (usize _idx = 0; _idx < _bitcount / 32; ++_idx)
            {
                _res.decimal_current <<= 32;
                _res.decimal_current += _decimal_current.get_underlying_array()[_arr_size - _idx - 1];

                _res.decimal_next <<= 32;
                _res.decimal_next += _decimal_next.get_underlying_array()[_arr_size - _idx - 1];

                _res.decimal_prev <<= 32;
                _res.decimal_prev += _decimal_prev.get_underlying_array()[_arr_size - _idx - 1];
            }

            return _res;
        }
    }

    template <FloatingPointType T>
    static inline auto to_string(T value)
    {
        auto _parse_result = parse_floating(value);

        auto _decimal_length = ryu_detail::decimal_factor(_parse_result.decimal_next);
        isize _exponent = _parse_result.exp_base10 + _decimal_length - 1;

        usize _removed = 0;

        if (_parse_result.dec_next_trailing && !_parse_result.is_mant_even)
        {
            --_parse_result.decimal_next;
        }

        while (_parse_result.decimal_next / 10 > _parse_result.decimal_prev / 10)
        {
            if (_parse_result.decimal_next < 100)
            {
                break;
            }

            _parse_result.dec_prev_trailing &= _parse_result.decimal_prev % 10 == 0;
            _parse_result.decimal_next /= 10;
            _parse_result.last_digit = _parse_result.decimal_current % 10;

            _parse_result.decimal_current /= 10;
            _parse_result.decimal_prev /= 10;

            ++_removed;
        }

        if (_parse_result.dec_prev_trailing && _parse_result.is_mant_even)
        {
            while (_parse_result.decimal_prev % 10 == 0)
            {
                if (_parse_result.decimal_next < 100)
                {
                    break;
                }

                _parse_result.decimal_next /= 10;

                _parse_result.last_digit = _parse_result.decimal_current % 10;
                _parse_result.decimal_current /= 10;
                _parse_result.decimal_prev /= 10;

                ++_removed;
            }
        }

        if (_parse_result.dec_current_trailing && _parse_result.last_digit == 5 && _parse_result.decimal_current % 2 == 0)
        {
            _parse_result.last_digit = 4;
        }

        bool _add_one = _parse_result.decimal_current == _parse_result.decimal_prev;
        
        _add_one = _add_one && !(_parse_result.dec_prev_trailing && _parse_result.is_mant_even);
        _add_one = _add_one || _parse_result.last_digit >= 5;

        auto _output_value = _parse_result.decimal_current + (_add_one ? 1 : 0);

        //auto _output_length = _decimal_length - _removed;

        to_string_detail::to_string_value_rev<char, 21> _str{};

        bool _is_exp_negative = _exponent < 0;

        if (_is_exp_negative)
        {
            _exponent = -_exponent;
        }

        if (_exponent == 0)
        {
            _str.push('0');
        }
        else
        {
            for (; _exponent != 0; _exponent /= 10)
            {
                _str.push('0' + (_exponent % 10));
            }
        }

        if (_is_exp_negative)
        {
            _str.push('-');
        }

        _str.push('e');

        while (_output_value > 10)
        {
            _str.push('0' + (_output_value % 10));
            _output_value /= 10;
        }

        _str.push('.');
        _str.push('0' + (_output_value % 10));

        if (_parse_result.is_negative)
        {
            _str.push('-');
        }

        return _str;
    }
}

int main()
{
    hsd::f64 _val = 1e-10;

    auto _str = hsd::to_string(_val);

    2;
}