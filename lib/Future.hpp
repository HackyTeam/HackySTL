#pragma once

#include "Functional.hpp"
#include "ConditionVariable.hpp"
#include "SharedPtr.hpp"

namespace hsd
{
    template <typename>
    class packaged_task;

    template <typename T, typename Err = runtime_error>
    class promise;

    namespace future_detail
    {
        template <typename T>
        struct promise_data_impl
        {
            T _result;
            mutex _mutex{};
            bool _is_set = false;
            condition_variable _cond_var{};

            inline promise_data_impl() requires(Constructible<T>) = default;

            inline promise_data_impl(T&& result)
                : _result{move(result)}
            {}

            inline promise_data_impl(const promise_data_impl&) = delete;

            inline promise_data_impl(promise_data_impl&& other)
            requires(MoveConstructible<T>)
                : _result{move(other._result)},
                _mutex{move(other._mutex)}, 
                _is_set{other._is_set},
                _cond_var{move(other._cond_var)}
            {
                other._is_set = false;
            }

            inline promise_data_impl& operator=(const promise_data_impl&) = delete;

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

        template <typename T, typename Err>
        requires (!IsVoid<T> && !IsVoid<Err>)
        struct promise_data<T, Err> :
            public promise_data_impl<result<T, Err>>
        {
            using res_type = result<T, Err>;
        };

        template <typename T>
        struct promise_data<T, void> :
            public promise_data_impl<option<T>>
        {
            using res_type = option<T>;
        };

        template <typename Err>
        struct promise_data<void, Err> :
            public promise_data_impl<option_err<Err>>
        {
            using res_type = option_err<Err>;
        };

        template <typename T, typename Err>
        using promise_data_ptr = safe_shared_ptr<promise_data<T, Err>>;

        template <typename T, typename Err>
        using promise_data_res_type = typename promise_data<T, Err>::res_type;
    } // namespace future_detail

    template <typename T, typename Err = runtime_error>
    class future
    {
    private:
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<T, Err>;
        promise_data_ptr_t _promise_ptr{};
        
        bool _is_valid = false;
        friend class promise<T, Err>;

        inline future(promise_data_ptr_t& promise_ptr)
            : _promise_ptr{promise_ptr}, _is_valid{true}
        {}

    public:
        inline future() = default;
        inline future(const future&) = delete;
        inline future& operator=(const future&) = delete;

        inline future(future&& other)
            : _promise_ptr{move(other._promise_ptr)}, 
            _is_valid{other._is_valid}
        {}

        inline future& operator=(future&& rhs)
        {
            swap(_promise_ptr, rhs._promise_ptr);
            return *this;
        }

        inline auto& get() const
        {
            return _promise_ptr->_result;
        }

        inline bool is_valid() const
        {
            return _is_valid && _promise_ptr->_is_set;
        }

        inline void wait()
        {
            unique_lock<mutex> _lock{_promise_ptr->_mutex};
            _promise_ptr->_cond_var.wait(
                _lock, [this] {
                    return _promise_ptr->_is_set;
                }
            );
        }

        inline void wait_for(const precise_clock& rel_time)
        {
            unique_lock<mutex> _lock{_promise_ptr->_mutex};
            _promise_ptr->_cond_var.wait_for(
                _lock, rel_time, [this] {
                    return _promise_ptr->_is_set;
                }
            );
        }

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

