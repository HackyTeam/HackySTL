#pragma once

/// @file Bitset.hpp
/// @brief Fixed-width bit storage and bitwise operations.
/// @details Supports whole-set and indexed mutation, counting, shifts,
/// constexpr construction, and checked access for a compile-time width.

#include "StackArray.hpp"
#include "../Serialize/_PrinterImpl.hpp"

namespace hsd
{
    /// @brief Compile-time-width bitset with checked indexed access.
    template <usize N>
    class bitset
    {
    protected:
        /// @brief Word type used for packed bit storage.
        using internal_type = u32;

        /// @brief Log2 of the number of bits in one storage word.
        static constexpr internal_type _div_val = 5;
        /// @brief Bit mask for an in-word position.
        static constexpr internal_type _mod_val = 31;
        /// @brief Number of bits in one storage word.
        static constexpr internal_type _size_val = 32;
        /// @brief Word value representing one set bit.
        static constexpr internal_type _one = 1;
        
        /// @brief Word with every bit set.
        static constexpr internal_type _set_val = static_cast<internal_type>(-1);
        /// @brief Number of valid bits in the leading storage word.
        static constexpr internal_type _len_mod = (N & _mod_val);
        /// @brief Number of storage words required for `N` bits.
        static constexpr internal_type _arr_sz = (N >> _div_val) + (_len_mod != 0);

        /// @brief Packed words, with unused leading bits masked after mutation.
        stack_array<internal_type, _arr_sz> _arr{};
        
        /// @brief Mask retaining only valid bits in the leading word.
        static constexpr internal_type _msb_val = 
            (_len_mod != 0) ? ((_one << _len_mod) - _one) : _set_val;

        template <usize M>
        friend class bitset;

    private:
        /// @brief Error returned for a checked access outside the bit width.
        struct bad_access
        {
            /// @brief Returns the diagnostic for an invalid bit index.
            /// @return A stable diagnostic string.
            constexpr const char* pretty_error() const
            {
                return "Tried to access an element out of bounds";
            }
        };

        /// @brief Proxy that maps one logical bit to its packed storage word.
        /// @tparam Val Mutable or const packed word type.
        template <typename Val>
        class bit_ref
        {
        private:
            /// @brief Pointer to the packed word array.
            Val* _data = nullptr;
            /// @brief Number of words in the packed array.
            usize _arr_size = 0;
            /// @brief Logical bit index represented by this proxy.
            usize _idx = 0;

            friend class bitset<N>;

            /// @brief Creates a proxy for one packed bit.
            /// @param data Packed word array.
            /// @param array_size Number of packed words.
            /// @param index Logical bit index.
            constexpr bit_ref(Val* data, usize array_size, usize index)
                : _data{data}, _arr_size{array_size}, _idx{index}
            {}

        public:
            constexpr bit_ref() = default;

            /// @brief Copies the proxy's storage address and bit index.
            constexpr bit_ref(const bit_ref&) = default;
            /// @brief Moves the proxy's storage address and bit index.
            constexpr bit_ref(bit_ref&&) = default;

            /// @brief Assigns the bit that `rhs` refers to to the bit this proxy refers to.
            /// @param rhs Proxy whose bit value is copied.
            /// @return `*this` after the assignment.
            /// @details Copying a proxy writes the bit; it does not rebind the proxy.
            constexpr bit_ref& operator=(const bit_ref& rhs)
            {
                return *this = static_cast<bool>(rhs);
            }

            /// @brief Assigns the bit that `rhs` refers to to the bit this proxy refers to.
            /// @param rhs Proxy whose bit value is copied.
            /// @return `*this` after the assignment.
            constexpr bit_ref& operator=(bit_ref&& rhs)
            {
                return *this = static_cast<bool>(rhs);
            }

            /// @brief Sets or clears the referenced bit.
            /// @param rhs New bit value.
            /// @return `*this` after mutation.
            constexpr bit_ref& operator=(bool rhs)
            requires (!hsd::IsConst<Val>)
            {
                if (rhs == true)
                {
                    _data[_arr_size - 1 - (_idx >> _div_val)] |= _one << (_idx & _mod_val);
                }
                else
                {
                    _data[_arr_size - 1 - (_idx >> _div_val)] &= ~(_one << (_idx & _mod_val));
                }

                return *this;
            }

            /// @brief Reads the referenced bit.
            /// @return True when the bit is set.
            constexpr operator bool() const
            {
                return ((_data[_arr_size - 1 - (_idx >> _div_val)] >> (_idx & _mod_val)) & _one);
            }

