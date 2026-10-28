/// @file Integer.hpp
/// @brief Fixed-width arbitrary-size integer arithmetic.
/// @details Implements signed integer storage, comparison, arithmetic,
/// division with remainder, bit access, and formatting for compile-time widths.

#include "Bitset.hpp"

namespace hsd
{
    template <usize N>
    /// @brief Fixed-width integer arithmetic built on bit storage.
    class integer : public bitset<N>
    {
    private:
        /// @brief Error returned by `divide_with_reminder` for a zero divisor.
        struct division_error
        {
            /// @brief Gets a human-readable description of this error.
            /// @return A stable diagnostic string.
            constexpr const char* pretty_error() const
            {
                return "Tried to divide by 0";
            }
        };

        /// @brief Quotient/remainder pair produced by `divide_with_reminder`.
        struct division_result
        {
            /// @brief Result of the division.
            integer quotient;
            /// @brief Remainder left over after the division.
            integer reminder;
        };

        /// @brief The underlying fixed-width bit storage type.
        using base_type = bitset<N>;

        template <usize M>
        friend class integer;

        /// @brief Splits the underlying words into high/low halves for Karatsuba.
        /// @param mid - Word index at which to split, clamped to the array size.
        /// @return A pair of integers built from the words before/after `mid`.
        constexpr auto _split_number(usize mid) const
        {
            using RetType = pair<integer<N>, integer<N>>;
            integer<N> _lhs, _rhs;

            if (mid > this->_arr.size())
            {
                mid = this->_arr.size(); // Clamp mid to the size of _arr
            }

            for (usize _idx = 0; _idx < mid; ++_idx)
            {
                _lhs <<= 32;
                _lhs |= this->_arr[_idx];
            }

            for (usize _idx = mid; _idx < this->_arr.size(); ++_idx)
            {
                _rhs <<= 32;
                _rhs |= this->_arr[_idx];
            }

            return RetType{_lhs, _rhs};
        }

        /// @brief Multiplies two integers using the Karatsuba algorithm.
        /// @details Falls back to word-by-word multiplication (via
        /// `operator*(u32)`) once either operand fits in 32 bits, otherwise
        /// splits both operands and recombines the three sub-products.
        /// @param lhs - Left multiplicand.
        /// @param rhs - Right multiplicand.
        /// @return The product `lhs * rhs`.
        static constexpr auto _karatusba_mult(const integer<N>& lhs, const integer<N>& rhs)
        {
            using IntegerType = integer<N>;

            constexpr IntegerType _max_num = 0xFFFF'FFFF;

            if (lhs <= _max_num)
            {
                return rhs * lhs.get_last_byte();
            }
            else if (rhs <= _max_num)
            {
                return lhs * rhs.get_last_byte();
            }

            const auto _rhs_size = rhs.num_size();
            const auto _lhs_size = lhs.num_size();

            const auto _max = (_rhs_size > _lhs_size) ? _rhs_size : _lhs_size;
            const auto _mid = (_max + 1) / 2;

            const auto [_high_lhs, _low_lhs] = lhs._split_number(lhs.get_underlying_array_size() - _mid);
            const auto [_high_rhs, _low_rhs] = rhs._split_number(rhs.get_underlying_array_size() - _mid);

            const auto _z1 = _karatusba_mult(_high_lhs, _high_rhs);
            const auto _z2 = _karatusba_mult(_low_lhs, _low_rhs);
            const auto _z3 = _karatusba_mult(_low_lhs + _high_lhs, _low_rhs + _high_rhs);

            return IntegerType{(IntegerType{_z1 << (32 * _mid)} + (_z3 - _z1 - _z2)) << (32 * _mid)} + _z2;
        }

        /// @brief Returns the high 64 bits of a 64-by-64-bit product.
        static constexpr u64 _multiply_high(u64 lhs, u64 rhs)
        {
            #if defined(__SIZEOF_INT128__)
            using wide_type = unsigned __int128;
            return static_cast<u64>(
                (static_cast<wide_type>(lhs) * rhs) >> 64
            );
            #else
            const u64 lhs_low = static_cast<u32>(lhs);
            const u64 lhs_high = lhs >> 32;
            const u64 rhs_low = static_cast<u32>(rhs);
            const u64 rhs_high = rhs >> 32;

            const u64 low_product = lhs_low * rhs_low;
            const u64 cross_product_a = lhs_low * rhs_high;
            const u64 cross_product_b = lhs_high * rhs_low;
            const u64 high_product = lhs_high * rhs_high;
            const u64 middle = (low_product >> 32) +
                static_cast<u32>(cross_product_a) +
                static_cast<u32>(cross_product_b);

            return high_product + (cross_product_a >> 32) +
                (cross_product_b >> 32) + (middle >> 32);
            #endif
        }

