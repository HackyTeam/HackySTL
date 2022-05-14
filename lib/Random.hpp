#pragma once

#include "Limits.hpp"
#include "Result.hpp"

namespace hsd
{
    template < 
        typename UType, UType n, 
        UType m, UType r, UType a, 
        UType u, UType d, UType s, 
        UType b, UType t, UType c, 
        UType l, UType f >
    requires (is_unsigned<UType>::value)
    class mersenne_twister
    {
    public:
        static constexpr UType word_size = sizeof(UType) * 8;
        static constexpr UType state_size = n;
        static constexpr UType shift_size = m;
        static constexpr UType mask_bits = r;
        static constexpr UType xor_mask = a;
        static constexpr UType tempering_u = u;
        static constexpr UType tempering_d = d;
        static constexpr UType tempering_s = s;
        static constexpr UType tempering_b = b;
        static constexpr UType tempering_t = t;
        static constexpr UType tempering_c = c;
        static constexpr UType tempering_l = l;
        static constexpr UType init_multiplier = f;
        static constexpr UType default_seed = 5489u;

    private:
        UType _mt_states[state_size]{};
        UType _index = state_size + 1;
        static constexpr UType _lower_mask = (1u << mask_bits) - 1u;
        static constexpr UType _upper_mask = ~((1u << mask_bits) - 1u);

        void _twist()
        {
            for (UType _state_index = 0; _state_index < state_size; _state_index++)
            {
                UType _value = (_mt_states[_state_index] & _upper_mask) +
                    (_mt_states[(_state_index + 1) % state_size] & _lower_mask);

                UType _twist_xor_mask = _value % 2 == 0 ? 
                    _value >> 1 : (_value >> 1) ^ xor_mask;
                
                _mt_states[_state_index] = _mt_states[
                    (_state_index + shift_size) % state_size
                ] ^ _twist_xor_mask;
            }

            _index = 0;
        }

    public:
        constexpr mersenne_twister(UType seed = default_seed)
            : _index{state_size}
        {
            _mt_states[0] = seed;

            for (UType _state_index = 1; _state_index < state_size; _state_index++)
            {
                _mt_states[_state_index] = init_multiplier * (_mt_states[_state_index - 1] ^ 
                    (_mt_states[_state_index - 1] >> (word_size - 2))) + _state_index;
            }
        }

        constexpr ~mersenne_twister() = default;

        // This one can "throw" an error
        template <typename RetType> requires (IsNumber<RetType>)
        constexpr auto generate(RetType from, RetType to)
            -> result<RetType, runtime_error>
        {
            if (from >= to)
            {
                return runtime_error{"Invalid interval"};
            }
            
            if (_index == state_size)
            {
                _twist();
            }

            UType _result = _mt_states[_index];
            _result = _result ^ ((_result >> tempering_u) & tempering_d);
            _result = _result ^ ((_result >> tempering_s) & tempering_b);
            _result = _result ^ ((_result >> tempering_t) & tempering_c);
            _result = _result ^ (_result >> tempering_l);
            _index++;

            if constexpr (word_size == 64)
            {
                auto _multiplier = static_cast<f64>(_result) / static_cast<f64>(limits<UType>::max);
                return static_cast<RetType>(static_cast<f64>(to - from) * _multiplier) + from;
            }
            else
            {
                auto _multiplier = static_cast<f32>(_result) / static_cast<f32>(limits<UType>::max);
                return static_cast<RetType>(static_cast<f32>(to - from) * _multiplier) + from;
            }
        }
        
        template <typename RetType>
        requires (IsFloat<RetType>)
        constexpr auto generate()
        {
            return generate<RetType>(0, 1).unwrap();
        }

        constexpr void discard(usize amount)
        {
            for(; amount != 0; amount--)
                generate();
        }

        // This one can't "throw" an error
        constexpr UType generate()
        {            
            if (_index == state_size)
                _twist();

            UType _result = _mt_states[_index];
            _result = _result ^ ((_result >> tempering_u) & tempering_d);
            _result = _result ^ ((_result >> tempering_s) & tempering_b);
            _result = _result ^ ((_result >> tempering_t) & tempering_c);
            _result = _result ^ (_result >> tempering_l);
            _index++;

            return _result;
        }
    };

    using mt19937_32 = mersenne_twister< 
        u32, 624, 397, 31, 0x9908b0df, 
        11, 0xffffffff, 7, 0x9d2c5680, 15,
        0xefc60000, 18, 1812433253 >; 	

    using mt19937_64 = mersenne_twister< 
        u64, 312, 156, 31, 0xb5026f5aa96619e9, 29,
        0x5555555555555555, 17, 0x71d67fffeda60000, 37,
        0xfff7eee000000000, 43, 6364136223846793005 >;
} // namespace hsd