            /// @brief Reads the inverse of the referenced bit.
            /// @return True when the bit is clear.
            constexpr bool operator~() const
            {
                return !static_cast<bool>(*this);
            }

            /// @brief Inverts the referenced bit.
            /// @return `*this` after mutation.
            constexpr bit_ref& flip()
            requires (!hsd::IsConst<Val>)
            {
                return (*this = ~(*this));
            }
        };

    public:
        /// @brief Constructs an all-clear bitset.
        constexpr bitset() = default;

        /// @brief Copies packed bit storage.
        constexpr bitset(const bitset&) = default;
        /// @brief Moves packed bit storage.
        constexpr bitset(bitset&&) = default;

        /// @brief Copies packed bit storage into this set.
        /// @return `*this` after copying.
        constexpr bitset& operator=(const bitset&) = default;
        /// @brief Moves packed bit storage into this set.
        /// @return `*this` after moving.
        constexpr bitset& operator=(bitset&&) = default;
        
        /// @brief Constructs a bitset from the low `N` bits of `other`.
        /// @param other Integer bits copied into the set.
        constexpr bitset(u64 other)
        {
            _arr[_arr.size() - 1] = other & 0xFFFF'FFFF;
            
            if  (_arr.size() >= 2)
            {
                _arr[_arr.size() - 2] = (other >> 32) & 0xFFFF'FFFF;
            } 

            _arr[0] &= _msb_val;
        }
        /// @brief Returns a mutable unchecked bit proxy.
        /// @param index Logical bit index.
        /// @return Proxy that reads or mutates the selected bit.
        constexpr bit_ref<internal_type> operator[](usize index)
        {
            return {_arr.data(), _arr.size(), index};
        }

        /// @brief Returns a const unchecked bit proxy.
        /// @param index Logical bit index.
        /// @return Proxy that reads the selected bit.
        constexpr bit_ref<const internal_type> operator[](usize index) const
        {
            return {_arr.data(), _arr.size(), index};
        }

        /// @brief Returns the compile-time bit width.
        /// @return Number of logical bits in the set.
        constexpr usize size() const
        {
            return N;
        }

        /// @brief Reads one bit with bounds checking.
        /// @param index Logical bit index.
        /// @return Bit value, or `bad_access` when `index >= N`.
        constexpr auto test(usize index) const
            -> result<bool, bad_access>
        {
            if (index >= N)
            {
                return bad_access{};
            }

            return static_cast<bool>(bit_ref<const internal_type>{_arr.data(), _arr.size(), index});
        }

        /// @brief Tests whether at least one bit is set.
        /// @return True when any logical bit is set.
        constexpr bool any() const
        {
            for (internal_type _val : _arr)
            {
                if (_val != 0)
                    return true;
            }

            return false;
        }

        /// @brief Tests whether all logical bits are clear.
        /// @return True when no logical bit is set.
        constexpr bool none() const
        {
            return !any();
        }

        /// @brief Tests whether every logical bit is set.
        /// @return True when all `N` bits are set.
        constexpr bool all() const
        {
            usize _idx = 1;
            
            if (_arr[0] != _msb_val)
                return false;

            for (; _idx != _arr.size(); ++_idx)
            {
                if (_arr[_idx] != _set_val)
                    return false;
            }

            return true;
        }

        /// @brief Counts set logical bits.
        /// @return Number of set bits.
        constexpr usize count() const
        {
            usize _cnt = 0;

            for (auto _val : _arr)
            {
                #if defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
                _cnt += __builtin_popcount(_val);
                #else
                // Kernighan's loop: each pass clears the lowest set bit.
                for (auto _bits = _val; _bits != 0; _bits &= _bits - 1)
                {
                    ++_cnt;
                }
                #endif
            }
            
            return _cnt;
        }

        /// @brief Clears every bit.
        /// @return `*this` after clearing.
        constexpr bitset& reset()
        {
            _arr.fill(0);
            return *this;
        }

        /// @brief Clears one bit without bounds checking.
        /// @param index Logical bit index.
        /// @return `*this` after clearing.
        constexpr bitset& reset(usize index)
        {
            (*this)[index] = 0;
            return *this;
        }

        /// @brief Sets every logical bit.
        /// @return `*this` after setting.
        constexpr bitset& set()
        {
            usize _idx = 1;
            _arr[0] = _msb_val;

            for (; _idx != _arr.size(); ++_idx)
            {
                _arr[_idx] = _set_val;
            }

            return *this;
        }

        /// @brief Inverts every logical bit.
        /// @return `*this` after inversion.
        constexpr bitset& flip()
        {
            usize _idx = 1;
            _arr[0] = (~_arr[0]) & _msb_val;

            for (; _idx != _arr.size(); ++_idx)
            {
                _arr[_idx] = ~_arr[_idx];
            }

            return *this;
        }

