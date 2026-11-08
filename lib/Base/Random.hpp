#pragma once

/// @file Random.hpp
/// @brief Deterministic Mersenne Twister generators with checked ranges.
/// @details Provides 32-bit and 64-bit engines, seeded reproducibility,
/// discarding, integral/floating bounds, and invalid-interval errors.

#include "Limits.hpp"
#include "Result.hpp"

namespace hsd
{
    /// @brief Parameterized Mersenne Twister pseudo-random engine.
    /// @details The engine owns a fixed state array, regenerates that state
    /// after `state_size` outputs, and applies the configured tempering steps
    /// to each returned word.
    /// @tparam UType Unsigned word type used by the engine.
    /// @tparam n Number of words in the state array.
    /// @tparam m State shift used by the twist recurrence.
    /// @tparam r Number of upper-mask bits.
    /// @tparam a Twist xor mask.
    /// @tparam first_right_shift Right shift of the first tempering step.
    /// @tparam first_mask Mask applied to the first tempering step.
    /// @tparam second_left_shift Left shift of the second tempering step.
    /// @tparam second_mask Mask applied to the second tempering step.
    /// @tparam third_left_shift Left shift of the third tempering step.
    /// @tparam third_mask Mask applied to the third tempering step.
    /// @tparam final_right_shift Right shift of the final tempering step.
    /// @tparam f State initialization multiplier.
    template <
        typename UType, UType n,
        UType m, UType r, UType a,
        UType first_right_shift, UType first_mask, UType second_left_shift,
        UType second_mask, UType third_left_shift, UType third_mask,
        UType final_right_shift, UType f >
    requires (is_unsigned<UType>::value)
    class mersenne_twister
    {
    public:
        /// @brief Number of bits in each generated word.
        static constexpr UType word_size = sizeof(UType) * 8;
        /// @brief Number of words in the internal state.
        static constexpr UType state_size = n;
        /// @brief Recurrence shift size.
        static constexpr UType shift_size = m;
        /// @brief Number of bits retained by the upper/lower state masks.
        static constexpr UType mask_bits = r;
        /// @brief Twist recurrence xor mask.
        static constexpr UType xor_mask = a;
        /// @brief First tempering shift.
        static constexpr UType tempering_u = first_right_shift;
        /// @brief First tempering mask.
        static constexpr UType tempering_d = first_mask;
        /// @brief Second tempering shift.
        static constexpr UType tempering_s = second_left_shift;
        /// @brief Second tempering mask.
        static constexpr UType tempering_b = second_mask;
        /// @brief Third tempering shift.
        static constexpr UType tempering_t = third_left_shift;
        /// @brief Third tempering mask.
        static constexpr UType tempering_c = third_mask;
        /// @brief Final tempering shift.
        static constexpr UType tempering_l = final_right_shift;
        /// @brief Initialization recurrence multiplier.
        static constexpr UType init_multiplier = f;
        /// @brief Seed used when no seed is supplied.
        static constexpr UType default_seed = 5489u;

    private:
        /// @brief Mersenne Twister recurrence state.
        UType _mt_states[state_size]{};
        /// @brief Index of the next state word to temper.
        UType _index = state_size + 1;
        /// @brief Mask selecting upper state bits.
        static constexpr UType _upper_mask = (~UType{}) << mask_bits;
        /// @brief Mask selecting lower state bits.
        static constexpr UType _lower_mask = ~_upper_mask;

        /// @brief Regenerates all state words after the current block is consumed.
        constexpr void _twist()
        {
            for (UType _state_index = 0; _state_index < (state_size - shift_size); ++_state_index)
            {
                UType _value = (_mt_states[_state_index] & _upper_mask) |
                    (_mt_states[_state_index + 1] & _lower_mask);

                UType _twist_xor_mask = (_value & 1) ? xor_mask : 0;
                
                _mt_states[_state_index] = (
                    _mt_states[_state_index + shift_size] ^ 
                    (_value >> 1) ^ _twist_xor_mask
                );
            }

            for (UType _state_index = state_size - shift_size; 
                _state_index < (state_size - 1); ++_state_index)
            {
                UType _value = (_mt_states[_state_index] & _upper_mask) |
                    (_mt_states[_state_index + 1] & _lower_mask);

                UType _twist_xor_mask = (_value & 1) ? xor_mask : 0;
                
                _mt_states[_state_index] = (
                    _mt_states[_state_index + (shift_size - state_size)] ^ 
                    (_value >> 1) ^ _twist_xor_mask
                );
            }

            UType _value = (
                (_mt_states[state_size - 1] & _upper_mask) |
                (_mt_states[0] & _lower_mask)
            );

            UType _twist_xor_mask = (_value & 1) ? xor_mask : 0;
            
            _mt_states[state_size - 1] = (
                _mt_states[shift_size - 1] ^ (_value >> 1) ^ _twist_xor_mask
            );
            _index = 0;
        }

