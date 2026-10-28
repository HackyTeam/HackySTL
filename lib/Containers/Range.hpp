#pragma once

/// @file Range.hpp
/// @brief Lazy range views and composable sequence adaptors.
/// @details Implements drop, take, reverse, filter, transform, and random
/// traversal pipelines while preserving references to source elements safely.

#include "../Base/Random.hpp"
#include "../Base/ReverseIterator.hpp"
#include "Span.hpp"

namespace hsd
{
    namespace ranges
    {
        namespace views
        {
            /// @brief Implementation details backing the public `views::` adaptors.
            namespace views_detail
            {
                /// @brief Wraps a bidirectional iterator so that incrementing it
                /// walks the referenced sequence back-to-front.
                /// @details Does not own the referenced elements; validity tracks
                /// the lifetime of the sequence the wrapped iterator points into.
                template <ReverseIterable T>
                class reverse_iterator
                {
                private:
                    /// @brief Underlying iterator, moved opposite to its natural direction.
                    T _iter;
                
                public:
                    ///
                    /// @brief Wraps an existing iterator without changing its position.
                    ///
                    /// @param iter - Underlying iterator to reverse the traversal of.
                    ///
                    constexpr reverse_iterator(T iter)
                        : _iter{iter}
                    {}

                    ///
                    /// @brief Advances the reverse iterator by moving the
                    /// underlying iterator one step backward.
                    ///
                    /// @return Reference to `*this` after the move.
                    ///
                    constexpr auto& operator++()
                    {
                        --_iter;
                        return *this;
                    }

                    ///
                    /// @brief Post-increment; advances `*this` and returns the
                    /// pre-increment position.
                    ///
                    /// @return Copy of the iterator before advancing.
                    ///
                    constexpr reverse_iterator operator++(i32)
                    {
                        reverse_iterator tmp = *this;
                        operator++();
                        return tmp;
                    }

                    ///
                    /// @brief Moves the reverse iterator one step forward by
                    /// moving the underlying iterator one step forward.
                    ///
                    /// @return Reference to `*this` after the move.
                    ///
                    constexpr auto& operator--()
                    {
                        ++_iter;
                        return *this;
                    }

                    ///
                    /// @brief Post-decrement; moves `*this` forward and returns
                    /// the pre-decrement position.
                    ///
                    /// @return Copy of the iterator before moving.
                    ///
                    constexpr reverse_iterator operator--(i32)
                    {
                        reverse_iterator tmp = *this;
                        operator--();
                        return tmp;
                    }

                    ///
                    /// @brief Compares the wrapped underlying iterator positions.
                    ///
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when both wrap iterators at the same position.
                    ///
                    constexpr bool operator==(const reverse_iterator& rhs) const
                    {
                        return _iter == rhs._iter;
                    }

                    ///
                    /// @brief Compares the wrapped underlying iterator positions.
                    ///
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when the wrapped iterators differ in position.
                    ///
                    constexpr bool operator!=(const reverse_iterator& rhs) const
                    {
                        return _iter != rhs._iter;
                    }

                    /// @brief Dereferences the underlying iterator.
                    /// @return Reference to the element currently pointed to.
                    constexpr auto& operator*()
                    {
                        return *_iter;
                    }

                    /// @brief Dereferences the underlying iterator.
                    /// @return Const reference to the element currently pointed to.
                    constexpr const auto& operator*() const
                    {
                        return *_iter;
                    }
                };

                /// @brief Forward iterator that visits a sequence in a
                /// pseudo-random, non-repeating step order.
                /// @details Each increment restarts from the origin and walks a
                /// uniformly chosen number of steps in `[0, _index]`, then
                /// advances `_index`; two instances compare equal once they have
                /// taken the same number of steps, which is how `end()` is
                /// detected rather than by underlying iterator position.
                template <ForwardIterable T>
                class random_iterator
                {
                private:
                    /// @brief Current traversal position, recomputed from `_origin` each step.
                    T _iter;
                    /// @brief Start-of-range position every increment restarts from.
                    T _origin;
                    /// @brief Number of increments performed so far; doubles as the equality key.
                    usize _index;
                    /// @brief Random engine shared by every instantiation of this iterator type.
                    static inline mt19937_64 engine{};
                
                public:
                    ///
                    /// @brief Wraps an iterator as the traversal origin at a given step index.
                    ///
                    /// @param iter - Iterator marking both the current and origin position.
                    /// @param index - Initial step count (`0` for begin, range size for end).
                    ///
                    constexpr random_iterator(T iter, usize index)
                        : _iter{iter}, _origin{iter}, _index{index}
                    {}

                    ///
                    /// @brief Restarts from the origin and advances a random number
                    /// of steps in `[0, _index + 1)`, then increments `_index`.
                    ///
                    /// @return Reference to `*this` after the move.
                    ///
                    constexpr auto& operator++()
                    {
                        _iter = _origin;
                        
                        usize _to = engine.generate<usize>(0u, _index + 1).unwrap();
                        
                        for (usize _from = 0; _from < _to; _from++, _iter++)
                            ;

                        _index++;
                        return *this;
                    }

