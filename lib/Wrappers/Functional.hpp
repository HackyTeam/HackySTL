#pragma once

/// @file Functional.hpp
/// @brief Type-erased callable wrapper with small-object storage.
/// @details Stores function pointers, lambdas, member adapters, and project
/// result-returning callables while preserving callable lifetime and errors.

#include "Tuple.hpp"
#include "../Base/Result.hpp"
#include "SharedPtr.hpp"

namespace hsd
{
    template <typename> class function;
    
    namespace func_detail
    {
        template < typename Func, typename Res, typename... Args >
        concept IsFunction = (
            !is_same<remove_cvref_t<Func>, function<Res(Args...)>>::value && 
            InvocableRet<Res, Func, Args...>
        );

        /// @brief Error returned when an empty function is invoked.
        /// @brief Error returned when a function wrapper has no callable target.
        struct bad_function
        {
            /// @brief Returns the diagnostic used for empty invocation.
            /// @return A stable diagnostic string.
            const char* pretty_error() const
            {
                return "Tried to invoke a bad function";
            }
        };

        template <typename T>
        /// @brief Maps reference results to the project's non-owning reference type.
        struct store_ref
        {
            /// @brief Stored result type for non-reference results.
            using type = T;
        };
        
        template <typename T>
        /// @brief Reference-result specialization used by function invocation.
        struct store_ref<T&>
        {
            /// @brief Non-owning wrapper used to return a reference result safely.
            using type = reference<T>;
        };

        /// @brief Result type used by function invocation for `T`.
        template <typename T>
        using store_ref_t = typename store_ref<T>::type;
    } // namespace func_detail

    /// @brief Type-erased callable with result-aware invocation.
    /// @tparam ResultType Callable result type.
    /// @tparam Args Invocation argument types.
    template < typename ResultType, typename... Args >
    class function<ResultType(Args...)>
    {
    private:
        /// @brief Maximum erased callable size stored directly in the wrapper.
        static constexpr usize small_func_size = 32; 

        /// @brief Private erased-callable interface used by function storage.
        struct callable_base
        {
            //virtual unique_ptr<callable_base> clone() const = 0;
            /// @brief Copies the target into a supplied inline buffer when it fits.
            /// @return True when the target was copied into the buffer.
            virtual bool copy_buf(char*) const = 0;
            /// @brief Moves the target into a supplied inline buffer when it fits.
            /// @return True when the target was moved into the buffer.
            virtual bool move_buf(char*) const = 0;
            /// @brief Reports whether the erased target is initialized.
            /// @return True when this erased target can be invoked.
            virtual bool is_initilized() const = 0;
            /// @brief Invokes the erased target and unwraps project results when needed.
            /// @return The result produced by the erased target.
            virtual ResultType operator()(Args&&...) = 0;
            /// @brief Destroys the erased target through the interface.
            virtual ~callable_base() = default;
        };

        template <typename Func>
        /// @brief Concrete erased callable holder for one decayed target type.
        struct callable : callable_base
        {
            /// @brief Concrete callable target stored by the erasure layer.
            Func _func;

            /// @brief Constructs the erased target from a compatible callable.
            /// @param func Callable object forwarded into storage.
            template <typename F>
            requires (Constructible<Func, F>)
            inline callable(F&& func)
                : _func{hsd::forward<F>(func)}
            {}

            /// @brief Reports whether pointer targets are non-null.
            /// @return True for non-pointer targets or non-null pointer targets.
            virtual bool is_initilized() const override
            {
                if constexpr (is_pointer<Func>::value)
                {
                    return _func != nullptr;
                }
                else
                {
                    return true;
                }
            }

            /// @brief Invokes the target, unwrapping result-like returns.
            /// @param args Arguments forwarded to the target.
            /// @return Target result after any required unwrap.
            virtual ResultType operator()(Args&&... args) override
            {
                if constexpr (requires {{_func(hsd::forward<Args>(args)...)} -> IsSame<ResultType>;})
                {
                    return _func(hsd::forward<Args>(args)...);
                }
                else if constexpr (UnwrapInvocable<Func, Args...>)
                {
                    return _func(hsd::forward<Args>(args)...).unwrap();
                }
            }

            /// @brief Copies this target into `buf` when small and copyable.
            /// @param buf Inline storage destination.
            /// @return True when the target was copied into the buffer.
            virtual bool copy_buf(char* buf) const override
            {
                if constexpr (CopyConstructible<Func> && sizeof(callable<Func>) <= small_func_size)
                {
                    hsd::construct_at(reinterpret_cast<callable<Func>*>(buf), _func);
                    return true;
                }
                else
                {
                    return false;
                }
            }

