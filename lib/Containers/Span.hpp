#pragma once

/// @file Span.hpp
/// @brief Non-owning contiguous views over existing storage.
/// @details Provides bounded access, iteration, subviews, and enumeration
/// without allocating or extending the lifetime of the referenced elements.

#include "../Base/Result.hpp"
#include "../Wrappers/Pair.hpp" 

namespace hsd
{
    namespace span_detail
    {
        ///
        /// @brief Wraps an iterator so dereferencing also yields its index.
        ///
        /// @tparam T - Underlying contiguous forward iterator type.
        ///
        template <ContiguousForwardIterable T>
        class countable_iterator
        {
        private:
            /// @brief Underlying iterator being advanced.
            T _iter;
            /// @brief Number of elements already produced.
            usize _count;
        
        public:
            ///
            /// @brief Wraps an underlying iterator starting at index `count`.
            ///
            /// @param iter - Underlying iterator to wrap.
            /// @param count - Initial index reported by `operator*`.
            ///
            constexpr countable_iterator(T iter, usize count = 0)
                : _iter{iter}, _count{count}
            {}

            ///
            /// @brief Advances to the next element and increments the index.
            ///
            /// @return A reference to this iterator.
            ///
            constexpr auto& operator++()
            {
                _iter++;
                _count++;
                return *this;
            }

            ///
            /// @brief Advances to the next element, returning the prior state.
            ///
            /// @return A copy of this iterator before the increment.
            ///
            constexpr auto operator++(i32)
            {
                countable_iterator tmp = *this;
                operator++();
                return tmp;
            }

            ///
            /// @brief Gets the distance between this and another iterator.
            ///
            /// @param other - Iterator to measure the distance to.
            /// @return The signed distance in the underlying iterator.
            ///
            constexpr isize operator-(countable_iterator other) const
            {
                return _iter - other._iter;
            }

            ///
            /// @brief Tests whether both iterators refer to the same position.
            ///
            /// @param rhs - Iterator compared against this one.
            /// @return `true` when both wrap the same underlying position.
            ///
            constexpr bool operator==(const countable_iterator& rhs) const
            {
                return _iter == rhs._iter;
            }

            ///
            /// @brief Tests whether the iterators refer to different positions.
            ///
            /// @param rhs - Iterator compared against this one.
            /// @return `true` when the two wrap different underlying positions.
            ///
            constexpr bool operator!=(const countable_iterator& rhs) const
            {
                return _iter != rhs._iter;
            }

            ///
            /// @brief Dereferences the underlying iterator paired with its index.
            ///
            /// @return A `pair` of the referenced element and its index.
            ///
            constexpr decltype(auto) operator*()
            {
                return hsd::pair<decltype(*_iter), usize>
                {
                    *_iter, _count
                };
            }
            
            ///
            /// @brief const-qualified overload of `operator*()`.
            ///
            /// @return A `pair` of the referenced element and its index.
            ///
            constexpr decltype(auto) operator*() const
            {
                return hsd::pair<decltype(*_iter), usize>
                {
                    *_iter, _count
                };
            }
        };

        ///
        /// @brief Thin forwarding wrapper around a contiguous forward iterator.
        ///
        /// @tparam T - Underlying contiguous forward iterator type.
        ///
        template <ContiguousForwardIterable T>
        class iterator
        {
        private:
            /// @brief Underlying iterator being wrapped.
            T _iter;

        public:
            ///
            /// @brief Wraps an underlying iterator.
            ///
            /// @param iter - Underlying iterator to wrap.
            ///
            constexpr iterator(T iter)
                : _iter{iter}
            {}

            ///
            /// @brief Advances to the next element.
            ///
            /// @return A reference to this iterator.
            ///
            constexpr auto& operator++()
            {
                _iter++;
                return *this;
            }

            ///
            /// @brief Advances to the next element, returning the prior state.
            ///
            /// @return A copy of this iterator before the increment.
            ///
            constexpr iterator operator++(i32)
            {
                iterator tmp = *this;
                operator++();
                return tmp;
            }

            ///
            /// @brief Gets the distance between this and another iterator.
            ///
            /// @param other - Iterator to measure the distance to.
            /// @return The signed distance in the underlying iterator.
            ///
            constexpr isize operator-(iterator other) const
            {
                return _iter - other._iter;
            }

            ///
            /// @brief Tests whether both iterators refer to the same position.
            ///
            /// @param rhs - Iterator compared against this one.
            /// @return `true` when both wrap the same underlying position.
            ///
            constexpr bool operator==(const iterator& rhs) const
            {
                return _iter == rhs._iter;
            }

            ///
            /// @brief Tests whether the iterators refer to different positions.
            ///
            /// @param rhs - Iterator compared against this one.
            /// @return `true` when the two wrap different underlying positions.
            ///
            constexpr bool operator!=(const iterator& rhs) const
            {
                return _iter != rhs._iter;
            }

            ///
            /// @brief Dereferences the underlying iterator.
            ///
            /// @return A reference to the referenced element.
            ///
            constexpr decltype(auto) operator*()
            {
                return *_iter;
            }
            
            ///
            /// @brief const-qualified overload of `operator*()`.
            ///
            /// @return A reference to the referenced element.
            ///
            constexpr decltype(auto) operator*() const
            {
                return *_iter;
            }
        };
    } // namespace span_detail