                    ///
                    /// @brief Post-increment; advances `*this` and returns the
                    /// pre-increment position.
                    ///
                    /// @return Copy of the iterator before advancing.
                    ///
                    constexpr random_iterator operator++(i32)
                    {
                        auto tmp = *this;
                        operator++();
                        return tmp;
                    }

                    ///
                    /// @brief Compares the number of steps taken by each iterator,
                    /// not their underlying positions.
                    ///
                    /// @param lhs - Left-hand iterator.
                    /// @param rhs - Right-hand iterator.
                    /// @return `true` when both have taken the same number of steps.
                    ///
                    constexpr friend bool operator==(
                        const random_iterator& lhs, const random_iterator& rhs)
                    {
                        return lhs._index == rhs._index;
                    }

                    ///
                    /// @brief Compares the number of steps taken by each iterator,
                    /// not their underlying positions.
                    ///
                    /// @param lhs - Left-hand iterator.
                    /// @param rhs - Right-hand iterator.
                    /// @return `true` when the step counts differ.
                    ///
                    constexpr friend bool operator!=(
                        const random_iterator& lhs, const random_iterator& rhs)
                    {
                        return lhs._index != rhs._index; 
                    }

                    /// @brief Dereferences the current traversal position.
                    /// @return Reference to the element currently pointed to.
                    constexpr auto& operator*()
                    {
                        return *_iter;
                    }

                    /// @brief Dereferences the current traversal position.
                    /// @return Const reference to the element currently pointed to.
                    constexpr const auto& operator*() const
                    {
                        return *_iter;
                    }
                };

                /// @brief Forward iterator that skips over elements not matching
                /// a predicate, exposing only the elements that satisfy it.
                /// @details Never advances past `_end`; references the original
                /// container's elements, so writes through `operator*` mutate it.
                template <ForwardIterable T, typename F>
                requires (InvocableRet<bool, F, decltype(*declval<T>())>)
                class filter_iterator
                {
                private:
                    /// @brief Underlying iterator type of the filtered range.
                    using iter_type = T;

                    /// @brief Predicate deciding which elements are visible.
                    F _func;
                    /// @brief Current position; always satisfies `_func` unless equal to `_end`.
                    iter_type _current;
                    /// @brief One-past-the-last position of the underlying range.
                    iter_type _end;
                
                public:
                    ///
                    /// @brief Builds an iterator positioned at the first element in
                    /// `[current, end)` that satisfies `func`, or at `end` if none do.
                    ///
                    /// @param func - Predicate applied to each element.
                    /// @param current - Start of the underlying range to search.
                    /// @param end - End of the underlying range; never dereferenced.
                    ///
                    constexpr filter_iterator(
                        const F& func, iter_type current, iter_type end)
                        : _func{func}, _current{current}, _end{end}
                    {
                        _current = find(_current, _end, _func);
                    }

                    ///
                    /// @brief Advances to the next element satisfying the predicate,
                    /// or to `_end` if no further element matches.
                    ///
                    /// @return Reference to `*this` after the move.
                    ///
                    constexpr auto& operator++()
                    {
                        auto _copy = _current;
                        
                        _current = find(
                            (_copy != _end) ? ++_copy : _copy, _end, _func
                        );
                        
                        return *this;
                    }

                    ///
                    /// @brief Post-increment; advances `*this` and returns the
                    /// pre-increment position.
                    ///
                    /// @return Copy of the iterator before advancing.
                    ///
                    constexpr filter_iterator operator++(i32)
                    {
                        filter_iterator tmp = *this;
                        operator++();
                        return tmp;
                    }

                    /// @brief Compares the current match positions.
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when both point at the same element.
                    constexpr bool operator==(const filter_iterator& rhs) const
                    {
                        return _current == rhs._current;
                    }

                    /// @brief Compares the current match positions.
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when the positions differ.
                    constexpr bool operator!=(const filter_iterator& rhs) const
                    {
                        return _current != rhs._current;
                    }

                    /// @brief Dereferences the current matching element.
                    /// @return Reference to the element, writable through the original container.
                    constexpr auto& operator*()
                    {
                        return *_current;
                    }

                    /// @brief Dereferences the current matching element.
                    /// @return Const reference to the element.
                    constexpr const auto& operator*() const
                    {
                        return *_current;
                    }
                };

                /// @brief Non-owning `begin()`/`end()` view exposing `filter_iterator`
                /// endpoints so a filtered sequence can be used in range-based for loops.
                template <ForwardIterable T, typename F>
                class filter_wrapper
                {
                private:
                    /// @brief Predicate type shared by `_begin` and `_end`.
                    using func_type = F;
                    /// @brief Iterator type produced by this wrapper.
                    using iter_type = filter_iterator<T, F>;

                    /// @brief Predicate shared by `_begin` and `_end`.
                    func_type _func;
                    /// @brief First matching element, or `_end` if none match.
                    iter_type _begin;
                    /// @brief One-past-the-last position of the underlying range.
                    iter_type _end;