            /// @brief Moves this target into `buf` when it fits inline.
            /// @param buf Inline storage destination.
            /// @return True when the target was moved into the buffer.
            virtual bool move_buf(char* buf) const override
            {
                if constexpr (sizeof(callable<Func>) <= small_func_size)
                {
                    hsd::construct_at(reinterpret_cast<callable<Func>*>(buf), hsd::move(_func));
                    return true;
                }
                else
                {
                    return false;
                }
            }
        };

        /// @brief Initializes the inline storage with a null function pointer target.
        /// @return Reference to the inline callable interface.
        inline callable_base& init_ref()
        {
            using func_type = ResultType(*)(Args...);
            hsd::construct_at(reinterpret_cast<callable<func_type>*>(_small_func), nullptr);
            return *reinterpret_cast<callable_base*>(_small_func);
        }
        
        /// @brief Inline storage for small erased callables.
        char _small_func[small_func_size]{};
        /// @brief Shared storage for callables too large for `_small_func`.
        safe_shared_ptr<callable_base> _func_impl{};
        /// @brief Non-owning reference to whichever callable storage is active.
        reference<callable_base> _func_ref = init_ref();

        /// @brief Destroys the active inline target or releases shared storage.
        inline void reset()
        {
            if (_func_ref.get_ptr() == reinterpret_cast<callable_base*>(_small_func))
            {
                reinterpret_cast<callable_base*>(_small_func)->~callable_base();
            }
            else
            {
                _func_impl = nullptr;
            }

            // Keep `_func_ref` on a live empty target, so a later reset or the
            // destructor never touches an inline target that is already destroyed.
            _func_ref = init_ref();
        }

        /// @brief Copies another wrapper using inline or shared storage as needed.
        /// @param func Wrapper whose target is copied.
        inline void _construct_func(const function& func)
        {
            if (func._func_ref.get().copy_buf(_small_func) == true)
            {
                _func_ref = *reinterpret_cast<callable_base*>(_small_func);
            }
            else
            {
                _func_impl = nullptr;
                _func_impl = func._func_impl;
                _func_ref = *_func_impl;
            }
        }

        /// @brief Moves another wrapper using inline or shared storage as needed.
        /// @param func Wrapper whose target is moved.
        inline void _move_func(function&& func)
        {
            if (func._func_ref.get().move_buf(_small_func) == true)
            {
                _func_ref = *reinterpret_cast<callable_base*>(_small_func);
            }
            else
            {
                hsd::swap(_func_impl, func._func_impl);
                _func_ref = *_func_impl;
                func._func_ref = func.init_ref();
            }
        }

        /// @brief Stores an arbitrary target using the small-object optimization.
        /// @param func Callable target to copy or move into this wrapper.
        inline void _copy_func(auto&& func)
        {
            using func_type = decay_t<decltype(func)>;

            if constexpr (sizeof(callable<func_type>) <= sizeof(_small_func))
            {
                hsd::construct_at(
                    reinterpret_cast<callable<func_type>*>(_small_func), 
                    hsd::forward<decltype(func)>(func)
                );

                _func_ref = *reinterpret_cast<callable_base*>(_small_func);
            }
            else
            {
                _func_impl = make_safe_shared<callable<func_type>>(hsd::forward<decltype(func)>(func));
                _func_ref = *_func_impl;
            }
        }

    public:
        /// @brief Constructs an empty callable wrapper.
        inline function() = default;
        /// @brief Constructs an empty wrapper from the null tag.
        inline function(null_type) {}

        /// @brief Stores a callable target, using inline storage when possible.
        /// @param func Callable target forwarded into this wrapper.
        inline function(auto&& func)
        requires (func_detail::IsFunction<decltype(func), ResultType, Args...>);

        /// @brief Copies a callable target using inline or shared storage.
        /// @param other Wrapper whose target is copied.
        inline function(const function& other);

        /// @brief Transfers callable ownership from another wrapper.
        /// @param other Wrapper whose target is moved.
        inline function(function&& other)
        {
            _move_func(hsd::forward<function&&>(other));
        }

        inline ~function()
        {
            reset();
        }

        /// @brief Replaces the target by copying another wrapper.
        /// @param other Wrapper whose target is copied.
        /// @return `*this` after replacement.
        inline function& operator=(const function& other);

        /// @brief Replaces the target with a callable object.
        /// @param func Callable target forwarded into this wrapper.
        /// @return `*this` after replacement.
        inline function& operator=(auto&& func)
        requires (func_detail::IsFunction<decltype(func), ResultType, Args...>);

