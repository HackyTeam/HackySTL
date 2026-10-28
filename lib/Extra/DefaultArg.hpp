#pragma once

/// @file DefaultArg.hpp
/// @brief Default-argument emulation for ordinary callables.
/// @details Stores a callable and fallback argument tuple, replacing selected
/// call positions with `default_v` at invocation time.

// Requires C++20

#include "../Wrappers/Functional.hpp"

/** @def default_cast_func
 *  @brief Local alias for the position-specific default-cast helper.
 *  @details The macro is defined only inside the callable implementation.
 */

namespace hsd
{
    /// @brief Tag type marking a call argument that should use its stored default.
    struct default_t {};
    /// @brief Placeholder value passed at a call site to request the stored default.
    constexpr default_t default_v{};

    /// @brief Primary template, specialized on a `Result(Args...)` call signature.
    template <typename S, typename F>
    struct default_call_t;

    /// @brief Forward declaration of the `Result(Args...)` specialization.
    template <typename F, typename Result, typename... Args>
    struct default_call_t<Result(Args...), F>;

    template <usize Id, typename F, typename Result, typename... Args, typename T>
    constexpr auto default_cast(default_call_t<Result(Args...), F> const&, T&& x);

    template <usize Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(default_call_t<Result(Args...), F> const& c, default_t);

    ///
    /// @brief Callable wrapper that substitutes `default_v` call-site arguments
    /// with pre-stored fallback values before invoking the wrapped callable.
    ///
    /// @tparam F - Wrapped callable type.
    /// @tparam Result - Return type of the wrapped call signature.
    /// @tparam Args - Parameter types of the wrapped call signature.
    ///
    template <typename F, typename Result, typename... Args>
    struct default_call_t<Result(Args...), F>
    {
        /// @brief Fallback argument values substituted for `default_v` positions.
        tuple<Args...> default_args;
        /// @brief Wrapped callable invoked with the resolved arguments.
        F func;

        ///
        /// @brief Stores the wrapped callable and its fallback arguments.
        ///
        /// @param func - Callable invoked by `operator()`.
        /// @param args - Fallback values stored for `default_v` positions.
        ///
        constexpr default_call_t(F&& func, Args&&... args) 
            : default_args(hsd::forward<Args>(args)...), func(hsd::forward<F>(func)) 
        {}

        ///
        /// @brief Invokes the wrapped callable, resolving each argument via
        /// `default_cast` (passthrough for real values, stored default for
        /// `default_v`), and unwrapping the result if it is itself invocable.
        ///
        /// @tparam U - Deduced argument types supplied at the call site.
        /// @param args - Per-position arguments; `default_v` selects the
        /// stored fallback for that position.
        /// @return The wrapped callable's result, unwrapped via `.unwrap()`
        /// when the resolved call is itself `UnwrapInvocable`.
        ///
        template <typename... U>
        constexpr Result operator()(U&&... args) const 
        {
            // Because GCC is stupid and doesn't support proper capturing
            return [this]<usize... Seq>(index_sequence<Seq...>, auto&&... args) 
            {
                #define default_cast_func default_cast<Seq, F, Result, Args...>
                
                if constexpr (UnwrapInvocable<
                    F, decltype(default_cast_func(*this, hsd::forward<U>(args)))...
                >)
                {
                    return func(default_cast_func(*this, hsd::forward<U>(args))...).unwrap();
                }
                else
                {
                    return func(default_cast_func(*this, hsd::forward<U>(args))...);
                }

                #undef default_cast_func
            }(index_sequence_for<Args...>{}, hsd::forward<U>(args)...);
        }
    };  

    ///
    /// @brief Builds a `default_call_t` for an explicit call signature `S`.
    ///
    /// @tparam S - Call signature in `Result(Args...)` form.
    /// @tparam F - Wrapped callable type.
    /// @tparam Args - Fallback argument types stored for `default_v` positions.
    /// @param func - Callable invoked by the returned wrapper.
    /// @param args - Fallback values stored for `default_v` positions.
    /// @return A `default_call_t<S, F>` wrapping `func` and `args`.
    ///
    template <typename S, typename F, typename... Args>
    default_call_t<S, F> make_defaultcall(F&& func, Args&&... args)
    {
        return default_call_t<S, F>(hsd::forward<F>(func), hsd::forward<Args>(args)...);
    }

    ///
    /// @brief Passes a non-placeholder argument through unchanged.
    ///
    /// @tparam Id - Zero-based position of the argument within `Args...`.
    /// @tparam F - Wrapped callable type.
    /// @tparam Result - Return type of the wrapped call signature.
    /// @tparam Args - Parameter types of the wrapped call signature.
    /// @tparam T - Deduced argument type; excluded when it is `default_t`.
    /// @param x - Argument supplied by the caller.
    /// @return `x`, forwarded unchanged.
    ///
    template <usize Id, typename F, typename Result, typename... Args, typename T>
    requires (!IsSame<T, default_t>)
    constexpr auto default_cast(default_call_t<Result(Args...), F> const&, T&& x) 
    {
        return hsd::forward<T>(x);
    }

    ///
    /// @brief Substitutes the stored default for position `Id`.
    ///
    /// @tparam Id - Zero-based position of the argument within `Args...`.
    /// @tparam F - Wrapped callable type.
    /// @tparam Result - Return type of the wrapped call signature.
    /// @tparam Args - Parameter types of the wrapped call signature.
    /// @param c - Call wrapper whose `default_args` supplies the fallback value.
    /// @return The stored default argument at position `Id`.
    ///
    template <usize Id, typename F, typename Result, typename... Args>
    constexpr auto default_cast(default_call_t<Result(Args...), F> const& c, default_t)
    {
        return c.default_args.template get<Id>();
    }

    /// @brief Deduction guide for plain function pointers.
    /// @param args - Fallback values stored for `default_v` positions.
    template <typename Res, typename... Args>
    default_call_t(Res(*)(Args...), Args&&... args)
        -> default_call_t<Res(Args...), Res(*)(Args...)>;

    /// @brief Deduction guide for `noexcept` function pointers.
    /// @param args - Fallback values stored for `default_v` positions.
    template <typename Res, typename... Args>
    default_call_t(Res(*)(Args...) noexcept, Args&&... args)
        -> default_call_t<Res(Args...), Res(*)(Args...) noexcept>;

    /// @brief Deduction guide for function references.
    /// @param args - Fallback values stored for `default_v` positions.
    template <typename Res, typename... Args>
    default_call_t(Res(&)(Args...), Args&&... args)
        -> default_call_t<Res(Args...), Res(&)(Args...)>;

    /// @brief Deduction guide for `noexcept` function references.
    /// @param args - Fallback values stored for `default_v` positions.
    template <typename Res, typename... Args>
    default_call_t(Res(&)(Args...) noexcept, Args&&... args)
        -> default_call_t<Res(Args...), Res(&)(Args...) noexcept>;

    /// @brief Deduction guide for `hsd::function` wrappers.
    /// @param args - Fallback values stored for `default_v` positions.
    template <typename Res, typename... Args> 
    default_call_t(function<Res(Args...)>&&, Args&&... args)
        -> default_call_t<Res(Args...), function<Res(Args...)>>;

    /// @brief Deduction guide for arbitrary callables, resolved through their `operator()`.
    /// @param args - Fallback values stored for `default_v` positions.
    template < typename Func, typename... Args, 
        typename Op = decltype(&remove_reference_t<Func>::operator()) > 
    default_call_t(Func&&, Args&&... args)
        -> default_call_t<typename functional_helper::as_function<Op>::type, Func>;
}
