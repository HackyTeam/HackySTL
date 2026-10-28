#pragma once

/// @file Algorithm.hpp
/// @brief Small iterator-based algorithms used throughout HackySTL.
/// @details Provides constexpr-friendly traversal, copying, moving, searching,
/// and assignment helpers for the project containers and wrappers.

#include "Utility.hpp"
#include "Concepts.hpp"

namespace hsd
{
    /// @brief Constrains a type to a sized, indexable container of unsigned
    /// integers, usable directly as the sort key by `radix_sort`. Signed and
    /// floating-point keys are rejected, since digit extraction is only defined
    /// for non-negative integers.
    template <typename T>
    concept DigitVector = requires(T a) {
        {a.size()} -> IsSame<usize>;
        {static_cast<remove_reference_t<decltype(a[0]) >> (a[0])} -> IsUnsigned;
    };

    /// @brief Constrains a type to a sized, indexable container whose elements
    /// yield an unsigned integer key through a separate `Getter` callable.
    template <typename T, typename Getter>
    concept DigitVectorGetter = requires(T a, Getter getter) {
        {a.size()} -> IsSame<usize>;
        {a[0]};
        {getter(a[0])} -> IsUnsigned;
    };

    /// @brief Constrains a type to a sized, indexable container.
    template <typename T>
    concept ArrayLike = requires(T a) {
        {a.size()} -> IsSame<usize>;
        {a[0]};
    };

    /// @brief Constrains a type to a sized, indexable container.
    /// @details Sorts that take a predicate need nothing more.
    template <typename T>
    concept ArrayLikeSized = requires(T a) {
        {a.size()} -> IsSame<usize>;
        {a[0]};
    };

    /// @brief Constrains a type to a sized, indexable container whose elements
    /// support `operator>`, which the default-order sorts require.
    template <typename T>
    concept ArrayLikeComparable = ArrayLikeSized<T> && requires(T a) {
        {a[0] > a[1]};
    };

    ///
    /// @brief One counting-sort pass of LSD radix sort, ordering elements by a
    /// single decimal digit of their key.
    ///
    /// The pass is stable: elements whose keys share a digit keep their
    /// relative order, so passes from the least significant place upward build
    /// a full sort. Keys are unsigned integers, and `exp` has the key's type,
    /// so `exp * 10` is never formed past the largest key.
    ///
    /// @tparam Vec - Array-like container sorted in place.
    ///
    template <typename Vec>
    struct count_sorter
    {
        ///
        /// @brief Stably reorders `arr` by the decimal digit at place value `exp`.
        ///
        /// @param arr - Container sorted in place.
        /// @param exp - Power-of-ten place value examined (1, 10, 100, ...).
        ///
        static constexpr void sort(Vec& arr, auto exp)
        {
            sort(arr, exp, [](const auto& value) { return value; });
        }

        ///
        /// @brief Overload of `sort()` ordering by a digit extracted from each
        /// element via `get_digit` rather than the element's own value.
        ///
        /// @param arr - Container sorted in place.
        /// @param exp - Power-of-ten place value examined (1, 10, 100, ...).
        /// @param get_digit - Callable extracting the unsigned key from an element.
        ///
        static constexpr void sort(Vec& arr, auto exp, auto&& get_digit)
        {
            const usize _size = arr.size();

            if (_size < 2)
            {
                return;
            }

            Vec _output = {};

            if constexpr (requires {_output.push_back(arr[0]);})
            {
                typename Vec::value_type _fill = {};

                for (usize _index = 0; _index < _size; _index++)
                {
                    _output.push_back(_fill);
                }
            }

            usize _count[10] = {};

            for (usize _index = 0; _index < _size; _index++)
            {
                _count[_digit_of(get_digit(arr[_index]), exp)]++;
            }

            // Prefix sums turn each count into the end of that digit's run.
            for (usize _digit = 1; _digit < 10; _digit++)
            {
                _count[_digit] += _count[_digit - 1];
            }

            // Walking from the back keeps equal digits in their original order.
            for (usize _index = _size; _index-- > 0;)
            {
                const usize _digit = _digit_of(get_digit(arr[_index]), exp);
                _output[--_count[_digit]] = hsd::move(arr[_index]);
            }

            for (usize _index = 0; _index < _size; _index++)
            {
                arr[_index] = hsd::move(_output[_index]);
            }
        }