                public:
                    ///
                    /// @brief Builds begin/end filter iterators over `[begin, end)`.
                    ///
                    /// @param func - Predicate applied to every element.
                    /// @param begin - Start of the underlying range.
                    /// @param end - End of the underlying range.
                    ///
                    constexpr filter_wrapper(
                        func_type func, const T& begin, const T& end)
                        : _func{func}, _begin{_func, begin, end}, _end{_func, end, end}
                    {}

                    /// @brief Returns the first matching element, or `end()` if none match.
                    /// @return Iterator at the first element satisfying the predicate.
                    constexpr iter_type begin()
                    {
                        return _begin;
                    }

                    /// @brief Returns the first matching element, or `end()` if none match.
                    /// @return Iterator at the first element satisfying the predicate.
                    constexpr const iter_type begin() const
                    {
                        return _begin;
                    }

                    /// @brief Returns the sentinel one past the last element of the range.
                    /// @return Iterator equal to the wrapped range's end.
                    constexpr iter_type end()
                    {
                        return _end;
                    }

                    /// @brief Returns the sentinel one past the last element of the range.
                    /// @return Iterator equal to the wrapped range's end.
                    constexpr const iter_type end() const
                    {
                        return _end;
                    }
                };

                /// @brief Forward iterator that lazily applies a function to each
                /// element of the underlying range on every dereference.
                /// @details The transform is recomputed on each `operator*` call
                /// rather than cached; traversal simply forwards to the
                /// underlying iterator.
                template <ForwardIterable T, typename F>
                requires (Invocable<F, decltype(*declval<T>())>)
                class transform_iterator
                {
                private:
                    /// @brief Underlying iterator type being transformed.
                    using iter_type = T;

                    /// @brief Function applied to each dereferenced element.
                    F _func;
                    /// @brief Current position in the underlying range.
                    iter_type _iter;
                
                public:
                    ///
                    /// @brief Wraps an iterator with the function to apply to its elements.
                    ///
                    /// @param func - Function applied on every dereference.
                    /// @param iter - Underlying range position.
                    ///
                    constexpr transform_iterator(F func, iter_type iter)
                        : _func{hsd::move(func)}, _iter{iter}
                    {}

                    /// @brief Advances the underlying iterator by one element.
                    /// @return Reference to `*this` after the move.
                    constexpr auto& operator++()
                    {
                        ++_iter;
                        return *this;
                    }

                    ///
                    /// @brief Post-increment; advances `*this` and returns the
                    /// pre-increment position.
                    ///
                    /// @return Copy of the iterator before advancing.
                    ///
                    constexpr transform_iterator operator++(i32)
                    {
                        transform_iterator tmp = *this;
                        operator++();
                        return tmp;
                    }

                    /// @brief Compares the underlying iterator positions.
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when both wrap the same position.
                    constexpr bool operator==(const transform_iterator& rhs) const
                    {
                        return _iter == rhs._iter;
                    }

                    /// @brief Compares the underlying iterator positions.
                    /// @param rhs - Iterator to compare against.
                    /// @return `true` when the positions differ.
                    constexpr bool operator!=(const transform_iterator& rhs) const
                    {
                        return _iter != rhs._iter;
                    }

                    /// @brief Applies `_func` to the current element.
                    /// @return Result of invoking `_func` on the dereferenced element.
                    constexpr decltype(auto) operator*()
                    {
                        return _func(*_iter);
                    }

                    /// @brief Applies `_func` to the current element.
                    /// @return Result of invoking `_func` on the dereferenced element.
                    constexpr decltype(auto) operator*() const
                    {
                        return _func(*_iter);
                    }
                };

                /// @brief Non-owning `begin()`/`end()` view exposing `transform_iterator`
                /// endpoints so a transformed sequence can be used in range-based for loops.
                template <ForwardIterable T, typename F>
                class transform_wrapper
                {
                private:
                    /// @brief Function type shared by `_begin` and `_end`.
                    using func_type = F;
                    /// @brief Iterator type produced by this wrapper.
                    using iter_type = transform_iterator<T, F>;

                    /// @brief Function shared by `_begin` and `_end`.
                    func_type _func;
                    /// @brief First element of the underlying range, transform applied lazily.
                    iter_type _begin;
                    /// @brief One-past-the-last position of the underlying range.
                    iter_type _end;

                public:
                    ///
                    /// @brief Builds begin/end transform iterators over `[begin, end)`.
                    ///
                    /// @param func - Function applied to every element.
                    /// @param begin - Start of the underlying range.
                    /// @param end - End of the underlying range.
                    ///
                    constexpr transform_wrapper(
                        func_type func, const T& begin, const T& end)
                        : _func{func}, _begin{_func, begin}, _end{_func, end}
                    {}

                    /// @brief Returns the first element of the transformed range.
                    /// @return Iterator applying the transform at the range's first element.
                    constexpr iter_type begin()
                    {
                        return _begin;
                    }