    template <typename T, typename Err>
    class promise
    {
    private:
        using promise_data_t = future_detail::promise_data<T, Err>;
        using res_type = future_detail::promise_data_res_type<T, Err>;
        using promise_data_ptr_t = future_detail::promise_data_ptr<T, Err>;
        
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{
                    Err{"Promise is in an uninitialized state"}
                }
            )}
        {}

        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{init_message}
            )}
        {}

        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{move(init_message)}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        inline promise(promise&& other)
            : _promise_ptr{move(other._promise_ptr)}
        {}

        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        inline void set_value(const T& value)
        {
            _set(res_type{value});
        }

        inline void set_value(T&& value)
        {
            _set(res_type{move(value)});
        }

        inline void set_error(const Err& err)
        {
            _set(res_type{err});
        }

        inline void set_error(Err&& err)
        {
            _set(res_type{move(err)});
        }

        inline future<T, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename T>
    class promise<T, void>
    {
    private:
        using promise_data_t = future_detail::promise_data<T, void>;
        using res_type = future_detail::promise_data_res_type<T, void>;
        using promise_data_ptr_t = future_detail::promise_data_ptr<T, void>;
        
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise()
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{err_value{}}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        inline promise(promise&& other)
            : _promise_ptr{move(other._promise_ptr)}
        {}

        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        inline void set_value(const T& value)
        {
            _set(res_type{value});
        }

        inline void set_value(T&& value)
        {
            _set(res_type{move(value)});
        }

        inline void set_error()
        {
            _set(res_type{err_value{}});
        }

        inline future<T, void> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename Err>
    class promise<void, Err>
    {
    private:
        using promise_data_t = future_detail::promise_data<void, Err>;
        using res_type = future_detail::promise_data_res_type<void, Err>;
        using promise_data_ptr_t = future_detail::promise_data_ptr<void, Err>;
        
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(res_type&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{
                    Err{"Promise is in an uninitialized state"}
                }
            )}
        {}

        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{init_message}
            )}
        {}

        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                res_type{move(init_message)}
            )}
        {}

        inline promise(const promise&) = delete;
        inline promise& operator=(const promise&) = delete;

        inline promise(promise&& other)
            : _promise_ptr{move(other._promise_ptr)}
        {}

        inline promise& operator=(promise&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(promise& lhs, promise& rhs)
        {
            swap(lhs._promise_ptr, rhs._promise_ptr);
        }

        inline void set_value()
        {
            _set(res_type{ok_value{}});
        }

        inline void set_error(const Err& err)
        {
            _set(res_type{err});
        }

        inline void set_error(Err&& err)
        {
            _set(res_type{move(err)});
        }

        inline future<void, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename Res, typename... Args>
    class packaged_task<Res(Args...)>
    {
    private:
        function<Res(Args...)> _func;
        promise<Res, runtime_error> _promise;

    public:
        inline packaged_task(function<Res(Args...)>&& func)
            : _func{move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        inline packaged_task(packaged_task&& other)
            : _func{move(other._func)}, _promise{move(other._promise)}
        {}

        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        inline auto get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            if constexpr (IsSame<Res, void>)
            {
                _func(forward<Args>(args)...).unwrap();
                _promise.set_value();
            }
            else
            {
                _promise.set_value(_func(forward<Args>(args)...).unwrap());
            }
        }

        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    template <typename Ok, typename Err, typename... Args>
    class packaged_task<result<Ok, Err>(Args...)>
    {
    private:
        function<result<Ok, Err>(Args...)> _func;
        promise<Ok, Err> _promise;

    public:
        inline packaged_task(function<result<Ok, Err>(Args...)>&& func)
            : _func{move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        inline packaged_task(packaged_task&& other)
            : _func{move(other._func)}, _promise{move(other._promise)}
        {}

        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        inline auto get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            auto _res = _func(forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value(_res.unwrap());
            }
            else
            {
                _promise.set_error(_res.unwrap_err());
            }
        }

        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };
    
    template <typename Ok, typename... Args>
    class packaged_task<option<Ok>(Args...)>
    {
    private:
        function<option<Ok>(Args...)> _func;
        promise<Ok, void> _promise;

    public:
        inline packaged_task(function<option<Ok>(Args...)>&& func)
            : _func{move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        inline packaged_task(packaged_task&& other)
            : _func{move(other._func)}, _promise{move(other._promise)}
        {}

        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        inline auto get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            auto _res = _func(forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value(_res.unwrap());
            }
            else
            {
                _promise.set_error();
            }
        }

        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    template <typename Err, typename... Args>
    class packaged_task<option_err<Err>(Args...)>
    {
    private:
        function<option_err<Err>(Args...)> _func;
        promise<void, Err> _promise;

    public:
        inline packaged_task(function<option_err<Err>(Args...)>&& func)
            : _func{move(func)}, _promise{}
        {}

        inline packaged_task(const packaged_task&) = delete;
        inline packaged_task& operator=(const packaged_task&) = delete;

        inline packaged_task(packaged_task&& other)
            : _func{move(other._func)}, _promise{move(other._promise)}
        {}

        inline packaged_task& operator=(packaged_task&& rhs)
        {
            swap(*this, rhs);
            return *this;
        }

        inline friend void swap(packaged_task& lhs, packaged_task& rhs)
        {
            swap(lhs._func, rhs._func);
            swap(lhs._promise, rhs._promise);
        }

        inline auto get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            auto _res = _func(forward<Args>(args)...).unwrap();
            
            if (_res.is_ok())
            {
                _promise.set_value();
            }
            else
            {
                _promise.set_error(_res.unwrap_err());
            }
        }

        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    template < typename Res, typename... Args > 
    packaged_task(Res(*)(Args...)) -> packaged_task<Res(Args...)>;
    
    template < typename Func, typename Op = decltype(&Func::operator()) > 
    packaged_task(Func) -> packaged_task<
        typename functional_helper::as_function<Op>::type
    >;
} // namespace hsd