        ///
        /// @brief Extracts the decimal digit of `key` at place value `exp`.
        ///
        /// @param key - Unsigned key being examined.
        /// @param exp - Power-of-ten place value (1, 10, 100, ...).
        /// @return The digit, in `[0, 9]`.
        ///
        static constexpr usize _digit_of(auto key, auto exp)
        {
            return static_cast<usize>((key / exp) % 10);
        }
    };

    ///
    /// @brief LSD radix sort over unsigned keys, in ascending order.
    ///
    /// One counting pass runs for each decimal place up to the largest key, so
    /// sorting `n` elements with `d`-digit keys takes O(n * d) time. Equal keys
    /// keep their original order. Empty and single-element containers are left
    /// untouched.
    ///
    /// @tparam Vec - Array-like container sorted in place.
    ///
    template <typename Vec>
    struct radix_sorter
    {
        ///
        /// @brief Sorts `arr` by the key that `get_digit` extracts from each
        /// element.
        ///
        /// @param arr - Container sorted in place.
        /// @param pred - Comparator used only to locate the largest key, which
        /// fixes how many passes run. The result is always ascending by key, so
        /// `pred` must order elements the way `operator>` orders their keys.
        /// @param get_digit - Callable extracting the unsigned key from an element.
        ///
        static constexpr void sort(Vec& arr, auto&& pred, auto&& get_digit)
        {
            const usize _size = arr.size();

            if (_size < 2)
            {
                return;
            }

            usize _largest = 0;

            for (usize _index = 1; _index < _size; _index++)
            {
                if (pred(arr[_index], arr[_largest]))
                {
                    _largest = _index;
                }
            }

            auto _max_key = get_digit(arr[_largest]);
            decltype(_max_key) _exp = 1;

            while (_max_key / _exp > 0)
            {
                count_sorter<Vec>::sort(arr, _exp, get_digit);

                // Stop before `_exp * 10` could overflow the key type.
                if (_exp > _max_key / 10)
                {
                    return;
                }

                _exp *= 10;
            }
        }

        ///
        /// @brief Overload of `sort()` ordering elements by their own value.
        ///
        /// @param arr - Container sorted in place.
        /// @param pred - Comparator used to locate the largest element; it must
        /// order elements the way `operator>` does.
        ///
        static constexpr void sort(Vec& arr, auto&& pred)
        {
            sort(arr, pred, [](const auto& value) { return value; });
        }
    };

    ///
    /// @brief Sorts `arr` in ascending order using LSD radix sort over decimal digits.
    ///
    /// @tparam Vec - `DigitVector`-conforming container sorted in place.
    /// @param arr - Container sorted in place. Empty and single-element
    /// containers are left untouched.
    ///
    template <DigitVector Vec>
    static constexpr void radix_sort(Vec& arr)
    {
        radix_sorter<Vec>::sort(arr, [](const auto& lhs, const auto& rhs)
        {
            return lhs > rhs;
        });
    }

    ///
    /// @brief Overload of `radix_sort()` locating the largest element with `pred`.
    ///
    /// @tparam Vec - `DigitVector`-conforming container sorted in place.
    /// @param arr - Container sorted in place. Empty and single-element
    /// containers are left untouched.
    /// @param pred - Comparator that orders elements the way `operator>` does.
    /// The result is always ascending, so `pred` only needs to find the largest.
    ///
    template <DigitVector Vec>
    static constexpr void radix_sort(Vec& arr, auto&& pred)
    {
        radix_sorter<Vec>::sort(arr, pred);
    }