                    /// @brief Returns the first element of the transformed range.
                    /// @return Iterator applying the transform at the range's first element.
                    constexpr const iter_type begin() const
                    {
                        return _begin;
                    }

                    /// @brief Returns the sentinel one past the last element of the range.
                    /// @return Iterator equal to the wrapped range's end.
                    constexpr iter_type end()
                    {
                        return _end;
                    }

                    /// @brief Returns the sentinel one past the last element of the range.
                    /// @return Iterator equal to the wrapped range's end.
                    constexpr const iter_type end() const
                    {
                        return _end;
                    }
                };

                /// @brief Non-owning adaptor pairing a `[begin, end)` iterator
                /// range with a precomputed element count.
                /// @details Returned by `drop`, `take`, `drop_while`, and
                /// `take_while` so their results remain chainable with further
                /// adaptors. References the original container's elements and
                /// performs no validation that `begin`/`end`/`size` are mutually
                /// consistent; that is the caller's responsibility.
                template <ForwardIterable T>
                class iter_wrapper
                {
                private:
                    /// @brief Underlying iterator type of the wrapped sub-range.
                    using iter_type = T;

                    /// @brief First element of the wrapped sub-range.
                    iter_type _begin;
                    /// @brief One-past-the-last element of the wrapped sub-range.
                    iter_type _end;
                    /// @brief Element count of `[_begin, _end)`, supplied by the caller.
                    usize _size = 0;

                public:
                    ///
                    /// @brief Wraps an already-computed `[begin, end)` sub-range.
                    ///
                    /// @param begin - First element of the sub-range.
                    /// @param end - One past the last element of the sub-range.
                    /// @param size - Element count of the sub-range; not verified against begin/end.
                    ///
                    constexpr iter_wrapper(
                        const iter_type& begin, const iter_type& end, usize size)
                        : _begin{begin}, _end{end}, _size{size}
                    {}

                    /// @brief Returns the element count supplied at construction.
                    /// @return The cached `_size` value.
                    constexpr usize size() const
                    {
                        return _size;
                    }

                    /// @brief Returns the first element of the wrapped sub-range.
                    /// @return Iterator equal to `_begin`.
                    constexpr iter_type begin()
                    {
                        return _begin;
                    }

                    /// @brief Returns the first element of the wrapped sub-range.
                    /// @return Iterator equal to `_begin`.
                    constexpr const iter_type begin() const
                    {
                        return _begin;
                    }

                    /// @brief Returns the sentinel one past the last element.
                    /// @return Iterator equal to `_end`.
                    constexpr iter_type end()
                    {
                        return _end;
                    }

                    /// @brief Returns the sentinel one past the last element.
                    /// @return Iterator equal to `_end`.
                    constexpr const iter_type end() const
                    {
                        return _end;
                    }

                    ///
                    /// @brief Returns the position one before `end()`, satisfying
                    /// `IsReverseContainer` so this wrapper can itself be piped
                    /// into reverse-traversal adaptors.
                    ///
                    /// @return The decremented end iterator; requires `T` to support `operator--`.
                    ///
                    constexpr iter_type rbegin()
                    {
                        auto _val = _end;
                        return --_val;
                    }

                    /// @copydoc rbegin()
                    constexpr const iter_type rbegin() const
                    {
                        auto _val = _end;
                        return --_val;
                    }

                    ///
                    /// @brief Returns the position one before `begin()`, pairing
                    /// with `rbegin()` to bound a reverse traversal.
                    ///
                    /// @return The decremented begin iterator; requires `T` to support `operator--`.
                    ///
                    constexpr iter_type rend()
                    {
                        auto _val = _begin;
                        return --_val;
                    }

                    /// @copydoc rend()
                    constexpr const iter_type rend() const
                    {
                        auto _val = _begin;
                        return --_val;
                    }
                };
                
                /// @brief Lazy view presenting a bidirectional container's
                /// elements back-to-front, without copying them.
                /// @details Backed by `reverse_iterator`; references the source
                /// container's storage, so the view is only valid as long as the
                /// container is and stays unmodified in size.
                template <ReverseIterable T>
                class reverse
                {
                private:
                    /// @brief Iterator type produced by this view.
                    // Containers that return reverse iterators are traversed with them directly;
                    // other element iterators are wrapped so that they step backward.
                    using iter_type = conditional_t<is_reverse_iterator_v<T>, T, views_detail::reverse_iterator<T>>;
                    /// @brief Number of elements visible through this view.
                    usize _view_size = 0;
                    /// @brief First element in reverse traversal order.
                    iter_type _begin;
                    /// @brief Sentinel one past the last element in reverse traversal order.
                    iter_type _end;
                
                    ///
                    /// @brief Builds a sub-view directly from known bounds; used
                    /// internally by `drop`/`take` to avoid re-deriving them.
                    ///
                    /// @param begin - First element of the sub-view.
                    /// @param end - Sentinel one past the last element.
                    /// @param view_size - Element count of `[begin, end)`.
                    ///
                    constexpr reverse(const iter_type& begin, 
                        const iter_type& end, usize view_size)
                        : _view_size{view_size}, _begin{begin}, _end{end}
                    {}