    template <ContiguousForwardIterable T>
    /// @brief Non-owning contiguous range with optional enumeration.
    class span
    {
    private:
        /// @brief Wrapped iterator type used to walk the viewed range.
        using iter_type = typename span_detail::iterator<T>;
        /// @brief Number of elements visible through this view.
        usize _view_size = 0;
        /// @brief Iterator to the first element of the view.
        iter_type _begin;
        /// @brief Iterator one past the last element of the view.
        iter_type _end;
        
    public:
        /// @brief Mutable reference to a viewed element.
        using reference_type = T&;
        /// @brief Pointer to a viewed element.
        using pointer_type = T*;
        /// @brief Type of a viewed element.
        using value_type = T;

        ///
        /// @brief Builds a view directly from a bound and its element count.
        ///
        /// @param begin - Iterator to the first element of the view.
        /// @param end - Iterator one past the last element of the view.
        /// @param view_size - Number of elements between `begin` and `end`.
        ///
        constexpr span(const iter_type& begin, 
            const iter_type& end, usize view_size)
            : _view_size{view_size}, _begin{begin}, _end{end}
        {}

        ///
        /// @brief Views the full range of a mutable container.
        ///
        /// @tparam U - Forward container type providing `begin`/`end`/`size`.
        /// @param container - Container viewed by this span; must outlive it.
        ///
        template <IsForwardContainer U>
        constexpr span(U& container)
            : _view_size{container.size()}, 
            _begin{container.begin()}, 
            _end{container.end()}
        {}

        ///
        /// @brief Views the full range of a const container.
        ///
        /// @tparam U - Forward container type providing `begin`/`end`/`size`.
        /// @param container - Container viewed by this span; must outlive it.
        ///
        template <IsForwardContainer U>
        constexpr span(const U& container)
            : _view_size{container.size()}, 
            _begin{container.begin()}, 
            _end{container.end()}
        {}

        /// @brief Deleted to prevent viewing a temporary that would dangle.
        template <IsForwardContainer U>
        constexpr span(U&& container) = delete;

        ///
        /// @brief Builds a view that skips the first `quantity` elements.
        ///
        /// @param quantity - Number of leading elements to skip.
        /// @return A span over the remaining elements, or a `runtime_error`
        /// when `quantity` exceeds the current view size.
        ///
        constexpr auto drop(usize quantity) const
            -> result<span, runtime_error>
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;

                for (; _index < quantity; _index++, _result_iter++)
                    ;

                return span{_result_iter, _end, _view_size - _index};
            }
        }

        ///
        /// @brief Builds a view limited to the first `quantity` elements.
        ///
        /// @param quantity - Number of leading elements to keep.
        /// @return A span over the first `quantity` elements, or a
        /// `runtime_error` when `quantity` exceeds the current view size.
        ///
        constexpr auto take(usize quantity) const
            -> result<span, runtime_error> 
        {
            if(quantity > _view_size)
            {
                return runtime_error{"Dropping out of bounds"};
            }
            else
            {
                usize _index = 0;
                iter_type _result_iter = _begin;
                
                for(; _index < quantity; _index++, _result_iter++)
                    ;

                return span {
                    _begin, _result_iter, _view_size - _index
                };
            }
        }

        ///
        /// @brief Builds a view whose elements dereference to (value, index) pairs.
        ///
        /// @return A span of `countable_iterator` over the same elements.
        ///
        constexpr auto enumerate()
        {
            using span_detail::countable_iterator;
            return span<countable_iterator<iter_type>> {
                {_begin}, {_end}, _view_size
            };
        }

        ///
        /// @brief const-qualified overload of `enumerate()`.
        ///
        /// @return A span of `countable_iterator` over the same elements.
        ///
        constexpr auto enumerate() const
        {
            using span_detail::countable_iterator;
            return span<countable_iterator<iter_type>> {
                {_begin}, {_end}, _view_size
            };
        }

        ///
        /// @brief Gets the number of elements visible through this view.
        ///
        /// @return The current view size.
        ///
        constexpr usize size() const
        {
            return _view_size;
        }

        ///
        /// @brief Gets an iterator to the first element of the view.
        ///
        /// @return An iterator to the first viewed element.
        ///
        constexpr iter_type begin()
        {
            return _begin;
        }

        ///
        /// @brief const-qualified overload of `begin()`.
        ///
        /// @return An iterator to the first viewed element.
        ///
        constexpr const iter_type begin() const
        {
            return _begin;
        }
        
        ///
        /// @brief Gets an iterator one past the last element of the view.
        ///
        /// @return An iterator one past the last viewed element.
        ///
        constexpr iter_type end()
        {
            return _end;
        }
        
        ///
        /// @brief const-qualified overload of `end()`.
        ///
        /// @return An iterator one past the last viewed element.
        ///
        constexpr const iter_type end() const
        {
            return _end;
        }
    };

    /// @brief Deduces `span` over a mutable container's iterator type.
    /// @param container - Container whose iterator type is deduced.
    template <IsForwardContainer U>
    span(U& container) -> span<decltype(container.end())>;
    /// @brief Deduces `span` over a const container's iterator type.
    /// @param container - Container whose iterator type is deduced.
    template <IsForwardContainer U>
    span(const U& container) -> span<decltype(container.end())>;
} // namespace hsd