    ///
    /// @brief Overload of `radix_sort()` ordering by a key extracted from each
    /// element via `get_digit` rather than the element's own value.
    ///
    /// @tparam Vec - Container sorted in place.
    /// @tparam Getter - Callable type satisfying `DigitVectorGetter<Vec, Getter>`.
    /// @param arr - Container sorted in place. Empty and single-element
    /// containers are left untouched.
    /// @param pred - Comparator used only to locate the largest key; it must
    /// order keys the way `operator>` does, since the result is always ascending.
    /// @param get_digit - Callable extracting the unsigned key from an element.
    ///
    template <typename Vec, typename Getter>
    requires (DigitVectorGetter<Vec, Getter>)
    static constexpr void radix_sort(Vec& arr, auto&& pred, Getter&& get_digit)
    {
        radix_sorter<Vec>::sort(arr, pred, get_digit);
    }

    ///
    /// @brief Insertion sort over an inclusive index range.
    ///
    /// Quadratic in the worst case, but fast on short or nearly sorted ranges,
    /// which is why `introsorter` uses it to finish small ranges. Equal keys
    /// keep their relative order.
    ///
    /// Orderings are expressed as "`pred(a, b)` holds when `a` must come after
    /// `b`", which `operator>` satisfies for ascending order.
    ///
    /// @tparam Vec - Array-like container sorted in place.
    ///
    template <typename Vec>
    struct insertion_sorter
    {
        ///
        /// @brief Sorts `vec[low..high]` in place using `operator>` ordering.
        ///
        /// @param vec - Container sorted in place.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, usize low, usize high)
        {
            auto _greater = [](const auto& lhs, const auto& rhs)
            {
                return lhs > rhs;
            };

            sort(vec, _greater, low, high);
        }

        ///
        /// @brief Overload of `sort()` ordering elements with `pred` instead of
        /// `operator>`.
        ///
        /// @param vec - Container sorted in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, auto& pred, usize low, usize high)
        {
            if (low >= high) return;

            for (usize _index = low + 1; _index <= high; ++_index)
            {
                if (!pred(vec[_index - 1], vec[_index])) continue;

                auto _moving = hsd::move(vec[_index]);
                usize _pos = _index;

                do
                {
                    vec[_pos] = hsd::move(vec[_pos - 1]);
                    --_pos;
                }
                while (_pos > low && pred(vec[_pos - 1], _moving));

                vec[_pos] = hsd::move(_moving);
            }
        }
    };

    ///
    /// @brief Heap sort over an inclusive index range.
    ///
    /// Builds a max-heap, then repeatedly moves its largest element to the end
    /// of the range. The cost is O(n log n) for every input, but the sort is not
    /// stable. `introsorter` falls back to it when partitioning keeps splitting
    /// badly.
    ///
    /// Orderings are expressed as "`pred(a, b)` holds when `a` must come after
    /// `b`", which `operator>` satisfies for ascending order.
    ///
    /// @tparam Vec - Array-like container sorted in place.
    ///
    template <typename Vec>
    struct heap_sorter
    {
    private:
        ///
        /// @brief Restores the max-heap property below `root` in a heap that
        /// occupies `count` elements starting at `low`.
        ///
        /// @param vec - Container reordered in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Index of the heap's first element.
        /// @param root - Heap position (relative to `low`) to sift down.
        /// @param count - Number of elements in the heap.
        ///
        static constexpr void _sift_down(
            Vec& vec, auto& pred, usize low, usize root, usize count)
        {
            while (true)
            {
                usize _child = 2 * root + 1;

                if (_child >= count) return;

                if (_child + 1 < count
                    && pred(vec[low + _child + 1], vec[low + _child]))
                {
                    ++_child;
                }

                if (!pred(vec[low + _child], vec[low + root])) return;

                hsd::swap(vec[low + root], vec[low + _child]);
                root = _child;
            }
        }