        /// @brief Inverts one bit without bounds checking.
        /// @param index Logical bit index.
        /// @return `*this` after inversion.
        constexpr bitset& flip(usize index)
        {
            (*this)[index].flip();
            return *this;
        }

        /// @brief Sets or clears one bit with bounds checking.
        /// @param index Logical bit index.
        /// @param value New bit value.
        /// @return Reference to this bitset, or `bad_access` when out of range.
        constexpr auto set(usize index, bool value = true)
            -> result<reference<bitset>, bad_access>
        {
            if (index >= N)
            {
                return bad_access{};
            }

            (*this)[index] = value;
            return {*this};
        }

        /// @brief Exposes the packed storage for formatting and low-level use.
        /// @return Const reference to the packed word array.
        constexpr const auto& get_underlying_array() const
        {
            return _arr;
        }

        /// @brief Returns the number of packed storage words.
        /// @return Packed array size.
        constexpr auto get_underlying_array_size() const
        {
            return _arr.size();
        }

        /// @brief Tests packed bit equality.
        /// @param rhs Bitset compared with this set.
        /// @return True when all logical bits match.
        constexpr bool operator==(const bitset& rhs) const
        {
            for (usize _idx = 0; _idx < _arr.size(); ++_idx)
            {
                if (_arr[_idx] != rhs._arr[_idx])
                    return false;
            }

            return true;
        }

        /// @brief Tests packed bit inequality.
        /// @param rhs Bitset compared with this set.
        /// @return True when any logical bit differs.
        constexpr bool operator!=(const bitset& rhs) const
        {
            return !(*this == rhs);
        }

        /// @brief Applies bitwise AND in place.
        /// @param rhs Bitset mask.
        /// @return `*this` after the operation.
        constexpr bitset& operator&=(const bitset& rhs)
        {
            for (usize _idx = 0; _idx < _arr.size(); ++_idx)
            {
                _arr[_idx] &= rhs._arr[_idx];
            }

            return *this;
        }

        /// @brief Applies bitwise OR in place.
        /// @param rhs Bitset mask.
        /// @return `*this` after the operation.
        constexpr bitset& operator|=(const bitset& rhs)
        {
            for (usize _idx = 0; _idx < _arr.size(); ++_idx)
            {
                _arr[_idx] |= rhs._arr[_idx];
            }

            return *this;
        }

        /// @brief Applies bitwise XOR in place.
        /// @param rhs Bitset mask.
        /// @return `*this` after the operation.
        constexpr bitset& operator^=(const bitset& rhs)
        {
            for (usize _idx = 0; _idx < _arr.size(); ++_idx)
            {
                _arr[_idx] ^= rhs._arr[_idx];
            }

            return *this;
        }

        /// @brief Returns the bitwise AND of two sets.
        /// @param rhs Bitset mask.
        /// @return New bitset containing the intersection.
        constexpr bitset operator&(const bitset& rhs) const 
        {
            return bitset<N>{*this} &= rhs;
        }

        /// @brief Returns the bitwise OR of two sets.
        /// @param rhs Bitset mask.
        /// @return New bitset containing the union.
        constexpr bitset operator|(const bitset& rhs) const
        {
            return bitset<N>{*this} |= rhs;
        }

        /// @brief Returns the bitwise XOR of two sets.
        /// @param rhs Bitset mask.
        /// @return New bitset containing differing bits.
        constexpr bitset operator^(const bitset& rhs) const
        {
            return bitset<N>{*this} ^= rhs;
        }

        /// @brief Shifts logical bits left and clears bits shifted out.
        /// @param pos Number of bit positions to shift.
        /// @return `*this` after the shift.
        constexpr bitset& operator<<=(usize pos)
        {
            if (pos >= N)
            {
                return reset();
            }
            else if (pos == 0)
            {
                return *this;
            }

            // assume 9, _to_idx = 1; _rest_idx = 1

            //                0        1        2
            // bitset<20> 0000bbbb bbbbbbbb bbbbbbbb

            usize _to_pos = pos >> _div_val;
            usize _rest_pos = pos & _mod_val;
            
            if (_to_pos != 0)
            {
                for (usize _idx = 0; _idx < _arr.size() - _to_pos; ++_idx)
                {
                    _arr[_idx] = _arr[_idx + _to_pos];
                }

                for (usize _idx = _arr.size() - _to_pos; _idx < _arr.size(); ++_idx)
                {
                    _arr[_idx] = 0;
                }
            }

            if (_rest_pos != 0)
            {
                internal_type _prev_carry = 0;
                internal_type _carry = 0;

                for (usize _idx = _arr.size() - _to_pos; _idx > 0; --_idx)
                {
                    const internal_type _shf_val = 
                        ((_one << _rest_pos) - _one) << (_size_val - _rest_pos);

                    _carry = (_shf_val & _arr[_idx - 1]) >> (_size_val - _rest_pos);
                    _arr[_idx - 1] <<= _rest_pos;
                    _arr[_idx - 1] |= _prev_carry;
                    _prev_carry = _carry;
                }
            }

            _arr[0] &= _msb_val;
            return *this;
        }

