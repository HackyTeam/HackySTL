#pragma once

/// @file Switch.hpp
/// @brief Compile-time-friendly value dispatch cases.
/// @details Supports equality, less-than, greater-than, and default branches
/// whose callbacks may return void or option_err for explicit failure paths.

#include "Result.hpp"

namespace hsd
{
    namespace switch_detail
    {
        ///
        /// @brief Tests whether `Func` can be invoked as a case callback.
        ///
        /// A case callback matches when it either returns `void` directly or
        /// returns a type whose `.unwrap()` yields `void`, so a case body may
        /// report failure through `option_err`/`result` instead of aborting.
        ///
        /// @tparam Func - Callback type checked for the required shape.
        ///
        template <typename Func>
        concept InvocableVoid = (
            InvocableRet<void, Func> || requires(Func func) { 
                {func().unwrap()} -> IsSame<void>;
            }
        );
    } // namespace switch_detail

    namespace case_operators
    {
        ///
        /// @brief Tests whether `T1` and `T2` support `operator==` returning `bool`.
        ///
        /// @tparam T1 - Left-hand operand type.
        /// @tparam T2 - Right-hand operand type.
        ///
        template <typename T1, typename T2>
        concept EqualComparable = 
            requires(T1 a, T2 b) { {a == b} -> IsSame<bool>; };

        ///
        /// @brief Tests whether `T1` and `T2` support `operator<` returning `bool`.
        ///
        /// @tparam T1 - Left-hand operand type.
        /// @tparam T2 - Right-hand operand type.
        ///
        template <typename T1, typename T2>
        concept LessComparable = 
            requires(T1 a, T2 b) { {a < b} -> IsSame<bool>; };

        ///
        /// @brief Tests whether `T1` and `T2` support `operator>` returning `bool`.
        ///
        /// @tparam T1 - Left-hand operand type.
        /// @tparam T2 - Right-hand operand type.
        ///
        template <typename T1, typename T2>
        concept GreaterComparable = 
            requires(T1 a, T2 b) { {a > b} -> IsSame<bool>; };

