#include "Result.hpp"
#include "Concepts.hpp"

namespace hsd
{
    namespace switch_detail
    {
        template <typename Func>
        concept InvocableVoid = (
            InvocableRet<void, Func> || requires(Func func) { 
                {func().unwrap()} -> IsSame<void>;
            }
        );
    } // namespace switch_detail

    namespace case_operators
    {
        template <typename T1, typename T2>
        concept EqualComparable = 
            requires(T1 a, T2 b) { {a == b} -> IsSame<bool>; };

        template <typename T1, typename T2>
        concept LessComparable = 
            requires(T1 a, T2 b) { {a < b} -> IsSame<bool>; };

        template <typename T1, typename T2>
        concept GreaterComparable = 
            requires(T1 a, T2 b) { {a > b} -> IsSame<bool>; };

        struct OpEqual
        {
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (EqualComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::Result<bool, ErrType>;

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
                    using ResType = hsd::Result<bool, hsd::runtime_error>;

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

        struct OpLess
        {
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (LessComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::Result<bool, ErrType>;

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
                    using ResType = hsd::Result<bool, hsd::runtime_error>;

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

        struct OpGreater
        {
            template <typename T, typename U, typename Func>
            constexpr auto operator()(
                const T& left, const U& right, const Func& invoke_func)
            requires (GreaterComparable<T, U> && switch_detail::InvocableVoid<Func>)
            {
                if constexpr (requires {invoke_func().unwrap();})
                {
                    using ErrType = decltype(invoke_func().unwrap_err());
                    using ResType = hsd::Result<bool, ErrType>;

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
                    using ResType = hsd::Result<bool, hsd::runtime_error>;

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

    template <typename CaseOp, typename T, typename Func>
    struct CasePair
    {
    private:
        T _value;
        Func _invoke_func;

    public:
        constexpr CasePair(T value, Func invoke_func)
            : _value{value}, _invoke_func{invoke_func}
        {}

        constexpr CasePair(const CasePair&) = default;
        constexpr CasePair& operator=(const CasePair&) = delete;

        template <typename U>
        requires (switch_detail::InvocableVoid<Func>)
        constexpr auto operator()(const U& compare_value) const
        {
            CaseOp _op_func;
            return _op_func(compare_value, _value, _invoke_func);
        }
    };

    template <typename Func>
    struct DefaultCase
    {
    private:
        Func _invoke_func;

    public:
        constexpr DefaultCase(Func invoke_func)
            : _invoke_func{invoke_func}
        {}

        constexpr DefaultCase(const DefaultCase&) = default;
        constexpr DefaultCase& operator=(const DefaultCase&) = delete;

        constexpr auto operator()() const
        requires (switch_detail::InvocableVoid<Func>)
        {
            using ResType = Result<void, runtime_error>;

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

    template <typename T, typename Func> 
    static void switch_cases(const T&, DefaultCase<Func> default_case)
    {
        default_case().unwrap();
    }

    template <typename Op, typename T, typename U, typename Func, typename... Pairs> 
    static void switch_cases(const T& compare_value, 
        CasePair<Op, U, Func> case_pair, Pairs... case_pairs)
    {
        if (case_pair(compare_value).unwrap()) return;
        switch_cases(compare_value, case_pairs...);
    }

    template <typename T, typename Func>
    using EqualCase = CasePair<case_operators::OpEqual, T, Func>;
    template <typename T, typename Func>
    using LessCase = CasePair<case_operators::OpLess, T, Func>;
    template <typename T, typename Func>
    using GreaterCase = CasePair<case_operators::OpGreater, T, Func>;
} // namespace hsd