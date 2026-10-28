#pragma once

/// @file Future.hpp
/// @brief Promises, futures, and packaged asynchronous tasks.
/// @details Shares allocator-aware promise state between producers and
/// consumers, propagates result/error values, and supports threaded execution.

#include "../Wrappers/Functional.hpp"
#include "ConditionVariable.hpp"
#include "../Wrappers/SharedPtr.hpp"

namespace hsd
{
    template <typename>
    class packaged_task;

    template <typename T, typename Err = runtime_error>
    class promise;

    namespace future_detail
    {
        /// @brief Shared result state protected by a mutex and completion condition.
        /// @tparam T Stored result representation, such as result or option.
        template <typename T>
        struct promise_data_impl
        {
            /// @brief Result or error state published by the producer.
            T _result;
            /// @brief Protects `_result` and `_is_set` during publication and waits.
            mutex _mutex{};
            /// @brief Indicates that the producer has published the final state.
            bool _is_set = false;
            /// @brief Wakes consumers after `_result` has been published.
            condition_variable _cond_var{};

            /// @brief Constructs an unset state with a default result object.
            inline promise_data_impl() requires(Constructible<T>) = default;

            /// @brief Constructs a state containing a moved result object.
            /// @param result Initial result or error state.
            inline promise_data_impl(T&& result)
                : _result{hsd::move(result)}
            {}

            inline promise_data_impl(const promise_data_impl&) = delete;

            /// @brief Moves result state and completion synchronization state.
            /// @param other State whose contents are transferred.
            inline promise_data_impl(promise_data_impl&& other)
            requires(MoveConstructible<T>)
                : _result{hsd::move(other._result)},
                _mutex{hsd::move(other._mutex)}, 
                _is_set{other._is_set},
                _cond_var{hsd::move(other._cond_var)}
            {
                other._is_set = false;
            }

            inline promise_data_impl& operator=(const promise_data_impl&) = delete;

            /// @brief Moves all state fields from `rhs`.
            /// @param rhs State whose contents are exchanged into this object.
            /// @return `*this` after the move.
            inline promise_data_impl& operator=(promise_data_impl&& rhs)
            requires(MoveConstructible<T>)
            {
                swap(_result, rhs._result);
                swap(_mutex, rhs._mutex);
                swap(_is_set, rhs._is_set);
                swap(_cond_var, rhs._cond_var);
                return *this;
            }
        };

        template <typename, typename>
        struct promise_data;

        /// @brief Shared state representation for value-producing promises.
        template <typename T, typename Err>
        requires (!IsVoid<T> && !IsVoid<Err>)
        struct promise_data<T, Err> :
            public promise_data_impl<result<T, Err>>
        {
            /// @brief Result wrapper stored for value/error promises.
            using res_type = result<T, Err>;
        };

        /// @brief Shared state representation for value-or-empty promises.
        template <typename T>
        struct promise_data<T, void> :
            public promise_data_impl<option<T>>
        {
            /// @brief Result wrapper stored for value-or-empty promises.
            using res_type = option<T>;
        };

        /// @brief Shared state representation for error-only promises.
        template <typename Err>
        struct promise_data<void, Err> :
            public promise_data_impl<option_err<Err>>
        {
            /// @brief Result wrapper stored for completion/error promises.
            using res_type = option_err<Err>;
        };

        /// @brief Reference-counted pointer to shared promise state.
        template <typename T, typename Err>
        using promise_data_ptr = safe_shared_ptr<promise_data<T, Err>>;

        /// @brief Result representation selected for a promise's value/error shape.
        template <typename T, typename Err>
        using promise_data_res_type = typename promise_data<T, Err>::res_type;
    } // namespace future_detail

    /// @brief Consumer handle for asynchronously produced result state.
    template <typename T, typename Err = runtime_error>
    class future
    {
    private:
        /// @brief Reference-counted link to the producer's shared state.
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<T, Err>;
        /// @brief Shared state observed by this consumer handle.
        promise_data_ptr_t _promise_ptr{};
        
        /// @brief Tracks whether this handle was created by a promise.
        bool _is_valid = false;
        friend class promise<T, Err>;