    public:
        constexpr integer() = default;
        
        /// @brief Copies bit storage from a `bitset`.
        /// @param other - Bitset copied into this integer.
        constexpr integer(const bitset<N>& other)
            : bitset<N>{other}
        {}

        /// @brief Moves bit storage from a `bitset`.
        /// @param other - Bitset moved into this integer.
        constexpr integer(bitset<N>&& other)
            : bitset<N>{hsd::forward<bitset<N>>(other)}
        {}

        /// @brief Constructs from a 64-bit unsigned value.
        /// @param _value - Value stored in the low-order bits.
        constexpr integer(u64 _value)
            : bitset<N>{_value}
        {}

        /// @brief Views this integer as its underlying `bitset`.
        /// @return A const reference to the underlying bit storage.
        constexpr const bitset<N>& to_bitset() const
        {
            return *this;
        }

        /// @brief Gets the last (least-significant) storage word.
        /// @return The trailing 32-bit word of the underlying array.
        constexpr auto get_last_byte() const
        {
            return this->_arr[this->_arr.size() - 1];
        }

        /// @brief Counts the storage words needed to represent the value.
        /// @return The number of words from the first non-zero word to the
        /// end of the array, or `0` when the value is zero.
        constexpr usize num_size() const
        {
            for (usize _idx = 0; _idx < this->_arr.size(); ++_idx)
            {
                if (this->_arr[_idx] != 0)
                {
                    return this->_arr.size() - _idx;
                }
            }

            return 0;
        }

        /// @brief Replaces the bit storage by copying from a `bitset`.
        /// @param other - Bitset copied into this integer.
        /// @return A reference to this integer.
        constexpr integer& operator=(const bitset<N>& other)
        {
            static_cast<bitset<N>&>(*this) = other;
            return *this;
        }

        /// @brief Replaces the bit storage by moving from a `bitset`.
        /// @param other - Bitset moved into this integer.
        /// @return A reference to this integer.
        constexpr integer& operator=(bitset<N>&& other)
        {
            static_cast<bitset<N>&>(*this) = hsd::forward<bitset<N>>(other);
            return *this;
        }

        /// @brief Adds `rhs` into this integer, wrapping on overflow.
        /// @param rhs - Value to add.
        /// @return A reference to this integer.
        constexpr integer& operator+=(const integer& rhs)
        {
            u64 _val = 0;

            for (usize _idx = this->_arr.size(); _idx > 0; --_idx)
            {
                _val = (_val >> this->_size_val);
                _val += this->_arr[_idx - 1];
                _val += rhs._arr[_idx - 1];
                this->_arr[_idx - 1] = static_cast<u32>(_val);
            }

            this->_arr[0] &= this->_msb_val;
            return *this;
        } 

        /// @brief Adds two integers, wrapping on overflow.
        /// @param rhs - Value to add.
        /// @return The sum `*this + rhs`.
        constexpr integer operator+(const integer& rhs) const
        {
            return integer{*this} += rhs;
        } 

        /// @brief Subtracts `rhs` from this integer.
        /// @details When `*this < rhs`, the result wraps around using the
        /// full modulus (computed via a recursive complement), matching
        /// unsigned fixed-width subtraction semantics.
        /// @param rhs - Value to subtract.
        /// @return A reference to this integer.
        constexpr integer& operator-=(const integer& rhs)
        {
            if (*this < rhs)
            {
                integer _new_val = rhs - *this - 1;
                this->set();

                return *this -= _new_val;
            }

            bool _overflow = false;

            for (usize _idx = this->_arr.size(); _idx > 0; --_idx)
            {
                if (this->_arr[_idx - 1] < (rhs._arr[_idx - 1]) && _overflow == false)
                {
                    _overflow = true;
                    this->_arr[_idx - 1] -= rhs._arr[_idx - 1];
                }
                else
                {
                    u64 _this_num = this->_arr[_idx - 1];
                    u64 _rhs_num = rhs._arr[_idx - 1];

                    bool _will_overflow = _this_num < (_rhs_num + _overflow);
                    this->_arr[_idx - 1] -= rhs._arr[_idx - 1];
                    this->_arr[_idx - 1] -= _overflow;
                    _overflow = _will_overflow;
                }
            }

            this->_arr[0] &= this->_msb_val;
            return *this;
        } 

