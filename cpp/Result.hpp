#pragma once

#include "Reference.hpp"
#include "Concepts.hpp"

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#define hsd_fprint_check(stream, format, ...)\
do\
{\
    if(fprintf(stream, format __VA_OPT__(,) __VA_ARGS__) == -1)\
        fwprintf(stream, L##format __VA_OPT__(,) __VA_ARGS__);\
} while (0)

#define hsd_fputs_check(stream, string)\
do\
{\
    if(fputs(string "\n", stream) == -1)\
        fputws(L##string L"\n", stream);\
} while (0)

#define hsd_panic(message)\
do\
{\
    hsd_fprint_check(\
        stderr, "Got an error in file:"\
        "\n%s\nInvoked from: %s at line:"\
        " %zu\nWith the Err result value:"\
        " %s\n", __FILE__, \
        __builtin_FUNCTION(), \
        __LINE__, message\
    );\
    \
    abort();\
} while (0) 

#define hsd_unimplemented()\
do\
{\
    hsd_fprint_check(\
        stderr, "Got an error in file:"\
        "\n%s\nInvoked from: %s at line:"\
        " %zu\nWith the Err result value:"\
        " This funcion is unimplemented\n", \
        __FILE__, __builtin_FUNCTION(), __LINE__\
    );\
    \
    abort();\
} while (0) 

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
        constexpr Result& operator=(const Result&) = delete;
        constexpr Result& operator=(Result&&) = delete;

        template <typename T>
        requires (std::is_constructible_v<Ok, T&&>)
        constexpr Result(T&& value, ok_value = {})
            : _ok_data{forward<T>(value)}, _initialized{true}
        {}

        template <typename T>
        requires (std::is_constructible_v<Err, T&&>)
        constexpr Result(T&& value, err_value = {})
            : _err_data{forward<T>(value)}, _initialized{false}
        {}

        Result(const Result& other)
            : _initialized{other._initialized}
        {
            if (_initialized)
            {
                new (&_ok_data) Ok(other._ok_data);
            }
            else
            {
                new (&_err_data) Err(other._err_data);
            }
        }

        Result(Result&& other)
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
                if constexpr (requires {_err_data();})
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err result value:"
                        " %s\n", file_name, func, line, 
                        _err_data()
                    );
                }
                else if constexpr (Result_detail::IsString<Err>)
                {
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, 
                        _err_data.c_str()
                    );
                }
                else
                {
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, _err_data
                    );
                }

                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
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
        requires (std::is_default_constructible_v<Ok>)
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
                hsd_fprint_check(
                    stderr, "Got an error in file:\n"
                    "%s\nInvoked from: %s at line: %zu"
                    "\nExpected Err, got Ok instead\n",
                    file_name, func, line
                );
                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
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
        constexpr Result(const Result&) = delete;
        constexpr Result(Result&&) = delete;
        constexpr Result& operator=(const Result&) = delete;
        constexpr Result& operator=(Result&&) = delete;
        constexpr void unwrap_or_default() = delete;
        constexpr void unwrap_or() = delete;

        constexpr Result()
            : _initialized{true}
        {}

        template <typename T>
        requires (std::is_constructible_v<Err, T&&>)
        constexpr Result(T&& value)
            : _err_data{forward<Err>(value)}, _initialized{false}
        {}

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
                if constexpr (requires {_err_data();})
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err result value:"
                        " %s\n", file_name, func, line, 
                        _err_data()
                    );
                }
                else if constexpr (Result_detail::IsString<Err>)
                {
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, 
                        _err_data.c_str()
                    );
                }
                else
                {
                    hsd_fprint_check(
                        stderr, "Got an error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, _err_data
                    );
                }
                
                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\n", file_name, func, line
                );
                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message);
                abort();
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
        constexpr Result& operator=(const Result&) = delete;
        constexpr Result& operator=(Result&&) = delete;

        constexpr Result()
            : _initialized{false}
        {}

        template <typename T>
        requires (std::is_constructible_v<Ok, T&&>)
        constexpr Result(T&& value, ok_value = {})
            : _ok_data{forward<T>(value)}, _initialized{true}
        {}

        Result(const Result& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                new(&_ok_data) Ok(other._ok_data);
            }
        }

        Result(Result&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                new(&_ok_data) Ok(move(other._ok_data));
            }
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\n", file_name, func, line
                );

                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
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
        requires (std::is_default_constructible_v<Ok>)
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
                hsd_fprint_check(
                    stderr, "Got an error in file:\n"
                    "%s\nInvoked from: %s at line: %zu"
                    "\nExpected Err, got Ok instead\n",
                    file_name, func, line
                );
                abort();
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
                hsd_fprint_check(
                    stderr, "Got an error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
            }
        }
    };

    template <typename T>
    using optional = Result<T, void>;
} // namespace hsd