    public:
        /// @brief Seeds and initializes the recurrence state.
        /// @param seed Initial unsigned seed value.
        constexpr mersenne_twister(UType seed = default_seed)
            : _index{state_size}
        {
            _mt_states[0] = seed;

            for (UType _state_index = 1; _state_index < state_size; ++_state_index)
            {
                UType _new_val = _mt_states[_state_index - 1];
                _new_val ^= _new_val >> (word_size - 2);
                _new_val *= init_multiplier;
                _mt_states[_state_index] = _new_val + _state_index;
            }
        }

        constexpr ~mersenne_twister() = default;

        /// @brief Generates a numeric value in the half-open interval `[from, to)`.
        /// @param from Inclusive lower bound.
        /// @param to Exclusive upper bound.
        /// @return A generated value, or `runtime_error` when `from >= to`.
        template <NumericType RetType>
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

            UType _result = _mt_states[_index++];
            _result ^= (_result >> tempering_u) & tempering_d;
            _result ^= (_result << tempering_s) & tempering_b;
            _result ^= (_result << tempering_t) & tempering_c;
            _result ^= (_result >> tempering_l);

            // Scale by one past the largest raw value, so the multiplier stays below
            // one and the upper bound `to` is never produced.
            const f64 _multiplier = static_cast<f64>(_result) /
                (static_cast<f64>(limits<UType>::max) + 1.0);
            const f64 _scaled = (static_cast<f64>(to) - static_cast<f64>(from)) * _multiplier +
                static_cast<f64>(from);

            if constexpr (IsFloat<RetType>)
            {
                const auto _value = static_cast<RetType>(_scaled);
                // Rounding to the narrower type can still reach `to`; fall back to `from`.
                return (_value < to) ? _value : from;
            }
            else
            {
                // Round toward negative infinity, so the value stays at or above `from`.
                auto _value = static_cast<RetType>(_scaled);
                if (static_cast<f64>(_value) > _scaled)
                {
                    _value = static_cast<RetType>(_value - 1);
                }
                return _value;
            }
        }
        
        /// @brief Generates a floating-point value in `[0, 1)`.
        /// @return A generated value in the unit interval.
        template <FloatingPointType RetType>
        constexpr auto generate()
        {
            return generate<RetType>(0, 1).unwrap();
        }

        /// @brief Advances the engine by discarding `amount` generated values.
        /// @param amount Number of outputs to consume.
        constexpr void discard(usize amount)
        {
            for(; amount != 0; amount--)
                generate();
        }

        /// @brief Generates one raw tempered engine word.
        /// @return The next pseudo-random unsigned word.
        constexpr UType generate()
        {            
            if (_index == state_size)
                _twist();

            UType _result = _mt_states[_index++];
            _result ^= (_result >> tempering_u) & tempering_d;
            _result ^= (_result << tempering_s) & tempering_b;
            _result ^= (_result << tempering_t) & tempering_c;
            _result ^= (_result >> tempering_l);

            return _result;
        }
    };

    /// @brief 32-bit MT19937 engine configuration.
    using mt19937_32 = mersenne_twister< 
        u32, 624, 397, 31, 0x9908b0df, 
        11, 0xffffffff, 7, 0x9d2c5680, 15,
        0xefc60000, 18, 1812433253 >; 	

    /// @brief 64-bit MT19937 engine configuration.
    using mt19937_64 = mersenne_twister< 
        u64, 312, 156, 31, 0xb5026f5aa96619e9, 29,
        0x5555555555555555, 17, 0x71d67fffeda60000, 37,
        0xfff7eee000000000, 43, 6364136223846793005 >;
} // namespace hsd