        /// @brief Subtracts two integers.
        /// @param rhs - Value to subtract.
        /// @return The difference `*this - rhs`.
        constexpr integer operator-(const integer& rhs) const
        {
            return integer{*this} -= rhs;
        }

        /// @brief Multiplies this integer by a 32-bit word, wrapping on overflow.
        /// @param rhs - Word multiplier.
        /// @return A reference to this integer.
        constexpr integer& operator*=(u32 rhs)
        {
            u64 _val = 0, _carry = 0;

            for (usize _idx = this->_arr.size(); _idx > 0; --_idx)
            {
                _val = this->_arr[_idx - 1];
                _val *= rhs;
                _val += _carry;

                _carry = (_val >> this->_size_val);
                this->_arr[_idx - 1] = static_cast<u32>(_val);
            }

            this->_arr[0] &= this->_msb_val;
            return *this;
        }

        /// @brief Multiplies an integer by a 32-bit word, wrapping on overflow.
        /// @param rhs - Word multiplier.
        /// @return The product `*this * rhs`.
        constexpr integer operator*(u32 rhs) const
        {
            return integer{*this} *= rhs;
        }

        /// @brief Multiplies this integer by another integer (Karatsuba).
        /// @param rhs - Value to multiply by.
        /// @return A reference to this integer.
        constexpr integer& operator*=(const integer& rhs)
        {
            *this = _karatusba_mult(*this, rhs);
            return *this;
        } 

        /// @brief Multiplies two integers (Karatsuba).
        /// @param rhs - Value to multiply by.
        /// @return The product `*this * rhs`.
        constexpr integer operator*(const integer& rhs) const
        {
            return _karatusba_mult(*this, rhs);
        } 

        /// @brief Compares two integers as unsigned magnitudes, word by word.
        /// @param rhs - Value to compare against.
        /// @return `-1` if less than `rhs`, `1` if greater, `0` if equal.
        constexpr i32 operator<=>(const integer& rhs) const
        {
            for (usize _idx = 0; _idx < rhs._arr.size(); ++_idx)
            {
                if (this->_arr[_idx] < rhs._arr[_idx])
                {
                    return -1;
                }
                else if (this->_arr[_idx] > rhs._arr[_idx])
                {
                    return 1;
                }
            }

            return 0;
        }

        /// @brief Tests two integers for equality.
        /// @param rhs - Value to compare against.
        /// @return `true` when `*this` and `rhs` compare equal.
        constexpr bool operator==(const integer& rhs) const
        {
            return operator<=>(rhs) == 0;
        }

        /// @brief Divides by a compile-time constant using reciprocal multiplication.
        /// @details Precomputes a fixed-point reciprocal of `b` (widened to
        /// `2 * N` bits) once per instantiation, then divides by multiplying
        /// and shifting instead of performing a full bitwise division.
        /// @tparam b - Compile-time divisor.
        /// @return A pair of quotient and remainder.
        template <usize b>
        constexpr auto divide_by_constant()
        {
            using WideInteger = integer<N * 2>;
            using Integer = integer<N>;
        
            constexpr auto mul_shft = []{
                usize k = 0, l = 1;
            
                while (l <= b / 2)
                {
                    l <<= 1;
                    ++k;
                }
            
                WideInteger res = 1;

                res <<= N + k;
                res /= WideInteger{b};
                res += 1;

                return pair<Integer, usize>{res, k};
            }();
        
            pair<Integer, Integer> _res;
            WideInteger _wide_num = *this;
        
            _wide_num *= mul_shft.first;
            _wide_num >>= (N + mul_shft.second);
        
            _res.first = Integer{_wide_num};
            _res.second = *this - (_res.first * Integer{b});
        
            return _res;
        }

