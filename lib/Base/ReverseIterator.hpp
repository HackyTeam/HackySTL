#pragma once

/// @file ReverseIterator.hpp
/// @brief Reverse iterator over contiguous storage that never forms a pointer
/// before the first element.
/// @details Reverse traversal of a vector or string view needs an end sentinel
/// one position before the first element when the sentinel is a raw pointer,
/// which is undefined behavior and is rejected in constant evaluation.

#include "Concepts.hpp"

namespace hsd
{
    /// @brief Reverse iterator over contiguous storage.
    /// @tparam Pointer Element pointer type, `T*` or `const T*`.
    /// @details Holds the position one past the element it refers to. The end
    /// sentinel is the forward `begin()` pointer, so no pointer is formed before
    /// the first element, including for empty containers.
    template <typename Pointer>
    class contiguous_reverse_iterator
    {
    private:
        /// Position one past the element currently referred to.
        Pointer _after = nullptr;

    public:
        /// @brief Constructs a reverse iterator that refers to the element before `after`.
        /// @param after Position one past the element to refer to.
        constexpr explicit contiguous_reverse_iterator(Pointer after)
            : _after{after}
        {}

        /// @brief Returns a reference to the referred element.
        /// @return Reference to the element one position before the stored position.
        constexpr auto& operator*() const
        {
            return *(_after - 1);
        }

        /// @brief Returns a pointer to the referred element.
        /// @return Pointer to the element one position before the stored position.
        constexpr Pointer operator->() const
        {
            return _after - 1;
        }

        /// @brief Advances toward the beginning of the storage.
        /// @return Reference to `*this`.
        constexpr contiguous_reverse_iterator& operator++()
        {
            --_after;
            return *this;
        }

        /// @brief Advances toward the beginning of the storage.
        /// @return The iterator before advancing.
        constexpr contiguous_reverse_iterator operator++(int)
        {
            contiguous_reverse_iterator _previous = *this;
            --_after;
            return _previous;
        }

        /// @brief Moves back toward the end of the storage.
        /// @return Reference to `*this`.
        constexpr contiguous_reverse_iterator& operator--()
        {
            ++_after;
            return *this;
        }

        /// @brief Moves back toward the end of the storage.
        /// @return The iterator before moving.
        constexpr contiguous_reverse_iterator operator--(int)
        {
            contiguous_reverse_iterator _previous = *this;
            ++_after;
            return _previous;
        }

        /// @brief Compares the stored positions.
        /// @param rhs Iterator compared with this one.
        /// @return True when both refer to the same position.
        constexpr bool operator==(const contiguous_reverse_iterator& rhs) const = default;
    };
    /// @brief Reverse iterator over any bidirectional iterator.
    /// @tparam Iter Bidirectional iterator type with `++`, `--` and `*`.
    /// @details Like `contiguous_reverse_iterator`, the end sentinel is the forward
    /// `begin()` iterator, so no position before the first element is formed.
    template <typename Iter>
    class reverse_bidirectional_iterator
    {
    private:
        /// Position one past the element currently referred to.
        remove_const_t<Iter> _after{};

    public:
        /// @brief Constructs a reverse iterator that refers to the element before `after`.
        /// @param after Position one past the element to refer to.
        constexpr explicit reverse_bidirectional_iterator(Iter after)
            : _after{after}
        {}

        /// @brief Returns a reference to the referred element.
        /// @return Reference to the element one position before the stored position.
        /// @details The copy is stepped back without const, then read through a const
        /// view when `Iter` is const, so a const iterator gives a const element.
        constexpr decltype(auto) operator*() const
        {
            remove_const_t<Iter> _previous = _after;
            --_previous;

            if constexpr (IsConst<Iter>)
            {
                const Iter& _view = _previous;
                return *_view;
            }
            else
            {
                return *_previous;
            }
        }

        /// @brief Advances toward the beginning of the sequence.
        /// @return Reference to `*this`.
        constexpr reverse_bidirectional_iterator& operator++()
        {
            --_after;
            return *this;
        }

        /// @brief Advances toward the beginning of the sequence.
        /// @return The iterator before advancing.
        constexpr reverse_bidirectional_iterator operator++(int)
        {
            reverse_bidirectional_iterator _previous = *this;
            --_after;
            return _previous;
        }

        /// @brief Moves back toward the end of the sequence.
        /// @return Reference to `*this`.
        constexpr reverse_bidirectional_iterator& operator--()
        {
            ++_after;
            return *this;
        }

        /// @brief Moves back toward the end of the sequence.
        /// @return The iterator before moving.
        constexpr reverse_bidirectional_iterator operator--(int)
        {
            reverse_bidirectional_iterator _previous = *this;
            ++_after;
            return _previous;
        }

        /// @brief Compares the stored positions.
        /// @param rhs Iterator compared with this one.
        /// @return True when both refer to the same position.
        constexpr bool operator==(const reverse_bidirectional_iterator& rhs) const
        {
            return _after == rhs._after;
        }
    };

    /// @brief Tells whether `T` is one of the reverse iterators in this header.
    template <typename T>
    inline constexpr bool is_reverse_iterator_v = false;

    /// @brief Specialization matching `contiguous_reverse_iterator`.
    template <typename Pointer>
    inline constexpr bool is_reverse_iterator_v<contiguous_reverse_iterator<Pointer>> = true;

    /// @brief Specialization matching `reverse_bidirectional_iterator`.
    template <typename Iter>
    inline constexpr bool is_reverse_iterator_v<reverse_bidirectional_iterator<Iter>> = true;
} // namespace hsd