        /// @brief Shifts logical bits right and clears bits shifted out.
        /// @param pos Number of bit positions to shift.
        /// @return `*this` after the shift.
        constexpr bitset& operator>>=(usize pos)
        {
            if (pos >= N)
            {
                return reset();
            }
            else if (pos == 0)
            {
                return *this;
            }

            // assume 9, _to_idx = 1; _rest_idx = 1 

            //                0        1        2
            // bitset<20> 0000bbbb bbbbbbbb bbbbbbbb

            usize _to_pos = pos >> _div_val;
            usize _rest_pos = pos & _mod_val;
            
            if (_to_pos != 0)
            {
                for (usize _idx = _arr.size(); _idx > _to_pos; --_idx)
                {
                    _arr[_idx - 1] = _arr[_idx - _to_pos - 1];
                }

                _to_pos = (_arr.size() > _to_pos) ? _to_pos : _arr.size();

                for (usize _idx = 0; _idx < _to_pos; ++_idx)
                {
                    _arr[_idx] = 0;
                }
            }

            if (_rest_pos != 0)
            {
                internal_type _prev_carry = 0;
                internal_type _carry = 0;

                for (usize _idx = _to_pos; _idx < _arr.size(); ++_idx)
                {
                    const internal_type _shf_val = (_one << _rest_pos) - _one;

                    _carry = (_shf_val & _arr[_idx]) << (_size_val - _rest_pos);
                    _arr[_idx] >>= _rest_pos;
                    _arr[_idx] |= _prev_carry;
                    _prev_carry = _carry;
                }
            }

            return *this;
        }

        /// @brief Returns a left-shifted copy.
        /// @param pos Number of bit positions to shift.
        /// @return Shifted bitset.
        constexpr bitset operator<<(usize pos) const
        {
            return bitset<N>{*this} <<= pos;
        }

        /// @brief Returns a right-shifted copy.
        /// @param pos Number of bit positions to shift.
        /// @return Shifted bitset.
        constexpr bitset operator>>(usize pos) const
        {
            return bitset<N>{*this} >>= pos;
        }

        /// @brief Returns a copy with every logical bit inverted.
        /// @return Inverted bitset.
        constexpr bitset operator~() const
        {
            return bitset<N>{*this}.flip();
        }

        /// @brief Converts to another compile-time bit width, preserving low bits.
        /// @tparam M Destination bit width.
        /// @return Converted bitset with truncation or zero-extension as needed.
        template <usize M>
        constexpr operator bitset<M>() const
        {
            bitset<M> _new_val;

            usize _min_sz = (_arr.size() < _new_val._arr.size()) ? _arr.size() : _new_val._arr.size();

            for (usize _idx = 1; _idx < _min_sz + 1; ++_idx)
            {
                _new_val._arr[_new_val._arr.size() - _idx] = _arr[_arr.size() - _idx];
            }

            _new_val._arr[0] &= _new_val._msb_val;
            return _new_val;
        }
    };

    /// @brief Formats a bitset as binary or hexadecimal text.
    /// @param str Destination character buffer.
    /// @param sz Destination capacity in characters.
    /// @param val Bitset to format.
    /// @return Number of written characters, or a runtime error for insufficient capacity.
    /// @details Only accepts a tag that `integral_tag_ok` allows: none, or hexadecimal.
    template <format_output fmt, typename CharT, usize N>
    requires (IsSame<fmt_char_type<decltype(fmt)>, CharT> && integral_tag_ok<fmt>())
    static constexpr result<usize, runtime_error> print_to(CharT* str, usize sz, const bitset<N>& val)
    {
        using FmtType = decltype(fmt);

        auto _arr = [&val]
        {
            if constexpr (fmt.tag == FmtType::none)
            {
                to_string_detail::to_string_value_for<CharT, N> _buf;

                for (usize _idx = N; _idx > 0; --_idx)
                {
                    _buf.push(static_cast<CharT>((val[_idx - 1]) ? '1' : '0'));
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
} // namespace hsd