                public:
                    /// @brief Element type traversed by this view.
                    using value_type = T;

                    ///
                    /// @brief Builds a view over the full reverse traversal of `container`.
                    ///
                    /// @param container - Container providing `rbegin()`/`rend()`/`size()`.
                    ///
                    template <IsReverseContainer U>
                    constexpr reverse(U&& container)
                        : _view_size{container.size()}, 
                        _begin{container.rbegin()}, 
                        _end{container.rend()}
                    {}

                    ///
                    /// @brief Returns a view with the first `quantity` elements
                    /// (in reverse order) skipped.
                    ///
                    /// @param quantity - Number of leading elements to skip.
                    /// @return The remaining view, or a `runtime_error` if
                    /// `quantity` exceeds the current view size.
                    ///
                    constexpr auto drop(usize quantity) const
                        -> result<reverse, runtime_error> 
                    {
                        if (quantity > _view_size)
                        {
                            return runtime_error{"Dropping out of bounds"};
                        }
                        else
                        {
                            usize _index = 0;
                            iter_type _result_iter = _begin;
                            
                            for (; _index < quantity; _index++, _result_iter++)
                                ;

                            return reverse{_result_iter, _end, _view_size - _index};
                        }
                    }

                    ///
                    /// @brief Returns a view limited to the first `quantity`
                    /// elements (in reverse order).
                    ///
                    /// @param quantity - Number of leading elements to keep.
                    /// @return The truncated view, or a `runtime_error` if
                    /// `quantity` exceeds the current view size.
                    ///
                    constexpr auto take(usize quantity) const
                        -> result<reverse, runtime_error> 
                    {
                        if(quantity > _view_size)
                        {
                            return runtime_error{"Dropping out of bounds"};
                        }
                        else
                        {
                            usize _index = 0;
                            iter_type _result_iter = _begin;
                            for(; _index < quantity; _index++, _result_iter++);
                            return reverse{_begin, _result_iter, _view_size - _index};
                        }
                    }

                    /// @brief Returns the number of elements visible through this view.
                    /// @return The cached `_view_size` value.
                    constexpr usize size() const
                    {
                        return _view_size;
                    }

                    /// @brief Returns the first element in reverse traversal order.
                    /// @return Iterator equal to `_begin`.
                    constexpr auto begin()
                    {
                        return _begin;
                    }

                    /// @brief Returns the first element in reverse traversal order.
                    /// @return Iterator equal to `_begin`.
                    constexpr auto begin() const
                    {
                        return _begin;
                    }

                    /// @brief Returns the sentinel one past the last element.
                    /// @return Iterator equal to `_end`.
                    constexpr auto end()
                    {
                        return _end;
                    }

                    /// @brief Returns the sentinel one past the last element.
                    /// @return Iterator equal to `_end`.
                    constexpr auto end() const
                    {
                        return _end;
                    }
                };

                /// @brief Lazy view presenting a forward container's elements
                /// in a pseudo-random traversal order.
                /// @details Backed by `random_iterator`; references the source
                /// container's storage, so the view is only valid as long as the
                /// container is and stays unmodified in size.
                template <ForwardIterable T>
                class random
                {
                private:
                    /// @brief Iterator type produced by this view.
                    using iter_type = views_detail::random_iterator<T>;
                    /// @brief Number of elements visible through this view.
                    usize _view_size = 0;
                    /// @brief First element (step index `0`) of the random traversal.
                    iter_type _begin;
                    /// @brief Sentinel step matching the current view size.
                    iter_type _end;
                
                    ///
                    /// @brief Builds a sub-view directly from known bounds; used
                    /// internally by `drop`/`take` to avoid re-deriving them.
                    ///
                    /// @param begin - First step of the sub-view.
                    /// @param end - Sentinel step of the sub-view.
                    /// @param view_size - Element count represented by `[begin, end)`.
                    ///
                    constexpr random(const iter_type& begin, const iter_type& end, usize view_size)
                        : _view_size{view_size}, _begin{begin}, _end{end}
                    {}

                public:
                    /// @brief Element type traversed by this view.
                    using value_type = T;

                    ///
                    /// @brief Builds a view over the full random traversal of `container`.
                    ///
                    /// @param container - Container providing `begin()`/`end()`/`size()`.
                    ///
                    template <IsForwardContainer U>
                    constexpr random(U&& container)
                        : _view_size{container.size()}, _begin{container.begin(), 0}, 
                        _end{container.end(), container.size()}
                    {}