        /// @brief Replaces the target by transferring callable ownership.
        /// @param other Wrapper whose target is moved.
        /// @return `*this` after replacement.
        inline function& operator=(function&& other)
        {
            if (this != &other)
            {
                reset();
                _move_func(hsd::forward<function&&>(other));
            }

            return *this;
        }

        /// @brief Clears the target and returns an empty wrapper.
        /// @return `*this` after clearing.
        inline function& operator=(null_type)
        {
            reset();
            return *this;
        }

        /// @brief Reports whether a callable target is available.
        /// @return True when inline or shared callable storage is initialized.
        inline explicit operator bool() const
        {
            return _func_ref.get().is_initilized() || _func_impl.get() != nullptr;
        }

        /// @brief Invokes a non-void target and returns its result or bad_function.
        /// @param args Arguments forwarded to the callable target.
        /// @return The callable result, or `bad_function` when empty.
        inline auto operator()(Args... args) 
            -> result<
                func_detail::store_ref_t<ResultType>, 
                func_detail::bad_function >
        requires (!is_void<ResultType>::value)
        {
            if (static_cast<bool>(*this) == false)
            {
                return func_detail::bad_function{};
            }

            return {_func_ref.get().operator()(hsd::forward<Args>(args)...)};
        }

        /// @brief Invokes a void target and returns success or bad_function.
        /// @param args Arguments forwarded to the callable target.
        /// @return Empty success, or `bad_function` when empty.
        inline auto operator()(Args... args) 
            -> option_err<func_detail::bad_function>
        requires (is_void<ResultType>::value)
        {
            if (static_cast<bool>(*this) == false)
            {
                return func_detail::bad_function{};
            }

            _func_ref.get().operator()(hsd::forward<Args>(args)...);
            return {};
        }

        /// @brief Const invocation of a non-void target.
        /// @param args Arguments forwarded to the callable target.
        /// @return The callable result, or `bad_function` when empty.
        inline auto operator()(Args... args) const
            -> result<
                func_detail::store_ref_t<ResultType>, 
                func_detail::bad_function >
        requires (!is_void<ResultType>::value)
        {
            if (static_cast<bool>(*this) == false)
            {
                return func_detail::bad_function{};
            }

            return {_func_ref.get().operator()(hsd::forward<Args>(args)...)};
        }

        /// @brief Const invocation of a void target.
        /// @param args Arguments forwarded to the callable target.
        /// @return Empty success, or `bad_function` when empty.
        inline auto operator()(Args... args) const
            -> option_err<func_detail::bad_function>
        requires (is_void<ResultType>::value)
        {
            if (static_cast<bool>(*this) == false)
            {
                return func_detail::bad_function{};
            }

            _func_ref.get().operator()(hsd::forward<Args>(args)...);
            return {};
        }
    };

    namespace functional_helper
    {
        /// @brief Maps member-function pointer types to function signatures.
        template <typename>
        struct as_function {};

        /// @brief Signature mapping for non-const member functions.
        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) noexcept(Case)>
        {
            /// @brief Function signature produced from the member pointer.
            using type = Res(Args...);
        };