    public:
        ///
        /// @brief Sorts `vec[low..high]` in place using `operator>` ordering.
        ///
        /// @param vec - Container sorted in place.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, usize low, usize high)
        {
            auto _greater = [](const auto& lhs, const auto& rhs)
            {
                return lhs > rhs;
            };

            sort(vec, _greater, low, high);
        }

        ///
        /// @brief Overload of `sort()` ordering elements with `pred` instead of
        /// `operator>`.
        ///
        /// @param vec - Container sorted in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, auto& pred, usize low, usize high)
        {
            if (low >= high) return;

            const usize _count = high - low + 1;

            for (usize _root = _count / 2; _root > 0; --_root)
            {
                _sift_down(vec, pred, low, _root - 1, _count);
            }

            for (usize _end = _count - 1; _end > 0; --_end)
            {
                hsd::swap(vec[low], vec[low + _end]);
                _sift_down(vec, pred, low, 0, _end);
            }
        }
    };

    ///
    /// @brief Partitioning step of quicksort over an inclusive index range.
    ///
    /// Ranges are split around a median-of-three pivot with a Hoare partition
    /// that stops on keys equal to the pivot, so runs of duplicates still split
    /// evenly. `introsorter` decides which side to recurse into.
    ///
    /// Orderings are expressed as "`pred(a, b)` holds when `a` must come after
    /// `b`", which `operator>` satisfies for ascending order.
    ///
    /// @tparam Vec - Array-like container partitioned in place.
    ///
    template <typename Vec>
    struct quick_sorter
    {
    private:
        ///
        /// @brief Ranks the elements at `low`, `middle` and `high` so that the
        /// median ends up at `low`, ready to serve as the partition pivot.
        ///
        /// @param vec - Container reordered in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Inclusive lower index; receives the median.
        /// @param high - Inclusive upper index; must be greater than `low + 1`.
        ///
        static constexpr void _median_to_front(
            Vec& vec, auto& pred, usize low, usize high)
        {
            const usize middle = low + (high - low) / 2;

            if (pred(vec[low], vec[middle])) swap(vec[low], vec[middle]);
            if (pred(vec[middle], vec[high])) swap(vec[middle], vec[high]);
            if (pred(vec[low], vec[middle])) swap(vec[low], vec[middle]);

            // vec[low] <= vec[middle] <= vec[high]: the median is in the middle.
            swap(vec[low], vec[middle]);
        }

    public:
        ///
        /// @brief Partitions `vec[low..high]` around a median-of-three pivot.
        ///
        /// Afterwards the pivot sits at the returned index, no element before
        /// it must come after it, and no element after it must come before it.
        ///
        /// @param vec - Container partitioned in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Inclusive lower index of the range to partition.
        /// @param high - Inclusive upper index of the range to partition; must
        /// not be below `low`. A range of two elements is simply put in order.
        /// @return Final index of the pivot, which splits the range for recursion.
        ///
        static constexpr usize partition(Vec& vec, auto& pred, usize low, usize high)
        {
            if (high - low < 2)
            {
                if (pred(vec[low], vec[high])) swap(vec[low], vec[high]);

                return low;
            }

            _median_to_front(vec, pred, low, high);

            // The median-of-three leaves a maximum at `high`, which stops the
            // left scan, and the pivot itself at `low`, which stops the right
            // scan, so neither scan needs a bounds check. Both stop on keys
            // equal to the pivot.
            usize _left = low;
            usize _right = high + 1;

            for (;;)
            {
                do { ++_left; } while (pred(vec[low], vec[_left]));
                do { --_right; } while (pred(vec[_right], vec[low]));

                if (_left >= _right) break;

                hsd::swap(vec[_left], vec[_right]);
            }

            hsd::swap(vec[low], vec[_right]);
            return _right;
        }

        ///
        /// @brief Partitions `vec[low..high]` ordering elements with `operator>`.
        ///
        /// @param vec - Container partitioned in place.
        /// @param low - Inclusive lower index of the range to partition.
        /// @param high - Inclusive upper index of the range to partition.
        /// @return Final index of the pivot, which splits the range for recursion.
        ///
        static constexpr usize partition(Vec& vec, usize low, usize high)
        {
            auto _greater = [](const auto& lhs, const auto& rhs)
            {
                return lhs > rhs;
            };

            return partition(vec, _greater, low, high);
        }
    };

