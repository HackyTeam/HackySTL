#pragma once

/// @file VarTuple.hpp
/// @brief Supporting heterogeneous tuple storage machinery.
/// @details Supplies internal tuple-like structures used by formatting,
/// argument expansion, and other compile-time heterogeneous operations.

#include "Vector.hpp"
#include <typeinfo>

namespace hsd
{
    /// @brief Move-only byte-buffer tuple representation for heterogeneous values.
    /// @details The current type exposes storage and iterator machinery for
    /// future variable-layout tuple operations; it does not provide public
    /// insertion or extraction operations.
    class variable_tuple
    {
    private:
        /// @brief Header and flexible payload describing one stored value.
        struct value_info
        {
            struct {
                /// @brief Bytes occupied by the preceding value record.
                usize _value_size_back;
                /// @brief Bytes occupied by the following value record.
                usize _value_size_front;
                /// @brief Runtime type descriptor for the payload.
                std::type_info _info;
            } _type_info; ///< Layout metadata for the variable-size record.
            /// @brief Flexible payload bytes following the metadata header.
            char _data[];
        };

        /// @brief Bidirectional iterator over variable-size value records.
        class value_iter
        {
        private:
            /// @brief Current record address in the byte buffer.
            char* _iter;
        
        public:
            /// @brief Constructs an iterator at a record address.
            /// @param iter Byte-buffer address of the current record.
            constexpr value_iter(char* iter)
                : _iter{iter}
            {}

            /// @brief Advances to the following variable-size record.
            /// @return `*this` after advancing.
            constexpr auto& operator++()
            {
                auto& _val = *reinterpret_cast<value_info*>(_iter);
                _iter += sizeof(value_info) + _val._type_info._value_size_front;
                return *this;
            }

            /// @brief Returns the current iterator and then advances it.
            /// @return Iterator value before advancing.
            constexpr auto operator++(i32)
            {
                auto tmp = *this;
                operator++();
                return tmp;
            }

            /// @brief Moves to the preceding variable-size record.
            /// @return `*this` after retreating.
            constexpr auto& operator--()
            {
                auto& _val = *reinterpret_cast<value_info*>(_iter);
                _iter -= sizeof(value_info) + _val._type_info._value_size_back;
                return *this;
            }

            /// @brief Returns the current iterator and then retreats it.
            /// @return Iterator value before retreating.
            constexpr auto operator--(i32)
            {
                auto tmp = *this;
                operator--();
                return tmp;
            }

            /// @brief Compares two record positions.
            /// @param rhs Iterator compared with this iterator.
            /// @return True when both point at the same byte address.
            constexpr bool operator==(const value_iter& rhs) const
            {
                return _iter == rhs._iter;
            }

            /// @brief Tests whether two record positions differ.
            /// @param rhs Iterator compared with this iterator.
            /// @return True when byte addresses differ.
            constexpr bool operator!=(const value_iter& rhs) const
            {
                return _iter != rhs._iter;
            }

            /// @brief Returns mutable access to the current value header.
            /// @return Current value metadata.
            constexpr auto& operator*()
            {
                auto* _val = reinterpret_cast<value_info*>(_iter);
                return *_val;
            }
            
            /// @brief Returns const access to the current value header.
            /// @return Current value metadata.
            constexpr const auto& operator*() const
            {
                auto* _val = reinterpret_cast<const value_info*>(_iter);
                return *_val;
            }
        };

        /// @brief Raw byte storage containing variable-size value records.
        vector<char> _buffer{};

    public:
        /// @brief Constructs an empty variable-layout tuple buffer.
        constexpr variable_tuple() = default;
        constexpr variable_tuple(const variable_tuple&) = delete;
        /// @brief Transfers the raw tuple buffer.
        constexpr variable_tuple(variable_tuple&&) = default;
        constexpr variable_tuple& operator=(const variable_tuple&) = delete;
        /// @brief Transfers the raw tuple buffer into this object.
        /// @brief Transfers the raw tuple buffer into this object.
        /// @return `*this` after the transfer.
        constexpr variable_tuple& operator=(variable_tuple&&) = default;
    };
} // namespace hsd