        /// @brief Performs bitwise long division, computing quotient and remainder.
        /// @param rhs - Divisor.
        /// @return The quotient/remainder pair, or `division_error` when
        /// `rhs` is zero.
        constexpr auto divide_with_reminder(const integer& rhs) const
            -> result<division_result, division_error>
        {
            if (rhs.none() == true)
            {
                return division_error{};
            }

            division_result _result{};

            if (rhs.num_size() <= 1)
            {
                const u32 _divisor = rhs.get_last_byte();
                if (_divisor == 1)
                {
                    _result.quotient = *this;
                    return {hsd::move(_result)};
                }

                u64 _reciprocal = 0;
                if constexpr (N > 256)
                {
                    constexpr u64 _max_u64 = ~static_cast<u64>(0);
                    _reciprocal = _max_u64 / _divisor;
                    if (_max_u64 % _divisor == _divisor - 1)
                    {
                        ++_reciprocal;
                    }
                }

                u64 _remainder = 0;

                for (usize _idx = 0; _idx < this->_arr.size(); ++_idx)
                {
                    const u64 _current =
                        (_remainder << this->_size_val) | this->_arr[_idx];
                    u64 _quotient_word;
                    if constexpr (N > 256)
                    {
                        _quotient_word = _multiply_high(_current, _reciprocal);
                        _remainder = _current - _quotient_word * _divisor;
                        if (_remainder >= _divisor)
                        {
                            ++_quotient_word;
                            _remainder -= _divisor;
                        }
                    }
                    else
                    {
                        _quotient_word = _current / _divisor;
                        _remainder = _current % _divisor;
                    }
                    _result.quotient._arr[_idx] =
                        static_cast<u32>(_quotient_word);
                }

                _result.quotient._arr[0] &= this->_msb_val;
                _result.reminder._arr[this->_arr.size() - 1] =
                    static_cast<u32>(_remainder);
                return {hsd::move(_result)};
            }

            for (usize _idx = N; _idx > 0; --_idx)
            {
                _result.reminder <<= 1;
                _result.quotient <<= 1;
                _result.reminder.set(0, this->test(_idx - 1).unwrap()).unwrap();

                if (_result.reminder >= rhs)
                {
                    _result.reminder -= rhs;
                    _result.quotient.set(0).unwrap();
                }
            }

            return {hsd::move(_result)};
        }

        /// @brief Divides two integers.
        /// @param rhs - Divisor.
        /// @return The quotient `*this / rhs`. Aborts (via `unwrap`) when
        /// `rhs` is zero.
        constexpr integer operator/(const integer& rhs) const
        {
            return divide_with_reminder(rhs).unwrap().quotient;
        }

        /// @brief Divides this integer by `rhs` in place.
        /// @param rhs - Divisor.
        /// @return A reference to this integer.
        constexpr integer& operator/=(const integer& rhs)
        {
            *this = hsd::move(*this / rhs);
            return *this;
        }

        /// @brief Computes the remainder of dividing two integers.
        /// @param rhs - Divisor.
        /// @return The remainder `*this % rhs`. Aborts (via `unwrap`) when
        /// `rhs` is zero.
        constexpr integer operator%(const integer& rhs) const
        {
            return divide_with_reminder(rhs).unwrap().reminder;
        }

        /// @brief Replaces this integer with the remainder of dividing by `rhs`.
        /// @param rhs - Divisor.
        /// @return A reference to this integer.
        constexpr integer& operator%=(const integer& rhs)
        {
            *this = hsd::move(*this % rhs);
            return *this;
        }

        /// @brief Converts to another compile-time bit width, preserving low bits.
        /// @tparam M - Destination bit width.
        /// @return Converted integer with truncation or zero-extension as needed.
        template <usize M>
        constexpr operator integer<M>() const
        {
            integer<M> _new_val;

            static_cast<bitset<M>&>(_new_val) = static_cast<const bitset<N>&>(*this);

            return _new_val;
        }

        /// @brief Pre-increments this integer by one, wrapping on overflow.
        /// @return A reference to this integer.
        constexpr integer& operator++()
        {
            *this += 1;
            return *this;
        }

        /// @brief Post-increments this integer by one, wrapping on overflow.
        /// @return A copy of the value before incrementing.
        constexpr integer operator++(i32)
        {
            integer _copy = *this;
            *this += 1;
            return _copy;
        }

        /// @brief Pre-decrements this integer by one, wrapping on underflow.
        /// @return A reference to this integer.
        constexpr integer& operator--()
        {
            *this -= 1;
            return *this;
        }

        /// @brief Post-decrements this integer by one, wrapping on underflow.
        /// @return A copy of the value before decrementing.
        constexpr integer operator--(i32)
        {
            integer _copy = *this;
            *this -= 1;
            return _copy;
        }
    };