    ///
    /// @brief Introspective sort over an inclusive index range.
    ///
    /// Ranges are partitioned by `quick_sorter`, and the recursion always
    /// descends into the smaller side first so its depth stays logarithmic.
    /// Short ranges finish with `insertion_sorter`, and `heap_sorter` takes over
    /// when partitioning keeps splitting badly. The sort is therefore O(n log n)
    /// for every input and is not stable.
    ///
    /// Orderings are expressed as "`pred(a, b)` holds when `a` must come after
    /// `b`", which `operator>` satisfies for ascending order.
    ///
    /// @tparam Vec - Array-like container sorted in place.
    ///
    template <typename Vec>
    struct introsorter
    {
    private:
        /// @brief Ranges of at most this many elements are insertion sorted.
        static constexpr usize small_range = 16;

        ///
        /// @brief Gets the partitioning depth allowed before heap sort is used.
        ///
        /// @param count - Number of elements in the range being sorted.
        /// @return Twice the base-2 logarithm of `count`, rounded down.
        ///
        static constexpr usize _depth_limit(usize count)
        {
            usize _depth = 0;

            for (; count > 1; count >>= 1) ++_depth;

            return 2 * _depth;
        }

        ///
        /// @brief Sorts `vec[low..high]` by partitioning, always recursing into
        /// the smaller part and looping on the larger one.
        ///
        /// @param vec - Container sorted in place.
        /// @param pred - Strict-ordering comparator; `pred(a, b)` holds when
        /// `a` must come after `b`.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort.
        /// @param depth - Remaining partitioning levels before heap sort.
        ///
        static constexpr void _introsort(
            Vec& vec, auto& pred, usize low, usize high, usize depth)
        {
            while (low < high && high - low >= small_range)
            {
                if (depth == 0)
                {
                    heap_sorter<Vec>::sort(vec, pred, low, high);
                    return;
                }

                --depth;

                const usize _middle = quick_sorter<Vec>::partition(vec, pred, low, high);

                if (_middle - low < high - _middle)
                {
                    if (_middle > low)
                    {
                        _introsort(vec, pred, low, _middle - 1, depth);
                    }

                    low = _middle + 1;
                }
                else
                {
                    if (_middle < high)
                    {
                        _introsort(vec, pred, _middle + 1, high, depth);
                    }

                    if (_middle == low) return;

                    high = _middle - 1;
                }
            }

            if (low < high)
            {
                insertion_sorter<Vec>::sort(vec, pred, low, high);
            }
        }

    public:
        ///
        /// @brief Sorts `vec[low..high]` in place using `operator>` ordering.
        ///
        /// @param vec - Container sorted in place.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, usize low, usize high)
        {
            if (low >= high) return;

            auto _greater = [](const auto& lhs, const auto& rhs)
            {
                return lhs > rhs;
            };

            _introsort(vec, _greater, low, high, _depth_limit(high - low + 1));
        }

        ///
        /// @brief Overload of `sort()` ordering elements with `pred` instead
        /// of `operator>`.
        ///
        /// @param vec - Container sorted in place.
        /// @param pred - Strict-ordering comparator; elements are moved so that
        /// `pred(a, b)` never holds for an `a` placed before a `b`.
        /// @param low - Inclusive lower index of the range to sort.
        /// @param high - Inclusive upper index of the range to sort; the
        /// function returns immediately once `low >= high`.
        ///
        static constexpr void sort(Vec& vec, auto& pred, usize low, usize high)
        {
            if (low >= high) return;

            _introsort(vec, pred, low, high, _depth_limit(high - low + 1));
        }
    };