        ///
        /// @brief Case comparator that matches when `left == right`.
        ///
        struct op_equal
        {
            ///
            /// @brief Invokes `invoke_func` when `left == right` holds.
            ///
            /// @tparam T - Left-hand operand type.
            /// @tparam U - Right-hand operand type.
            /// @tparam Func - Case callback type.
            /// @param left - Value being matched.
            /// @param right - Case value compared against `left`.
            /// @param invoke_func - Callback run when the values compare equal.
            /// @return A `result<bool, ErrType>` (`ErrType` is `runtime_error`
            /// unless `invoke_func` returns its own error type) that is `true`
            /// when the case matched, `false` when it did not, or an error
            /// forwarded from a failing `invoke_func`.
            ///
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (EqualComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::result<bool, ErrType>;

                    if (left == right)
                    {
                        auto _res = invoke_func();

                        if (!_res.is_ok())
                        {
                            return ResType{_res.unwrap_err()};
                        }
                        else
                        {
                            return ResType{true};
                        }
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
                else
                {
                    using ResType = hsd::result<bool, hsd::runtime_error>;

                    if (left == right)
                    {
                        invoke_func();
                        return ResType{true};
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
            }
        };

        ///
        /// @brief Case comparator that matches when `left < right`.
        ///
        struct op_less
        {
            ///
            /// @brief Invokes `invoke_func` when `left < right` holds.
            ///
            /// @tparam T - Left-hand operand type.
            /// @tparam U - Right-hand operand type.
            /// @tparam Func - Case callback type.
            /// @param left - Value being matched.
            /// @param right - Case value compared against `left`.
            /// @param invoke_func - Callback run when `left < right` holds.
            /// @return A `result<bool, ErrType>` (`ErrType` is `runtime_error`
            /// unless `invoke_func` returns its own error type) that is `true`
            /// when the case matched, `false` when it did not, or an error
            /// forwarded from a failing `invoke_func`.
            ///
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (LessComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::result<bool, ErrType>;

                    if (left < right)
                    {
                        auto _res = invoke_func();

                        if (!_res.is_ok())
                        {
                            return ResType{_res.unwrap_err()};
                        }
                        else
                        {
                            return ResType{true};
                        }
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
                else
                {
                    using ResType = hsd::result<bool, hsd::runtime_error>;

                    if (left < right)
                    {
                        invoke_func();
                        return ResType{true};
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
            }
        };

        ///
        /// @brief Case comparator that matches when `left > right`.
        ///
        struct op_greater
        {
            ///
            /// @brief Invokes `invoke_func` when `left > right` holds.
            ///
            /// @tparam T - Left-hand operand type.
            /// @tparam U - Right-hand operand type.
            /// @tparam Func - Case callback type.
            /// @param left - Value being matched.
            /// @param right - Case value compared against `left`.
            /// @param invoke_func - Callback run when `left > right` holds.
            /// @return A `result<bool, ErrType>` (`ErrType` is `runtime_error`
            /// unless `invoke_func` returns its own error type) that is `true`
            /// when the case matched, `false` when it did not, or an error
            /// forwarded from a failing `invoke_func`.
            ///
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (GreaterComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::result<bool, ErrType>;

                    if (left > right)
                    {
                        auto _res = invoke_func();

                        if (!_res.is_ok())
                        {
                            return ResType{_res.unwrap_err()};
                        }
                        else
                        {
                            return ResType{true};
                        }
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
                else
                {
                    using ResType = hsd::result<bool, hsd::runtime_error>;

                    if (left > right)
                    {
                        invoke_func();
                        return ResType{true};
                    }
                    else
                    {
                        return ResType{false};
                    }
                }
            }
        };
    } // namespace case_operators    

    ///
    /// @brief One dispatch case: a comparison value paired with a callback.
    ///
    /// @tparam CaseOp - Comparator functor (`op_equal`, `op_less`, `op_greater`).
    /// @tparam T - Type of the value this case compares against.
    /// @tparam Func - Callback invoked when the case matches.
    ///
    template <typename CaseOp, typename T, typename Func>
    struct case_pair
    {
    private:
        /// @brief Value compared against the dispatched value.
        T _value;
        /// @brief Callback invoked when the comparison matches.
        Func _invoke_func;

    public:
        ///
        /// @brief Builds a case from a comparison value and its callback.
        ///
        /// @param value - Value compared against the dispatched value.
        /// @param invoke_func - Callback run when the case matches.
        ///
        constexpr case_pair(T value, Func invoke_func)
            : _value{value}, _invoke_func{invoke_func}
        {}

        /// @brief Copies the comparison value and callback.
        constexpr case_pair(const case_pair&) = default;
        constexpr case_pair& operator=(const case_pair&) = delete;

        ///
        /// @brief Tests `compare_value` against this case and runs the callback.
        ///
        /// @tparam U - Type of the dispatched value.
        /// @param compare_value - Value being dispatched.
        /// @return A `result<bool, ErrType>` that is `true` when this case
        /// matched, `false` when it did not, or an error forwarded from a
        /// failing callback.
        ///
        template <typename U>
        requires (switch_detail::InvocableVoid<Func>)
        constexpr auto operator()(const U& compare_value) const
        {
            CaseOp _op_func;
            return _op_func(compare_value, _value, _invoke_func);
        }
    };

    ///
    /// @brief Fallback branch invoked when no preceding case matched.
    ///
    /// @tparam Func - Callback invoked unconditionally.
    ///
    template <typename Func>
    struct default_case
    {
    private:
        /// @brief Callback invoked unconditionally.
        Func _invoke_func;

    public:
        ///
        /// @brief Builds a default branch from its callback.
        ///
        /// @param invoke_func - Callback run when this branch is reached.
        ///
        constexpr default_case(Func invoke_func)
            : _invoke_func{invoke_func}
        {}

        /// @brief Copies the unconditional callback.
        constexpr default_case(const default_case&) = default;
        constexpr default_case& operator=(const default_case&) = delete;

        ///
        /// @brief Runs the callback unconditionally.
        ///
        /// @return An empty `option_err<runtime_error>` (or the callback's own
        /// error type) that is empty on success or holds a forwarded error.
        ///
        constexpr auto operator()() const
        requires (switch_detail::InvocableVoid<Func>)
        {
            using ResType = option_err<runtime_error>;

            if constexpr (requires {_invoke_func().unwrap();})
            {
                return _invoke_func();
            }
            else
            {
                _invoke_func();
                return ResType{};
            }
        }
    };

    ///
    /// @brief Terminal overload run once no case before the default matched.
    ///
    /// @tparam T - Type of the dispatched value.
    /// @tparam Func - Callback type of the default branch.
    /// @param default_case - Default branch, run unconditionally.
    ///
    template <typename T, typename Func> 
    static constexpr void switch_cases(const T&, default_case<Func> default_case)
    {
        default_case().unwrap();
    }

    ///
    /// @brief Tests `compare_value` against `case_pair` and, on a miss,
    /// recurses into the remaining cases.
    ///
    /// @tparam Op - Comparator functor of the current case.
    /// @tparam T - Type of the dispatched value.
    /// @tparam U - Type of the current case's comparison value.
    /// @tparam Func - Callback type of the current case.
    /// @tparam Pairs - Types of the remaining cases (and default branch).
    /// @param compare_value - Value being dispatched.
    /// @param case_pair - Current case tested against `compare_value`.
    /// @param case_pairs - Remaining cases, tried when this one does not match.
    ///
    template <typename Op, typename T, typename U, typename Func, typename... Pairs> 
    static constexpr void switch_cases(const T& compare_value, 
        case_pair<Op, U, Func> case_pair, Pairs... case_pairs)
    {
        if (case_pair(compare_value).unwrap()) return;
        switch_cases(compare_value, case_pairs...);
    }

    ///
    /// @brief Dispatch case that matches by equality.
    ///
    /// @tparam T - Type of the value this case compares against.
    /// @tparam Func - Callback invoked when the case matches.
    ///
    template <typename T, typename Func>
    class equal_case : 
        public case_pair<case_operators::op_equal, T, Func>
    {
    private:
        /// @brief Base case_pair specialized for equality comparison.
        using _Base = case_pair<case_operators::op_equal, T, Func>;

    public:
        ///
        /// @brief Builds an equality case from a value and its callback.
        ///
        /// @param value - Value compared against the dispatched value.
        /// @param invoke_func - Callback run when the values compare equal.
        ///
        constexpr equal_case(const T& value, const Func& invoke_func)
            : _Base{value, invoke_func}
        {}

        /// @brief Copies the comparison value and callback.
        /// @param other - Case copied from.
        constexpr equal_case(const equal_case& other) = default;
        constexpr equal_case& operator=(const equal_case&) = delete;
    };
    
    ///
    /// @brief Dispatch case that matches when the dispatched value is less.
    ///
    /// @tparam T - Type of the value this case compares against.
    /// @tparam Func - Callback invoked when the case matches.
    ///
    template <typename T, typename Func>
    class less_case : 
        public case_pair<case_operators::op_less, T, Func>
    {
    private:
        /// @brief Base case_pair specialized for less-than comparison.
        using _Base = case_pair<case_operators::op_less, T, Func>;

    public:
        ///
        /// @brief Builds a less-than case from a value and its callback.
        ///
        /// @param value - Value compared against the dispatched value.
        /// @param invoke_func - Callback run when the dispatched value is less.
        ///
        constexpr less_case(const T& value, const Func& invoke_func)
            : _Base{value, invoke_func}
        {}

        /// @brief Copies the comparison value and callback.
        /// @param other - Case copied from.
        constexpr less_case(const less_case& other) = default;
        constexpr less_case& operator=(const less_case&) = delete;
    };
    
    ///
    /// @brief Dispatch case that matches when the dispatched value is greater.
    ///
    /// @tparam T - Type of the value this case compares against.
    /// @tparam Func - Callback invoked when the case matches.
    ///
    template <typename T, typename Func>
    class greater_case : 
        public case_pair<case_operators::op_greater, T, Func>
    {
    private:
        /// @brief Base case_pair specialized for greater-than comparison.
        using _Base = case_pair<case_operators::op_greater, T, Func>;

    public:
        ///
        /// @brief Builds a greater-than case from a value and its callback.
        ///
        /// @param value - Value compared against the dispatched value.
        /// @param invoke_func - Callback run when the dispatched value is greater.
        ///
        constexpr greater_case(const T& value, const Func& invoke_func)
            : _Base{value, invoke_func}
        {}

        /// @brief Copies the comparison value and callback.
        /// @param other - Case copied from.
        constexpr greater_case(const greater_case& other) = default;
        constexpr greater_case& operator=(const greater_case&) = delete;
    };

    /// @brief Deduces `equal_case` from a value and callback argument pair.
    template <typename T, typename Func>
    equal_case(T, Func) -> equal_case<decay_t<T>, Func>;
    /// @brief Deduces `less_case` from a value and callback argument pair.
    template <typename T, typename Func>
    less_case(T, Func) -> less_case<decay_t<T>, Func>;
    /// @brief Deduces `greater_case` from a value and callback argument pair.
    template <typename T, typename Func>
    greater_case(T, Func) -> greater_case<decay_t<T>, Func>;
} // namespace hsd