                    ///
                    /// @brief Returns a view with the first `quantity` steps of
                    /// the random traversal skipped.
                    ///
                    /// @param quantity - Number of leading steps to skip.
                    /// @return The remaining view, or a `runtime_error` if
                    /// `quantity` exceeds the current view size.
                    ///
                    constexpr auto drop(usize quantity) const
                        -> result<random, runtime_error>
                    {
                        if (quantity > _view_size)
                        {
                            return runtime_error{"Dropping out of bounds"};
                        }
                        else
                        {
                            usize _index = 0;
                            iter_type _result_iter = _begin;
                            
                            for (; _index < quantity; _index++, _result_iter++)
                                ;

                            return random{_result_iter, _end, _view_size - _index};
                        }
                    }

                    ///
                    /// @brief Returns a view limited to the first `quantity`
                    /// steps of the random traversal.
                    ///
                    /// @param quantity - Number of leading steps to keep.
                    /// @return The truncated view, or a `runtime_error` if
                    /// `quantity` exceeds the current view size.
                    ///
                    constexpr auto take(usize quantity) const
                        -> result<random, runtime_error> 
                    {
                        if (quantity > _view_size)
                        {
                            return runtime_error{"Dropping out of bounds"};
                        }
                        else
                        {
                            usize _index = 0;
                            iter_type _result_iter = _begin;
                            
                            for(; _index < quantity; _index++, _result_iter++)
                                ;

                            return random{_begin, _result_iter, _view_size - _index};
                        }
                    }

                    /// @brief Returns the number of elements visible through this view.
                    /// @return The cached `_view_size` value.
                    constexpr usize size() const
                    {
                        return _view_size;
                    }

                    /// @brief Returns the first step of the random traversal.
                    /// @return Iterator equal to `_begin`.
                    constexpr auto begin()
                    {
                        return _begin;
                    }

                    /// @brief Returns the first step of the random traversal.
                    /// @return Iterator equal to `_begin`.
                    constexpr auto begin() const
                    {
                        return _begin;
                    }

                    /// @brief Returns the sentinel step ending the traversal.
                    /// @return Iterator equal to `_end`.
                    constexpr auto end()
                    {
                        return _end;
                    }

                    /// @brief Returns the sentinel step ending the traversal.
                    /// @return Iterator equal to `_end`.
                    constexpr auto end() const
                    {
                        return _end;
                    }
                };

                ///
                /// @brief Deduces the element iterator type from `container.rend()`
                /// so `reverse{container}` does not need an explicit template argument.
                ///
                /// @param container - Container to deduce the reverse view's iterator type from.
                ///
                template <IsReverseContainer U>
                reverse(U&& container) -> reverse<decltype(container.rend())>;
                ///
                /// @brief Deduces the element iterator type from `container.end()`
                /// so `random{container}` does not need an explicit template argument.
                ///
                /// @param container - Container to deduce the random view's iterator type from.
                ///
                template <IsForwardContainer U>
                random(U&& container) -> random<decltype(container.end())>;

                /// @brief Tag type enabling `container | views::reverse` pipe syntax.
                struct _reverse
                {
                    ///
                    /// @brief Builds a `reverse` view over `rhs`.
                    ///
                    /// @param rhs - Container to view in reverse.
                    /// @return A `reverse<...>` view referencing `rhs`'s elements.
                    ///
                    template <IsReverseContainer U>
                    constexpr friend auto operator|(U&& rhs, const _reverse&)
                    {
                        // Reverse iterators are used as they are; element iterators are wrapped.
                        using reverse_type = remove_cvref_t<decltype(rhs.rbegin())>;
                        using iter_type = conditional_t<is_reverse_iterator_v<reverse_type>,
                            reverse_type, remove_cvref_t<decltype(rhs.begin())>>;

                        return reverse<iter_type>{rhs};
                    }
                };

                /// @brief Tag type enabling `container | views::random` pipe syntax.
                struct _random
                {
                    ///
                    /// @brief Builds a `random` view over `rhs`.
                    ///
                    /// @param rhs - Container to traverse in pseudo-random order.
                    /// @return A `random<...>` view referencing `rhs`'s elements.
                    ///
                    template <IsForwardContainer U>
                    constexpr friend auto operator|(U&& rhs, const _random&)
                    {
                        using iter_type = remove_cvref_t<decltype(rhs.begin())>;

                        return random<iter_type>{rhs};
                    }
                };
            } // namespace views_detail
            
            /// @brief Identifies the lazy view types produced by this header:
            /// `span`, `reverse`, and `random`, all keyed on their `value_type`.
            template <typename T>
            concept IsView = (
                IsSame<T, span<typename T::value_type>> ||
                IsSame<T, views_detail::reverse<typename T::value_type>> ||
                IsSame<T, views_detail::random<typename T::value_type>>
            );

            /// @brief Pipe adaptor producing a lazy reverse view: `container | views::reverse`.
            static constexpr views_detail::_reverse reverse = {};
            /// @brief Pipe adaptor producing a lazy pseudo-random view: `container | views::random`.
            static constexpr views_detail::_random random = {};

            /// @brief Pipe adaptor that skips the first `n` elements of a
            /// forward container: `container | views::drop(n)`.
            /// @details Aborts via `panic` if the container has fewer than `n`
            /// elements; the container's element count is read from `.length()`
            /// when available, otherwise `.size()`.
            class drop
            {
            private:
                /// @brief Number of leading elements to skip.
                usize _count;