    ///
    /// @brief Gets the number of elements the free sort functions cover.
    ///
    /// @tparam Vec - Container whose elements are counted.
    /// @param arr - Container measured.
    /// @return `length()` when the container has one, otherwise `size()`.
    ///
    template <typename Vec>
    static constexpr usize _sort_count(Vec& arr)
    {
        if constexpr (requires {arr.length();})
        {
            return arr.length();
        }
        else
        {
            return arr.size();
        }
    }

    ///
    /// @brief Sorts `arr` in place using insertion sort and `operator>` ordering.
    ///
    /// @tparam Vec - `ArrayLikeComparable`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    ///
    template <ArrayLikeComparable Vec>
    static constexpr void insertion_sort(Vec& arr)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            insertion_sorter<Vec>::sort(arr, 0, _count - 1);
        }
    }

    ///
    /// @brief Overload of `insertion_sort()` ordering elements with `pred`
    /// instead of `operator>`.
    ///
    /// @tparam Vec - `ArrayLikeSized`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    /// @param pred - Strict-ordering comparator forwarded to `insertion_sorter`.
    ///
    template <ArrayLikeSized Vec>
    static constexpr void insertion_sort(Vec& arr, auto&& pred)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            insertion_sorter<Vec>::sort(arr, pred, 0, _count - 1);
        }
    }

    ///
    /// @brief Sorts `arr` in place using heap sort and `operator>` ordering.
    ///
    /// @tparam Vec - `ArrayLikeComparable`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    ///
    template <ArrayLikeComparable Vec>
    static constexpr void heap_sort(Vec& arr)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            heap_sorter<Vec>::sort(arr, 0, _count - 1);
        }
    }

    ///
    /// @brief Overload of `heap_sort()` ordering elements with `pred` instead
    /// of `operator>`.
    ///
    /// @tparam Vec - `ArrayLikeSized`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    /// @param pred - Strict-ordering comparator forwarded to `heap_sorter`.
    ///
    template <ArrayLikeSized Vec>
    static constexpr void heap_sort(Vec& arr, auto&& pred)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            heap_sorter<Vec>::sort(arr, pred, 0, _count - 1);
        }
    }

    ///
    /// @brief Sorts `arr` in place using introspective sort and `operator>`
    /// ordering.
    ///
    /// @tparam Vec - `ArrayLikeComparable`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    ///
    template <ArrayLikeComparable Vec>
    static constexpr void introsort(Vec& arr)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            introsorter<Vec>::sort(arr, 0, _count - 1);
        }
    }

    ///
    /// @brief Overload of `introsort()` ordering elements with `pred` instead
    /// of `operator>`.
    ///
    /// @tparam Vec - `ArrayLikeSized`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    /// @param pred - Strict-ordering comparator forwarded to `introsorter`.
    ///
    template <ArrayLikeSized Vec>
    static constexpr void introsort(Vec& arr, auto&& pred)
    {
        const usize _count = _sort_count(arr);

        if (_count > 1)
        {
            introsorter<Vec>::sort(arr, pred, 0, _count - 1);
        }
    }

    ///
    /// @brief Sorts `arr` in place using `operator>` ordering; the same algorithm
    /// as `introsort()`, kept under its original name.
    ///
    /// @tparam Vec - `ArrayLikeComparable`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    ///
    template <ArrayLikeComparable Vec>
    static constexpr void quick_sort(Vec& arr)
    {
        introsort(arr);
    }

    ///
    /// @brief Overload of `quick_sort()` ordering elements with `pred` instead
    /// of `operator>`.
    ///
    /// @tparam Vec - `ArrayLikeSized`-conforming container sorted in place.
    /// @param arr - Container sorted in place; uses `length()` when available,
    /// otherwise `size()`, to determine the upper bound. Empty and
    /// single-element containers are left untouched.
    /// @param pred - Strict-ordering comparator forwarded to `introsorter`.
    ///
    template <ArrayLikeSized Vec>
    static constexpr void quick_sort(Vec& arr, auto&& pred)
    {
        introsort(arr, pred);
    }
} // namespace hsd