        /// @brief Constructs a consumer handle for `promise_ptr`.
        /// @param promise_ptr Shared state owned by the corresponding promise.
        inline future(promise_data_ptr_t& promise_ptr)
            : _promise_ptr{promise_ptr}, _is_valid{true}
        {}

    public:
        /// @brief Constructs an invalid future with no shared state.
        inline future() = default;
        inline future(const future&) = delete;
        inline future& operator=(const future&) = delete;

        /// @brief Transfers the shared state and validity flag from `other`.
        /// @param other Future whose state is moved.
        inline future(future&& other)
            : _promise_ptr{hsd::move(other._promise_ptr)}, 
            _is_valid{other._is_valid}
        {}

        /// @brief Replaces this handle by exchanging state with `rhs`.
        /// @param rhs Future whose state is exchanged.
        /// @return `*this` after the exchange.
        inline future& operator=(future&& rhs)
        {
            swap(_promise_ptr, rhs._promise_ptr);
            return *this;
        }

        /// @brief Returns the result object stored by the producer.
        /// @return A reference to the shared result/error representation.
        inline auto& get() const
        {
            return _promise_ptr->_result;
        }

        /// @brief Reports whether this handle has published result state.
        /// @return True only for a promise-created handle whose state is set.
        inline bool is_valid() const
        {
            return _is_valid && _promise_ptr->_is_set;
        }

        /// @brief Blocks until the producer publishes a result or error.
        inline void wait()
        {
            unique_lock<mutex> _lock{_promise_ptr->_mutex};
            _promise_ptr->_cond_var.wait(
                _lock, [this] {
                    return _promise_ptr->_is_set;
                }
            );
        }

        /// @brief Waits until completion or until a relative duration expires.
        /// @param rel_time Relative duration to wait.
        inline void wait_for(const precise_clock& rel_time)
        {
            unique_lock<mutex> _lock{_promise_ptr->_mutex};
            _promise_ptr->_cond_var.wait_for(
                _lock, rel_time, [this] {
                    return _promise_ptr->_is_set;
                }
            );
        }

        /// @brief Waits until completion or an absolute deadline is reached.
        /// @param abs_time Absolute deadline for the wait.
        inline void wait_until(const precise_clock& abs_time)
        {
            unique_lock<mutex> _lock{_promise_ptr->_mutex};
            _promise_ptr->_cond_var.wait_until(
                _lock, abs_time, [this] {
                    return _promise_ptr->_is_set;
                }
            );
        }
    };

    /// @brief Producer handle that fulfills a shared future state.
    template <typename T, typename Err>
    class promise
    {
    private:
        /// @brief Shared-state type for this value/error combination.
        using promise_data_t = future_detail::promise_data<T, Err>;
        /// @brief Result wrapper published to the shared state.
        using res_type = future_detail::promise_data_res_type<T, Err>;
        /// @brief Reference-counted shared-state handle.
        using promise_data_ptr_t = future_detail::promise_data_ptr<T, Err>;
        
        /// @brief State fulfilled by this producer and observed by its future.
        promise_data_ptr_t _promise_ptr{};
        