    /// @brief Formats an integer as decimal or hexadecimal text.
    /// @param str - Destination character buffer.
    /// @param sz - Destination capacity in characters.
    /// @param val - Integer to format.
    /// @return Number of written characters, or a runtime error for insufficient capacity.
    /// @details Only accepts a tag that `integral_tag_ok` allows: none, or hexadecimal.
    template <format_output fmt, typename CharT, usize N>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && integral_tag_ok<fmt>())
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const integer<N>& val)
    {
        using FmtType = decltype(fmt);

        auto _arr = [&val]
        {
            if constexpr (fmt.tag == FmtType::none)
            {
                auto _copy = val;
                constexpr auto _arr_sz = _copy.get_underlying_array_size();

                constexpr usize _buf_len = static_cast<usize>(1 + N * math::log10(2.));
                to_string_detail::to_string_value_rev<CharT, _buf_len> _buf;

                if (val.none() == true)
                {
                    _buf.push('0');
                    return _buf;
                }

                if constexpr (N > (sizeof(u64) * 8))
                {
                    constexpr integer<N> _div_constant = 10'000'000'000'000'000'000ULL;
                    constexpr integer<N> _cmp_num = 0xFFFF'FFFF'FFFF'FFFFULL;

                    while (_copy > _cmp_num)
                    {
                        const auto [_new_val, _reminder] = _copy.divide_with_reminder(_div_constant).unwrap();

                        constexpr auto _arr_sz = _reminder.get_underlying_array_size();
                        const auto& _internal_arr = _reminder.get_underlying_array();

                        u64 _chunk = _internal_arr[_arr_sz - 2];
                        
                        _chunk <<= 32;
                        _chunk |= _internal_arr[_arr_sz - 1];
                        _copy = hsd::move(_new_val);

                        for (usize _idx = 0; _idx < 19; ++_idx)
                        {
                            _buf.push(static_cast<CharT>('0') + (_chunk % 10));
                            _chunk /= 10;
                        }
                    }
                }

                u64 _chunk = 0;

                if constexpr (_arr_sz >= 2)
                {
                    const auto& _internal_arr = _copy.get_underlying_array();
                    _chunk = _internal_arr[_arr_sz - 2];
                
                    _chunk <<= 32;
                    _chunk |= _internal_arr[_arr_sz - 1];
                }
                else
                {
                    _chunk = _copy.get_last_byte();   
                }

                while (_chunk != 0)
                {
                    _buf.push(static_cast<CharT>('0') + (_chunk % 10));
                    _chunk /= 10;
                }

                return _buf;
            }
            else
            {
                const auto& _copy = val.get_underlying_array();
                using CopyType = remove_cvref_t<decltype(_copy)>;

                to_string_detail::to_string_value_for<CharT, CopyType::size() * 8 + 2> _buf;

                constexpr const CharT _hex_table[16] = {
                    '0', '1', '2', '3', '4', '5', '6', '7', 
                    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
                };

                _buf.push('0');
                _buf.push('x');

                for (usize _idx = 0; _idx < _copy.size(); ++_idx)
                {
                    _buf.push(_hex_table[(_copy[_idx] >> 28)]);
                    _buf.push(_hex_table[(_copy[_idx] >> 24) & 15]);
                    _buf.push(_hex_table[(_copy[_idx] >> 20) & 15]);
                    _buf.push(_hex_table[(_copy[_idx] >> 16) & 15]);
                    _buf.push(_hex_table[(_copy[_idx] >> 12) & 15]);
                    _buf.push(_hex_table[(_copy[_idx] >> 8) & 15]);
                    _buf.push(_hex_table[(_copy[_idx] >> 4) & 15]);
                    _buf.push(_hex_table[_copy[_idx] & 15]);
                }

                return _buf;
            }
        }();

        usize _write_sz = (_arr.size() + fmt.format.size() + fmt.style.size() + fmt.reset.size() - 3);

        // The terminator written after the text takes one position more than its characters.
        if (_write_sz + 1 > sz)
        {
            return runtime_error{"Buffer too small for printing"};
        }

        str = _print_to_impl(str, fmt.format);
        str = _print_to_impl(str, fmt.style);
        str = _print_to_impl(str, _arr) + 1;
        str = _print_to_impl(str, fmt.reset);

        return _write_sz;
    }
}