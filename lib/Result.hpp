#pragma once

#include "Reference.hpp"
#include "Concepts.hpp"

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

namespace hsd
{
    namespace Result_detail
    {
        template <typename From, typename To>
        concept is_same = hsd::is_same<From, To>::value;

        template <typename T>
        using InvokeType = decltype(declval<T>()());

        template <typename T>
        concept IsReference = (requires {typename T::value_type;} && (
            hsd::is_same<T, reference<typename T::value_type>>::value ||
            hsd::is_same<T, const reference<typename T::value_type>>::value
        ));

        template <typename T>
        concept IsString = requires(T str) 
        {
            {str.c_str()} -> is_same<const char*>;
        };
    } // namespace Result_detail

    [[noreturn]] static inline void panic(
        const char* message,
        const char* func = __builtin_FUNCTION(),
        const char* file_name = __builtin_FILE(),
        usize line = __builtin_LINE())
    {
        fprintf(
            stderr, "Got an error in file:"
            "\n%s\nInvoked from: %s at line:"
            " %zu\nWith the message: %s\n", 
            file_name, func, line, message
        );

        abort();
    }

    [[noreturn]] static inline void unimplemented(
        const char* func = __builtin_FUNCTION(),
        const char* file_name = __builtin_FILE(),
        usize line = __builtin_LINE())
    {
        panic("This funcion is unimplemented", func, file_name, line);
    }

    struct ok_value {};
    struct err_value {};
    
    class runtime_error
    {
    protected:
        const char* _err = nullptr;
        
    public:
        constexpr runtime_error(const char* error)
            : _err{error}
        {}

        constexpr const char* operator()() const
        {
            return _err;
        }
    };

    template < typename Ok, typename Err >
    class [[nodiscard("Result type should not be discarded")]] Result
    {
    private:
        union
        {
            Ok _ok_data;
            Err _err_data;
        };

        bool _initialized = false;
        
    public:
        template <typename T>
        requires (Constructible<Ok, T&&>)
        constexpr Result(T&& value, ok_value = {})
            : _ok_data{forward<T>(value)}, _initialized{true}
        {}

        constexpr Result(const Ok& value, ok_value = {})
        requires (CopyConstructible<Ok>)
            : _ok_data{value}, _initialized{true}
        {}

        constexpr Result(Ok&& value, ok_value = {})
        requires (MoveConstructible<Ok>)
            : _ok_data{move(value)}, _initialized{true}
        {}

        template <typename T>
        requires (Constructible<Err, T&&>)
        constexpr Result(T&& value, err_value = {})
            : _err_data{forward<T>(value)}, _initialized{false}
        {}

        constexpr Result(const Err& value, err_value = {})
        requires (CopyConstructible<Err>)
            : _err_data{value}, _initialized{false}
        {}

        constexpr Result(Err&& value, err_value = {})
        requires (MoveConstructible<Err>)
            : _err_data{move(value)}, _initialized{false}
        {}

        inline Result(const Result& other)
            : _initialized{other._initialized}
        {
            if (_initialized)
            {
                new (&_ok_data) Ok{other._ok_data};
            }
            else
            {
                new (&_err_data) Err{other._err_data};
            }
        }

        inline Result(Result&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                new (&_ok_data) Ok(move(other._ok_data));
            }
            else
            {
                new (&_err_data) Err(move(other._err_data));
            }
        }

        inline Result& operator=(Result&& rhs)
        {
            Result _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                new (&rhs._ok_data) Ok{move(_ok_data)};
            }
            else
            {
                new (&rhs._err_data) Err{move(_err_data)};
            }

            swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                new (&_ok_data) Ok{move(_tmp._ok_data)};
            }
            else
            {
                new (&_err_data) Err{move(_tmp._err_data)};
            }

            return *this;
        }

        constexpr ~Result()
        {
            if (_initialized)
            {
                _ok_data.~Ok();
            }
            else
            {
                _err_data.~Err();
            }
        }

        constexpr bool is_ok() const
        {
            return _initialized;
        }

        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        constexpr decltype(auto) unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                if constexpr (Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                if constexpr (Invocable<Err>)
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    panic(_err_data(), func, file_name, line);
                }
                else if constexpr (Result_detail::IsString<Err>)
                {
                    panic(_err_data.c_str(), func, file_name, line);
                }
                else
                {
                    panic(_err_data, func, file_name, line);
                }
            }
        }

        constexpr decltype(auto) expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                if constexpr (Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (Result_detail::IsReference<Ok>)
        {
            if (_initialized)
            {
                if constexpr (is_const<Ok>::value)
                {
                    return static_cast<const typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
            }
            else
            {
                return typename Ok::value_type {
                    forward<Args>(args)...
                };
            }
        } 

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (!Result_detail::IsReference<Ok>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{forward<Args>(args)...};
            }
        } 
        
        constexpr decltype(auto) unwrap_or_default()
        requires (DefaultConstructible<Ok>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{};
            }
        }

        template <typename Func>
        requires (InvocableRet<Ok, Func>)
        constexpr decltype(auto) unwrap_or_else(Func&& func)
        {
            if (_initialized)
            {
                if constexpr(Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                return func();
            }
        }

        constexpr decltype(auto) unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                if constexpr (Result_detail::IsReference<Err>)
                {
                    if constexpr (is_const<Err>::value)
                    {
                        return static_cast<const typename Err::value_type&>(_err_data);
                    }
                    else
                    {
                        return static_cast<typename Err::value_type&>(_err_data);
                    }
                }
                else
                {
                    return release(_err_data);
                }
            }
            else
            {
                panic("Expected Err, got Ok instead", func, file_name, line);
            }
        }

        constexpr decltype(auto) expect_err(
            const char* message = "Object was initialized",
            const char* func = __builtin_FUNCTION(), 
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                if constexpr (Result_detail::IsReference<Err>)
                {
                    if constexpr (is_const<Err>::value)
                    {
                        return static_cast<const typename Err::value_type&>(_err_data);
                    }
                    else
                    {
                        return static_cast<typename Err::value_type&>(_err_data);
                    }
                }
                else
                {
                    return release(_err_data);
                }
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }
    };

    template <typename Err>
    class [[nodiscard("Result type should not be discarded")]] Result<void, Err>
    {
    private:
        union {
            Err _err_data;
        };
        
        bool _initialized = false;

    public:
        constexpr Result()
            : _initialized{true}
        {}

        inline Result(const Result& other)
            : _initialized{other._initialized}
        {
            if (!_initialized)
            {
                _err_data = other._err_data;
            }
        }

        inline Result(Result&& other)
            : _initialized{other._initialized}
        {
            if (!_initialized)
            {
                _err_data = move(other._err_data);
            }
        }

        template <typename T>
        requires (Constructible<Err, T&&>)
        constexpr Result(T&& value)
            : _err_data{forward<Err>(value)}, _initialized{false}
        {}

        template <typename T>
        requires (Constructible<Err, const T&>)
        constexpr Result(const T& value)
            : _err_data{value}, _initialized{false}
        {}

        inline Result& operator=(Result&& rhs)
        {
            Result _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (!rhs._initialized)
            {
                new (&rhs._err_data) Err{move(_err_data)};
            }

            swap(_initialized, _tmp._initialized);

            if (!_initialized)
            {
                new (&_err_data) Err{move(_tmp._err_data)};
            }

            return *this;
        }

        constexpr ~Result()
        {
            if(!_initialized)
            {
                _err_data.~Err();
            }
        }
        
        constexpr bool is_ok() const
        {
            return _initialized;
        }

        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        constexpr void unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                if constexpr (Invocable<Err>)
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    panic(_err_data(), func, file_name, line);
                }
                else if constexpr (Result_detail::IsString<Err>)
                {
                    panic(_err_data.c_str(), func, file_name, line);
                }
                else
                {
                    panic(_err_data, func, file_name, line);
                }
            }
        }

        constexpr void expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename Func> 
        requires (InvocableRet<void, Func>)
        constexpr void unwrap_or_else(Func&& func)
        {
            if (!_initialized)
            {
                func();
            }
        }

        constexpr decltype(auto) unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic("Expected Err, got Ok instead", func, file_name, line);
            }
            else
            {
                if constexpr (Result_detail::IsReference<Err>)
                {
                    return static_cast<typename Err::value_type&>(_err_data);
                }
                else
                {
                    return release(_err_data);
                }
            }
        }

        constexpr decltype(auto) expect_err(
            const char* message = "Object was initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic(message, func, file_name, line);
            }
            else
            {
                if constexpr (Result_detail::IsReference<Err>)
                {
                    if constexpr (is_const<Err>::value)
                    {
                        return static_cast<const typename Err::value_type&>(_err_data);
                    }
                    else
                    {
                        return static_cast<typename Err::value_type&>(_err_data);
                    }
                }
                else
                {
                    return release(_err_data);
                }
            }
        }
    };

    template <typename Ok>
    class [[nodiscard("Result type should not be discarded")]] Result<Ok, void>
    {
    private:
        union {
            Ok _ok_data;
        };

        bool _initialized = false;
        
    public:
        constexpr Result()
            : _initialized{false}
        {}

        template <typename T>
        requires (Constructible<Ok, T&&>)
        constexpr Result(T&& value, ok_value = {})
            : _ok_data{forward<T>(value)}, _initialized{true}
        {}

        inline Result(const Result& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                new(&_ok_data) Ok{other._ok_data};
            }
        }

        inline Result(Result&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                new(&_ok_data) Ok{move(other._ok_data)};
            }
        }

        inline Result& operator=(Result&& rhs)
        {
            Result _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                new (&rhs._ok_data) Ok{move(_ok_data)};
            }

            swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                new (&_ok_data) Ok{move(_tmp._ok_data)};
            }

            return *this;
        }

        constexpr ~Result()
        {
            if(_initialized)
            {
                _ok_data.~Ok();
            }
        }

        constexpr bool is_ok() const
        {
            return _initialized;
        }

        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        constexpr decltype(auto) unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                if constexpr (Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                panic("Expected Ok, got Err instead", func, file_name, line);
            }
        }

        constexpr decltype(auto) expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                if constexpr (Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (Result_detail::IsReference<Ok>)
        {
            if (_initialized)
            {
                if constexpr (is_const<Ok>::value)
                {
                    return static_cast<const typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
            }
            else
            {
                return typename Ok::value_type {
                    forward<Args>(args)...
                };
            }
        } 

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (!Result_detail::IsReference<Ok>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{forward<Args>(args)...};
            }
        } 
        
        constexpr decltype(auto) unwrap_or_default()
        requires (DefaultConstructible<Ok>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{};
            }
        }

        template <typename Func>
        requires (InvocableRet<Ok, Func>)
        constexpr decltype(auto) unwrap_or_else(Func&& func)
        {
            if (_initialized)
            {
                if constexpr (Result_detail::IsReference<Ok>)
                {
                    return static_cast<typename Ok::value_type&>(_ok_data);
                }
                else
                {
                    return release(_ok_data);
                }
            }
            else
            {
                return func();
            }
        }

        constexpr void unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic("Expected Err, got Ok instead", func, file_name, line);
            }
        }

        constexpr void expect_err(
            const char* message = "Object was initialized",
            const char* func = __builtin_FUNCTION(), 
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic(message, func, file_name, line);
            }
        }
    };

    template <>
    class [[nodiscard("Result type should not be discarded")]] Result<void, void>
    {
    private:
        bool _initialized = false;
        
    public:
        constexpr Result()
            : _initialized{true}
        {}

        constexpr Result(err_value)
            : _initialized{false}
        {}

        constexpr Result(const Result& other)
            : _initialized{other._initialized}
        {}

        constexpr Result(Result&& other)
            : _initialized{other._initialized}
        {}

        constexpr Result& operator=(Result&& rhs)
        {
            swap(_initialized, rhs._initialized);
            return *this;
        }

        constexpr bool is_ok() const
        {
            return _initialized;
        }

        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        inline friend void swap(Result& lhs, Result& rhs)
        {
            swap(lhs._initialized, rhs._initialized);
        }

        constexpr void unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized == false)
            {
                panic("Expected Ok, got Err instead", func, file_name, line);
            }
        }

        constexpr void expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized == false)
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename Func>
        requires (InvocableRet<void, Func>)
        constexpr void unwrap_or_else(Func&& func)
        {
            if (_initialized == false)
            {
                return func();
            }
        }

        constexpr void unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic("Expected Err, got Ok instead", func, file_name, line);
            }
        }

        constexpr void expect_err(
            const char* message = "Object was initialized",
            const char* func = __builtin_FUNCTION(), 
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic(message, func, file_name, line);
            }
        }
    };

    template <typename T>
    using optional = Result<T, void>;
} // namespace hsd