        /// @brief Publishes a complete result/error state and wakes all waiters.
        /// @param result State moved into the shared result slot.
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        /// @brief Constructs a promise with a default uninitialized error.
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{
                    Err{"Promise is in an uninitialized state"}
                }
            )}
        {}

        /// @brief Constructs a promise with an initial error state.
        /// @param init_message Error copied into the initial result.
        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{init_message}
            )}
        {}

        /// @brief Constructs a promise with an initial moved error state.
        /// @param init_message Error moved into the initial result.
        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{hsd::move(init_message)}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        /// @brief Transfers producer ownership of shared state from `other`.
        /// @param other Promise whose state is moved.
        inline promise(promise&& other)
            : _promise_ptr{hsd::move(other._promise_ptr)}
        {}

        /// @brief Exchanges shared state with `rhs`.
        /// @param rhs Promise whose state is exchanged.
        /// @return `*this` after the exchange.
        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two producer handles without changing shared state.
        /// @param lhs First producer handle.
        /// @param rhs Second producer handle.
        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        /// @brief Publishes a copied successful value and wakes future waiters.
        /// @param value Value copied into the shared result.
        inline void set_value(const T& value)
        {
            _set(res_type{value});
        }

        /// @brief Publishes a moved successful value and wakes future waiters.
        /// @param value Value moved into the shared result.
        inline void set_value(T&& value)
        {
            _set(res_type{hsd::move(value)});
        }

        /// @brief Publishes a copied error and wakes future waiters.
        /// @param err Error copied into the shared result.
        inline void set_error(const Err& err)
        {
            _set(res_type{err});
        }

        /// @brief Publishes a moved error and wakes future waiters.
        /// @param err Error moved into the shared result.
        inline void set_error(Err&& err)
        {
            _set(res_type{hsd::move(err)});
        }

        /// @brief Creates the consumer handle for this shared state.
        /// @return A future connected to this promise.
        inline future<T, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    /// @brief Promise specialization whose failure is represented by an empty option.
    template <typename T>
    class promise<T, void>
    {
    private:
        /// @brief Shared-state type for a value-or-empty promise.
        using promise_data_t = future_detail::promise_data<T, void>;
        /// @brief Optional result wrapper published to the shared state.
        using res_type = future_detail::promise_data_res_type<T, void>;
        /// @brief Reference-counted shared-state handle.
        using promise_data_ptr_t = future_detail::promise_data_ptr<T, void>;
        
        /// @brief State fulfilled by this producer.
        promise_data_ptr_t _promise_ptr{};
        
        /// @brief Publishes a value/empty state and wakes all waiters.
        /// @param result State moved into the shared result slot.
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        /// @brief Constructs an unset value-or-empty promise.
        inline promise()
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{err_value{}}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        /// @brief Transfers producer ownership of shared state from `other`.
        /// @param other Promise whose state is moved.
        inline promise(promise&& other)
            : _promise_ptr{hsd::move(other._promise_ptr)}
        {}

        /// @brief Exchanges shared state with `rhs`.
        /// @param rhs Promise whose state is exchanged.
        /// @return `*this` after the exchange.
        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two producer handles.
        /// @param lhs First producer handle.
        /// @param rhs Second producer handle.
        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        /// @brief Publishes a copied successful value.
        /// @param value Value copied into the shared result.
        inline void set_value(const T& value)
        {
            _set(res_type{value});
        }

        /// @brief Publishes a moved successful value.
        /// @param value Value moved into the shared result.
        inline void set_value(T&& value)
        {
            _set(res_type{hsd::move(value)});
        }

        /// @brief Publishes the empty/error state.
        inline void set_error()
        {
            _set(res_type{err_value{}});
        }

        /// @brief Creates the consumer handle for this shared state.
        /// @return A future connected to this promise.
        inline future<T, void> get_future()
        {
            return {_promise_ptr};
        }
    };

    /// @brief Promise specialization representing completion or a typed error.
    template <typename Err>
    class promise<void, Err>
    {
    private:
        /// @brief Shared-state type for an error-only promise.
        using promise_data_t = future_detail::promise_data<void, Err>;
        /// @brief Success/error wrapper published to the shared state.
        using res_type = future_detail::promise_data_res_type<void, Err>;
        /// @brief Reference-counted shared-state handle.
        using promise_data_ptr_t = future_detail::promise_data_ptr<void, Err>;
        
        /// @brief State fulfilled by this producer.
        promise_data_ptr_t _promise_ptr{};
        
        /// @brief Publishes a success/error state and wakes all waiters.
        /// @param result State moved into the shared result slot.
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        /// @brief Constructs a promise with a default uninitialized error.
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{
                    Err{"Promise is in an uninitialized state"}
                }
            )}
        {}

        /// @brief Constructs a promise with an initial copied error.
        /// @param init_message Error copied into the initial result.
        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{init_message}
            )}
        {}

        /// @brief Constructs a promise with an initial moved error.
        /// @param init_message Error moved into the initial result.
        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{hsd::move(init_message)}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        /// @brief Transfers producer ownership of shared state from `other`.
        /// @param other Promise whose state is moved.
        inline promise(promise&& other)
            : _promise_ptr{hsd::move(other._promise_ptr)}
        {}

        /// @brief Exchanges shared state with `rhs`.
        /// @param rhs Promise whose state is exchanged.
        /// @return `*this` after the exchange.
        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two producer handles.
        /// @param lhs First producer handle.
        /// @param rhs Second producer handle.
        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        /// @brief Publishes successful completion and wakes future waiters.
        inline void set_value()
        {
            _set(res_type{ok_value{}});
        }

        /// @brief Publishes a copied error and wakes future waiters.
        /// @param err Error copied into the shared result.
        inline void set_error(const Err& err)
        {
            _set(res_type{err});
        }

        /// @brief Publishes a moved error and wakes future waiters.
        /// @param err Error moved into the shared result.
        inline void set_error(Err&& err)
        {
            _set(res_type{hsd::move(err)});
        }

        /// @brief Creates the consumer handle for this shared state.
        /// @return A future connected to this promise.
        inline future<void, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    /// @brief Callable task that publishes a value or runtime error to a future.
    /// @tparam Res Callable result type, including `void`.
    /// @tparam Args Callable argument types.
    template <typename Res, typename... Args>
    class packaged_task<Res(Args...)>
    {
    private:
        /// @brief Type-erased callable executed by operator().
        function<Res(Args...)> _func;
        /// @brief Promise fulfilled by successful or failed invocation.
        promise<Res, runtime_error> _promise;

    public:
        /// @brief Takes ownership of a type-erased callable.
        /// @param func Callable invoked when the task runs.
        inline packaged_task(function<Res(Args...)>&& func)
            : _func{hsd::move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        /// @brief Transfers callable and promise state from `other`.
        /// @param other Task whose state is moved.
        inline packaged_task(packaged_task&& other)
            : _func{hsd::move(other._func)}, _promise{hsd::move(other._promise)}
        {}

        /// @brief Exchanges callable and promise state with `rhs`.
        /// @param rhs Task whose state is exchanged.
        /// @return `*this` after the exchange.
        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two task handles.
        /// @param lhs First task.
        /// @param rhs Second task.
        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        /// @brief Returns the future connected to this task's promise.
        /// @return Consumer handle for the eventual task result.
        inline auto get_future()
        {
            return _promise.get_future();
        }

        /// @brief Executes the callable and publishes its unwrapped result.
        /// @param args Arguments forwarded to the callable.
        inline void operator()(Args... args)
        {
            if constexpr (IsSame<Res, void>)
            {
                _func(hsd::forward<Args>(args)...).unwrap();
                _promise.set_value();
            }
            else
            {
                _promise.set_value(_func(hsd::forward<Args>(args)...).unwrap());
            }
        }

        /// @brief Reports whether both callable and future state are usable.
        /// @return True when the callable is non-empty and its future is valid.
        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    /// @brief Callable task that propagates a `result<Ok, Err>` outcome.
    /// @tparam Ok Successful result type.
    /// @tparam Err Error result type.
    /// @tparam Args Callable argument types.
    template <typename Ok, typename Err, typename... Args>
    class packaged_task<result<Ok, Err>(Args...)>
    {
    private:
        /// @brief Type-erased result-producing callable.
        function<result<Ok, Err>(Args...)> _func;
        /// @brief Promise receiving either the callable value or error.
        promise<Ok, Err> _promise;

    public:
        /// @brief Takes ownership of a result-producing callable.
        /// @param func Callable invoked when the task runs.
        inline packaged_task(function<result<Ok, Err>(Args...)>&& func)
            : _func{hsd::move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        /// @brief Transfers callable and promise state from `other`.
        /// @param other Task whose state is moved.
        inline packaged_task(packaged_task&& other)
            : _func{hsd::move(other._func)}, _promise{hsd::move(other._promise)}
        {}

        /// @brief Exchanges callable and promise state with `rhs`.
        /// @param rhs Task whose state is exchanged.
        /// @return `*this` after the exchange.
        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two result task handles.
        /// @param lhs First task.
        /// @param rhs Second task.
        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        /// @brief Returns the future connected to this task's promise.
        /// @return Consumer handle for the eventual result.
        inline auto get_future()
        {
            return _promise.get_future();
        }

        /// @brief Executes the callable and publishes its value or error.
        /// @param args Arguments forwarded to the callable.
        inline void operator()(Args... args)
        {
            auto _res = _func(hsd::forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value(_res.unwrap());
            }
            else
            {
                _promise.set_error(_res.unwrap_err());
            }
        }

        /// @brief Reports whether the callable and future state are usable.
        /// @return True when both task components are valid.
        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };
    
    /// @brief Callable task that publishes an optional successful value.
    /// @tparam Ok Successful value type.
    /// @tparam Args Callable argument types.
    template <typename Ok, typename... Args>
    class packaged_task<option<Ok>(Args...)>
    {
    private:
        /// @brief Type-erased optional-result callable.
        function<option<Ok>(Args...)> _func;
        /// @brief Promise receiving the value or empty state.
        promise<Ok, void> _promise;

    public:
        /// @brief Takes ownership of an optional-result callable.
        /// @param func Callable invoked when the task runs.
        inline packaged_task(function<option<Ok>(Args...)>&& func)
            : _func{hsd::move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        /// @brief Transfers callable and promise state from `other`.
        /// @param other Task whose state is moved.
        inline packaged_task(packaged_task&& other)
            : _func{hsd::move(other._func)}, _promise{hsd::move(other._promise)}
        {}

        /// @brief Exchanges callable and promise state with `rhs`.
        /// @param rhs Task whose state is exchanged.
        /// @return `*this` after the exchange.
        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two optional-result task handles.
        /// @param lhs First task.
        /// @param rhs Second task.
        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        /// @brief Returns the future connected to this task's promise.
        /// @return Consumer handle for the eventual optional result.
        inline auto get_future()
        {
            return _promise.get_future();
        }

        /// @brief Executes the callable and publishes its value or empty state.
        /// @param args Arguments forwarded to the callable.
        inline void operator()(Args... args)
        {
            auto _res = _func(hsd::forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value(_res.unwrap());
            }
            else
            {
                _promise.set_error();
            }
        }

        /// @brief Reports whether the callable and future state are usable.
        /// @return True when both task components are valid.
        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    /// @brief Callable task that publishes success or a typed error.
    /// @tparam Err Error type produced by the callable.
    /// @tparam Args Callable argument types.
    template <typename Err, typename... Args>
    class packaged_task<option_err<Err>(Args...)>
    {
    private:
        /// @brief Type-erased success/error callable.
        function<option_err<Err>(Args...)> _func;
        /// @brief Promise receiving completion or the callable error.
        promise<void, Err> _promise;

    public:
        /// @brief Takes ownership of a success/error callable.
        /// @param func Callable invoked when the task runs.
        inline packaged_task(function<option_err<Err>(Args...)>&& func)
            : _func{hsd::move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        /// @brief Transfers callable and promise state from `other`.
        /// @param other Task whose state is moved.
        inline packaged_task(packaged_task&& other)
            : _func{hsd::move(other._func)}, _promise{hsd::move(other._promise)}
        {}

        /// @brief Exchanges callable and promise state with `rhs`.
        /// @param rhs Task whose state is exchanged.
        /// @return `*this` after the exchange.
        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        /// @brief Exchanges two success/error task handles.
        /// @param lhs First task.
        /// @param rhs Second task.
        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        /// @brief Returns the future connected to this task's promise.
        /// @return Consumer handle for eventual completion or error.
        inline auto get_future()
        {
            return _promise.get_future();
        }

        /// @brief Executes the callable and publishes completion or its error.
        /// @param args Arguments forwarded to the callable.
        inline void operator()(Args... args)
        {
            auto _res = _func(hsd::forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value();
            }
            else
            {
                _promise.set_error(_res.unwrap_err());
            }
        }

        /// @brief Reports whether the callable and future state are usable.
        /// @return True when both task components are valid.
        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    /// @brief Deduction guide for a plain function pointer task.
    /// @param function_ptr Function pointer whose signature selects the task type.
    template < typename Res, typename... Args > 
    packaged_task(Res(*function_ptr)(Args...)) -> packaged_task<Res(Args...)>;
    
    /// @brief Deduction guide for a callable object with a non-overloaded call operator.
    /// @param callable Callable object used to infer its function signature.
    template < typename Func, typename Op = decltype(&Func::operator()) > 
    packaged_task(Func callable) -> packaged_task<
        typename functional_helper::as_function<Op>::type
    >;
} // namespace hsd