        /// @brief Signature mapping for lvalue-qualified member functions.
        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...)& noexcept(Case)>
        { 
            /// @brief Function signature produced from the member pointer.
            using type = Res(Args...);
        };

        /// @brief Signature mapping for const member functions.
        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const noexcept(Case)>
        { 
            /// @brief Function signature produced from the member pointer.
            using type = Res(Args...);
        };

        /// @brief Signature mapping for const lvalue-qualified member functions.
        template <typename Res, typename Scope, bool Case, typename... Args>
        struct as_function<Res(Scope::*)(Args...) const& noexcept(Case)>
        { 
            /// @brief Function signature produced from the member pointer.
            using type = Res(Args...);
        };
    } // namespace functional_helper

    template < typename ResultType, typename... Args >
    inline function<ResultType(Args...)>::function(const function& other)
    {
        _construct_func(other);
    }

    template < typename ResultType, typename... Args >
    inline function<ResultType(Args...)>::function(auto&& func)
    requires (func_detail::IsFunction<decltype(func), ResultType, Args...>)
    {
        //_func_impl = make_unique<callable<func_type>>(forward<decltype(func)>(func));
        _copy_func(hsd::forward<decltype(func)>(func));
    }

    template < typename ResultType, typename... Args >
    inline auto function<ResultType(Args...)>::operator=(const function<ResultType(Args...)>& other)
        -> function<ResultType(Args...)>&
    {
        if (this != &other)
        {
            reset();
            _construct_func(other);
        }

        return *this;
    }

    template < typename ResultType, typename... Args >
    inline auto function<ResultType(Args...)>::operator=(auto&& func)
        -> function<ResultType(Args...)>&
    requires (func_detail::IsFunction<decltype(func), ResultType, Args...>)
    {
        reset();
        //_func_impl = make_unique<callable<func_type>>(forward<decltype(func)>(func));
        
        _copy_func(hsd::forward<decltype(func)>(func));
        return *this;
    }

    /// @brief Binds a member function to an owned object or pointer and arguments.
    /// @param func Member function pointer to invoke.
    /// @param value Object or pointer captured by value.
    /// @param args Arguments captured for each invocation.
    /// @return A callable adapter that forwards the bound invocation and unwraps
    ///         project result values when necessary.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T&& value, Args&&... args)
    {
        return [=, value{hsd::move(value)}]() mutable {
            if constexpr (requires {value.*(func(declval<Args>()...)).unwrap();})
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...).unwrap();
                }
                else
                {
                    return (value.*func)(args...).unwrap();
                }
            }
            else
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...);
                }
                else
                {
                    return (value.*func)(args...);
                }
            }
        };
    }

    /// @brief Binds a member function to a moved tuple of arguments.
    /// @param func Member function pointer to invoke.
    /// @param value Object or pointer captured by value.
    /// @param args Tuple whose elements are expanded for each invocation.
    /// @return A callable adapter for the bound member invocation.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T&& value, hsd::tuple<Args...>&& args)
    {
        return [=, value{hsd::move(value)}]() mutable {
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {(value.*func)(declval<Args>()...).unwrap();})
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                }
                else
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        );
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        );
                    }
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    /// @brief Binds a member function to an externally owned object reference.
    /// @param func Member function pointer to invoke.
    /// @param value Object or pointer captured by reference.
    /// @param args Arguments captured for each invocation.
    /// @return A callable adapter that retains the caller's object reference.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T& value, Args&&... args)
    {
        return [=, &value]{
            if constexpr (requires {value.*(func(declval<Args>()...)).unwrap();})
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...).unwrap();
                }
                else
                {
                    return (value.*func)(args...).unwrap();
                }
            }
            else
            {
                if constexpr (is_pointer<T>::value)
                {
                    return (value->*func)(args...);
                }
                else
                {
                    return (value.*func)(args...);
                }
            }
        };
    }

    /// @brief Binds a member function to an object reference and tuple arguments.
    /// @param func Member function pointer to invoke.
    /// @param value Object or pointer captured by reference.
    /// @param args Tuple whose elements are expanded for each invocation.
    /// @return A callable adapter for the bound member invocation.
    template < typename Func, typename T, typename... Args >
    requires (std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, T& value, hsd::tuple<Args...>&& args)
    {
        return [=, &value]{
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {(value.*func)(declval<Args>()...).unwrap();})
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        ).unwrap();
                    }
                }
                else
                {
                    if constexpr (is_pointer<T>::value)
                    {
                        return (value->*func)(
                            args.template get<Ints>()...
                        );
                    }
                    else
                    {
                        return (value.*func)(
                            args.template get<Ints>()...
                        );
                    }
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    /// @brief Binds a free callable to arguments captured by value.
    /// @param func Callable to invoke.
    /// @param args Arguments captured for each invocation.
    /// @return A callable adapter that unwraps project result values when needed.
    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, Args&&... args)
    {
        return [=]{
            if constexpr (requires {func(declval<Args>()...).unwrap();})
            {
                return func(args...).unwrap();
            }
            else
            {
                return func(args...);
            }
        };
    }

    /// @brief Binds a free callable to a moved tuple of arguments.
    /// @param func Callable to invoke.
    /// @param args Tuple whose elements are expanded for each invocation.
    /// @return A callable adapter for the bound call.
    template < typename Func, typename... Args >
    requires (!std::is_member_function_pointer_v<Func>)
    static inline auto bind(Func func, hsd::tuple<Args...>&& args)
    {
        return [=]{
            return [&]<usize... Ints>(hsd::index_sequence<Ints...>)
            {
                if constexpr (requires {func(declval<Args>()...).unwrap();})
                {
                    return func(args.template get<Ints>()...).unwrap();
                }
                else
                {
                    return func(args.template get<Ints>()...);
                }
            }(hsd::index_sequence_for<Args...>{});
        };
    }

    /// @brief Deduction guide for a plain function pointer.
    template < typename Res, typename... Args > 
    function(Res(*)(Args...)) -> function<Res(Args...)>;
    
    /// @brief Deduction guide for a callable object's call operator.
    template < typename Func, typename Op = decltype(&Func::operator()) > 
    function(Func) -> function<
        typename functional_helper::as_function<Op>::type
    >;
} // namespace hsd