#pragma once

#include "Functional.hpp"
#include "ConditionVariable.hpp"

namespace hsd
{
    template <typename>
    class packaged_task;

    template <typename T, typename Err = runtime_error>
    class promise;

    namespace future_detail
    {
        template <typename T, typename Err>
        struct promise_data
        {
            Result<T, Err> _result;
            mutex _mutex{};
            bool _is_set = false;
            condition_variable _cond_var{};

            inline promise_data() 
            requires(Constructible<Result<T, Err>>)
            {}

            inline promise_data(Result<T, Err>&& result)
                : _result{move(result)}
            {}

            inline promise_data(const promise_data&) = delete;

            inline promise_data(promise_data&& other)
            requires(MoveConstructible<Result<T, Err>>)
                : _result{move(other._result)},
                _mutex{move(other._mutex)}, 
                _is_set{other._is_set},
                _cond_var{move(other._cond_var)}
            {
                other._is_set = false;
            }

            inline promise_data& operator=(const promise_data&) = delete;

            inline promise_data& operator=(promise_data&& rhs)
            requires(MoveConstructible<Result<T, Err>>)
            {
                swap(_result, rhs._result);
                swap(_mutex, rhs._mutex);
                swap(_is_set, rhs._is_set);
                swap(_cond_var, rhs._cond_var);
                return *this;
            }
        };

        template <typename T, typename Err>
        using promise_data_ptr = safe_shared_ptr<promise_data<T, Err>>;
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
    class future<T&, Err>
    {
    private:
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<reference<T>, Err>;
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
            : _promise_ptr{move(other._lock)}, 
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
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<T, Err>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(Result<T, Err>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<T, Err>{
                    Err{"Promise is in an uninitialized state"}, err_value{}
                }
            )}
        {}

        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<T, Err>{init_message, err_value{}}
            )}
        {}

        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<T, Err>{move(init_message), err_value{}}
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
            _set({value, ok_value{}});
        }

        inline void set_value(T&& value)
        {
            _set({move(value), ok_value{}});
        }

        inline void set_error(const Err& err)
        {
            _set({err, err_value{}});
        }

        inline future<T, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename T, typename Err>
    class promise<T&, Err>
    {
    private:
        using promise_data_t = 
            future_detail::promise_data<reference<T>, Err>;
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<reference<T>, Err>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(Result<reference<T>, Err>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<reference<T>, Err>{
                    Err{"Promise is in an uninitialized state"}, err_value{}
                }
            )}
        {}

        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<reference<T>, Err>{init_message, err_value{}}
            )}
        {}

        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<reference<T>, Err>{move(init_message), err_value{}}
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

        inline void set_value(T& value)
        {
            _set({reference<T>{value}, ok_value{}});
        }

        inline void set_error(const Err& err)
        {
            _set({err, err_value{}});
        }

        inline future<T&, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename Err>
    class promise<void, Err>
    {
    private:
        using promise_data_t = future_detail::promise_data<void, Err>;
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<void, Err>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(Result<void, Err>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise() requires (Constructible<Err, const char*>)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<void, Err>{
                    Err{"Promise is in an uninitialized state"}
                }
            )}
        {}

        inline promise(const Err& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<void, Err>{init_message}
            )}
        {}

        inline promise(Err&& init_message)
            : _promise_ptr{make_safe_shared<promise_data_t>(
                Result<void, Err>{move(init_message)}
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
            _set({});
        }

        inline void set_error(const Err& err)
        {
            _set({err});
        }

        inline future<void, Err> get_future()
        {
            return {_promise_ptr};
        }
    };

    // <Err = void> overloads for promise

    template <typename T>
    class promise<T, void>
    {
    private:
        using promise_data_t = 
            future_detail::promise_data<T, void>;
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<T, void>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(optional<T>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise()
            : _promise_ptr{make_safe_shared<promise_data_t>(
                optional<T>{}
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
            _set({value, ok_value{}});
        }

        inline void set_value(T&& value)
        {
            _set({move(value), ok_value{}});
        }

        inline void set_error()
        {
            _set({});
        }

        inline future<T, void> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename T>
    class promise<T&, void>
    {
    private:
        using promise_data_t = 
            future_detail::promise_data<reference<T>, void>;
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<reference<T>, void>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(optional<reference<T>>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise()
            : _promise_ptr{make_safe_shared<promise_data_t>(
                optional<reference<T>>{}
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

        inline void set_value(T& value)
        {
            _set({reference<T>{value}, ok_value{}});
        }

        inline void set_error()
        {
            _set({});
        }

        inline future<T&, void> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <>
    class promise<void, void>
    {
    private:
        using promise_data_t = 
            future_detail::promise_data<void, void>;
        using promise_data_ptr_t = 
            future_detail::promise_data_ptr<void, void>;
        promise_data_ptr_t _promise_ptr{};
        
        inline void _set(optional<void>&& result)
        {
            hsd::unique_lock<hsd::mutex> _lock{_promise_ptr->_mutex};
            swap(_promise_ptr->_result, result);
            _promise_ptr->_is_set = true;
            _promise_ptr->_cond_var.notify_all();
        }

    public:
        inline promise()
            : _promise_ptr{make_safe_shared<promise_data_t>(
                optional<void>{}
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
            _set({});
        }

        inline void set_error()
        {
            _set({err_value{}});
        }

        inline future<void, void> get_future()
        {
            return {_promise_ptr};
        }
    };

    template <typename Res, typename... Args>
    class packaged_task<Res(Args...)>
    {
    private:
        function<Res(Args...)> _func;
        promise<Res, void> _promise;

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

        inline future<Res, void> get_future()
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

    template <typename Res, typename Err, typename... Args>
    class packaged_task<Result<Res, Err>(Args...)>
    {
    private:
        function<Result<Res, Err>(Args...)> _func;
        promise<Res, Err> _promise;

    public:
        inline packaged_task(function<Result<Res, Err>(Args...)>&& func)
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

        inline future<Res, Err> get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            Result<Res, Err> _result = _func(forward<Args>(args)...).unwrap();
            
            if (_result.is_ok())
            {
                if constexpr (IsSame<Res, void>)
                {
                    _result.unwrap();
                    _promise.set_value();
                }
                else
                {
                    _promise.set_value(_result.unwrap());
                }
            }
            else
            {
                if constexpr (IsSame<Err, void>)
                {
                    _result.unwrap_err();
                    _promise.set_error();
                }
                else
                {
                    _promise.set_error(_result.unwrap_err());
                }
            }
        }

        inline bool valid() const
        {
            return static_cast<bool>(_func) && get_future().valid();
        }
    };

    template <typename Res, typename Err, typename... Args>
    class packaged_task<Result<reference<Res>, Err>(Args...)>
    {
    private:
        function<Res(Args...)> _func;
        promise<Res&, Err> _promise;

    public:
        inline packaged_task(
            function<Result<reference<Res>, Err>(Args...)>&& func)
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

        inline future<reference<Res>, Err> get_future()
        {
            return _promise.get_future();
        }

        inline void operator()(Args... args)
        {
            Result<reference<Res>, Err> _result = _func(forward<Args>(args)...).unwrap();
            
            if (_result.is_ok())
            {
                _promise.set_value(_result.unwrap());
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

    template < typename Res, typename... Args > 
    packaged_task(Res(*)(Args...)) -> packaged_task<Res(Args...)>;
    
    template < typename Func, typename Op = decltype(&Func::operator()) > 
    packaged_task(Func) -> packaged_task<
        typename functional_helper::as_function<Op>::type
    >;
} // namespace hsd