            public:
                ///
                /// @brief Builds the adaptor with the number of elements to skip.
                ///
                /// @param count - Number of leading elements to skip.
                ///
                constexpr drop(usize count)
                    : _count{count}
                {}

                ///
                /// @brief Skips `rhs._count` elements from the front of `lhs`.
                ///
                /// @param lhs - Container to skip elements from.
                /// @param rhs - Adaptor holding the skip count.
                /// @return An `iter_wrapper` view over the remaining elements.
                /// @note Aborts via `panic("Too few elements")` if `lhs` has
                /// fewer than `rhs._count` elements.
                ///
                template <IsForwardContainer U>
                constexpr friend auto operator|(U&& lhs, const drop& rhs)
                {
                    auto _iter = lhs.begin();
                    usize _cnt = rhs._count;
                    usize _new_size = 0;

                    if constexpr (requires {lhs.length();})
                    {
                        _new_size = lhs.length() - _cnt;
                    }
                    else
                    {
                        _new_size = lhs.size() - _cnt;
                    }

                    while (_cnt != 0)
                    {
                        if (_iter == lhs.end())
                        {
                            panic("Too few elements");
                        }

                        ++_iter; --_cnt;
                    }
                    
                    return views_detail::iter_wrapper{_iter, lhs.end(), _new_size};
                }
            };

            /// @brief Pipe adaptor that skips leading elements while a predicate
            /// holds: `container | views::drop_while(pred)`.
            /// @details Stops at the first element for which `pred` returns
            /// `false` (or, for a fallible predicate, unwraps to `false`), or at
            /// the end of the container; never panics.
            template <typename FuncType>
            class drop_while
            {
            private:
                /// @brief Predicate deciding how many leading elements to skip.
                FuncType _func;

            public:
                ///
                /// @brief Builds the adaptor with the predicate to test elements against.
                ///
                /// @param func - Predicate returning `bool`, or a result unwrapping to `bool`.
                ///
                constexpr drop_while(FuncType func)
                    : _func(hsd::forward<FuncType>(func))
                {}

                ///
                /// @brief Skips leading elements of `lhs` while `rhs._func` holds.
                ///
                /// @param lhs - Container to skip elements from.
                /// @param rhs - Adaptor holding the predicate.
                /// @return An `iter_wrapper` view over the remaining elements.
                ///
                template <IsForwardContainer U>
                constexpr friend auto operator|(U&& lhs, const drop_while& rhs)
                requires (InvocableRet<bool, FuncType, decltype(*lhs.begin())>)
                {
                    auto _iter = lhs.begin();
                    usize _new_size = 0;

                    if constexpr (requires{rhs._func(*lhs.begin()).unwrap();})
                    {
                        for (; _iter != lhs.end() && 
                            rhs._func(*_iter).unwrap(); _iter++)
                            ;
                    }
                    else
                    {
                        for (; _iter != lhs.end() && rhs._func(*_iter); _iter++)
                            ;
                    }

                    for (auto _iter_copy = _iter; _iter_copy != lhs.end(); ++_iter_copy)
                    {
                        ++_new_size;
                    }

                    return views_detail::iter_wrapper{_iter, lhs.end(), _new_size};
                }
            };

            /// @brief Pipe adaptor that keeps only the first `n` elements of a
            /// forward container: `container | views::take(n)`.
            /// @details Aborts via `panic` if the container has fewer than `n` elements.
            class take
            {
            private:
                /// @brief Number of leading elements to keep.
                usize _count;

            public:
                ///
                /// @brief Builds the adaptor with the number of elements to keep.
                ///
                /// @param count - Number of leading elements to keep.
                ///
                constexpr take(usize count)
                    : _count{count}
                {}

                ///
                /// @brief Keeps the first `rhs._count` elements of `lhs`.
                ///
                /// @param lhs - Container to take elements from.
                /// @param rhs - Adaptor holding the element count to keep.
                /// @return An `iter_wrapper` view over the first `rhs._count` elements.
                /// @note Aborts via `panic("Too few elements")` if `lhs` has
                /// fewer than `rhs._count` elements.
                ///
                template <IsForwardContainer U>
                constexpr friend auto operator|(U&& lhs, const take& rhs)
                {
                    auto _iter = lhs.begin();
                    usize _cnt = rhs._count;

                    while (_cnt != 0)
                    {
                        if (_iter == lhs.end())
                        {
                            panic("Too few elements");
                        }

                        ++_iter; --_cnt;
                    }
                    
                    return views_detail::iter_wrapper{lhs.begin(), _iter, rhs._count};
                }
            };

            /// @brief Pipe adaptor that keeps only the leading elements
            /// satisfying a predicate: `container | views::take_while(pred)`.
            /// @details Stops at the first element for which `pred` returns
            /// `false` (or, for a fallible predicate, unwraps to `false`), or at
            /// the end of the container; never panics.
            template <typename FuncType>
            class take_while
            {
            private:
                /// @brief Predicate deciding how many leading elements to keep.
                FuncType _func;

            public:
                ///
                /// @brief Builds the adaptor with the predicate to test elements against.
                ///
                /// @param func - Predicate returning `bool`, or a result unwrapping to `bool`.
                ///
                constexpr take_while(FuncType func)
                    : _func(hsd::forward<FuncType>(func))
                {}

                ///
                /// @brief Keeps the leading elements of `lhs` while `rhs._func` holds.
                ///
                /// @param lhs - Container to take elements from.
                /// @param rhs - Adaptor holding the predicate.
                /// @return An `iter_wrapper` view over the matched leading elements.
                ///
                template <IsForwardContainer U>
                constexpr friend auto operator|(U&& lhs, const take_while& rhs)
                requires (InvocableRet<bool, FuncType, decltype(*lhs.begin())>)
                {
                    auto _iter = lhs.begin();
                    usize _new_size = 0;

                    if constexpr (requires{rhs._func(*lhs.begin()).unwrap();})
                    {
                        for (; _iter != lhs.end() && rhs._func(*_iter).unwrap(); _iter++)
                        {
                            ++_new_size;
                        }
                    }
                    else
                    {
                        for (; _iter != lhs.end() && rhs._func(*_iter); _iter++)
                        {
                            ++_new_size;
                        }
                    }

                    return views_detail::iter_wrapper{lhs.begin(), _iter, _new_size};
                }
            };
        } // namespace views
        
        /// @brief Pipe adaptor that lazily filters elements by predicate:
        /// `container | ranges::filter(pred)`.
        /// @details Elements are visited in-place, so writes through the
        /// resulting view mutate the original container; the wrapped predicate
        /// unwraps a fallible result if one is returned, otherwise uses the
        /// `bool` result directly.
        template <typename FuncType>
        class filter
        {
        private:
            /// @brief Predicate deciding which elements are visible.
            FuncType _func;

        public:
            ///
            /// @brief Builds the adaptor with the predicate to test elements against.
            ///
            /// @param func - Predicate returning `bool`, or a result unwrapping to `bool`.
            ///
            constexpr filter(FuncType func)
                : _func(hsd::forward<FuncType>(func))
            {}

            ///
            /// @brief Builds a lazy filtered view over `lhs`.
            ///
            /// @param lhs - Container to filter.
            /// @param rhs - Adaptor holding the predicate.
            /// @return A `filter_wrapper` view exposing only matching elements.
            ///
            template <IsForwardContainer U>
            constexpr friend auto operator|(U&& lhs, const filter& rhs)
            requires (InvocableRet<bool, FuncType, decltype(*lhs.begin())>)
            {
                auto _filter_func = [rhs](auto&& val)
                {
                    if constexpr (requires{rhs._func(val).unwrap();})
                    {
                        return rhs._func(val).unwrap();
                    }
                    else
                    {
                        return rhs._func(val);
                    }
                };

                return views::views_detail::filter_wrapper{
                    _filter_func, lhs.begin(), lhs.end()
                };
            }
        };

        /// @brief Pipe adaptor that lazily maps elements through a function:
        /// `container | ranges::transform(func)`.
        /// @details The transform is applied on each dereference rather than
        /// eagerly; the wrapped function unwraps a fallible result if one is
        /// returned, otherwise uses the raw result directly.
        template <typename FuncType>
        class transform
        {
        private:
            /// @brief Function applied to each element.
            FuncType _func;

        public:
            ///
            /// @brief Builds the adaptor with the function to apply to elements.
            ///
            /// @param func - Function returning a value convertible to the
            /// element type, or a result unwrapping to one.
            ///
            constexpr transform(FuncType func)
                : _func(hsd::forward<FuncType>(func))
            {}

            ///
            /// @brief Builds a lazy transformed view over `lhs`.
            ///
            /// @param lhs - Container to transform.
            /// @param rhs - Adaptor holding the mapping function.
            /// @return A `transform_wrapper` view exposing the mapped elements.
            ///
            template <IsForwardContainer U> 
            constexpr friend auto operator|(U&& lhs, const transform& rhs)
            requires (InvocableRet<
                remove_cvref_t<decltype(*lhs.begin())>, FuncType, decltype(*lhs.begin())>)
            {
                auto _transform_func = [rhs](auto&& val)
                {
                    if constexpr (requires{rhs._func(val).unwrap();})
                    {
                        return rhs._func(val).unwrap();
                    }
                    else
                    {
                        return rhs._func(val);
                    }
                };

                return views::views_detail::transform_wrapper{
                    _transform_func, lhs.begin(), lhs.end()
                };
            }
        };
    } // namespace ranges

    /// @brief Public alias exposing `ranges::views` adaptors (`drop`, `take`,
    /// `drop_while`, `take_while`, `reverse`, `random`) as `hsd::views`.
    namespace views = ranges::views;
} // namespace hsd
