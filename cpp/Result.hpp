#pragma once

#include "Reference.hpp"
#include <bits/stl_construct.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(HSD_COMPILER_MSVC)
    #define HSD_FUNCTION_NAME __FUNCSIG__
#elif defined(HSD_COMPILER_GCC) || defined(HSD_COMPILER_CLANG)
    #define HSD_FUNCTION_NAME __PRETTY_FUNCTION__
#else
    #define HSD_FUNCTION_NAME __builtin_FUNCTION()
#endif

namespace hsd
{
    namespace Result_detail
    {
        template <typename From, typename To>
        concept is_same = hsd::is_same<From, To>::value;

        template <typename T>
        using InvokeType = decltype(std::declval<T>()());

        template < typename Func, typename Result >
        concept UnwrapInvocable = requires(Func a) 
        {
            {a()} -> is_same<Result>;
        };

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

    template < typename Ok, typename Err >
    class Result
    {
    private:
        union
        {
            Ok _ok_data;
            Err _err_data;
        };

        bool _initialized = false;
        
    public:
        HSD_CONSTEXPR Result& operator=(const Result&) = delete;
        HSD_CONSTEXPR Result& operator=(Result&&) = delete;

        HSD_CONSTEXPR Result(const Ok& value)
            : _ok_data{value}, _initialized{true}
        {}

        HSD_CONSTEXPR Result(Ok&& value)
            : _ok_data{move(value)}, _initialized{true}
        {}

        template <typename T>
        requires (std::is_convertible_v<T, Err>)
        HSD_CONSTEXPR Result(T&& value)
            : _err_data{forward<T>(value)}, _initialized{false}
        {}

        HSD_CONSTEXPR Result(const Result& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                std::construct_at(&_ok_data, other._ok_data);
            }
            else
            {
                std::construct_at(&_err_data, other._err_data);
            }
        }

        HSD_CONSTEXPR Result(Result&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                std::construct_at(&_ok_data, move(other._ok_data));
            }
            else
            {
                std::construct_at(&_err_data, move(other._err_data));
            }
        }

        HSD_CONSTEXPR ~Result()
        {
            if(_initialized)
            {
                _ok_data.~Ok();
            }
            else
            {
                _err_data.~Err();
            }
        }

        constexpr decltype(auto) unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE()) const
        {
            if(_initialized)
            {
                if constexpr(Result_detail::IsReference<Ok>)
                {
                    if constexpr(is_const<Ok>::value)
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
                    return _ok_data;
                }
            }
            else
            {
                if constexpr(requires {_err_data();})
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err result value:"
                        " %s\n", file_name, func, line, 
                        _err_data()
                    );
                }
                else if constexpr(Result_detail::IsString<Err>)
                {
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, 
                        _err_data.c_str()
                    );
                }
                else
                {
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, _err_data
                    );
                }

                abort();
            }
        }

        constexpr decltype(auto) expect(
            const char* message = "Got an Error instead",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE()) const
        {
            if(_initialized)
            {
                if constexpr(Result_detail::IsReference<Ok>)
                {
                    if constexpr(is_const<Ok>::value)
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
                    return _ok_data;
                }
            }
            else
            {
                fprintf(
                    stderr, "Got type error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
            }
        }

        template <typename... Args>
        constexpr decltype(auto) unwrap_or(Args&&... args) const
        requires (Result_detail::IsReference<Ok>)
        {
            if(_initialized)
            {
                if constexpr(is_const<Ok>::value)
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
                if constexpr(is_const<Ok>::value)
                {
                    return static_cast<const typename Ok::value_type&>(
                        Ok{forward<Args>(args)...}
                    );
                }
                else
                {
                    return static_cast<typename Ok::value_type&>(
                        Ok{forward<Args>(args)...}
                    );
                }
            }
        } 

        template <typename... Args>
        constexpr decltype(auto) unwrap_or(Args&&... args) const
        requires (!Result_detail::IsReference<Ok>)
        {
            if(_initialized)
            {
                return _ok_data;
            }
            else
            {
                return Ok{forward<Args>(args)...};
            }
        } 
        
        constexpr decltype(auto) unwrap_or_default() const
        requires (std::is_default_constructible_v<Ok>)
        {
            if(_initialized)
            {
                return _ok_data;
            }
            else
            {
                return Ok{};
            }
        }

        template <typename Func>
        requires (Result_detail::UnwrapInvocable<Func, Ok>)
        constexpr decltype(auto) unwrap_or_else(Func&& func) const
        {
            if(_initialized)
            {
                if constexpr(Result_detail::IsReference<Ok>)
                {
                    if constexpr(is_const<Ok>::value)
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
                    return _ok_data;
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
            usize line = __builtin_LINE()) const
        {
            if(!_initialized)
            {
                if constexpr(Result_detail::IsReference<Err>)
                {
                    if constexpr(is_const<Err>::value)
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
                    return _err_data;
                }
            }
            else
            {
                fprintf(
                    stderr, "Got type error in file:\n"
                    "%s\nInvoked from: %s at line: %zu"
                    "\nExpected Err, got Ok instead\n",
                    file_name, func, line
                );
                abort();
            }
        }

        constexpr decltype(auto) expect_err(
            const char* message = "Got a Value instead",
            const char* func = __builtin_FUNCTION(), 
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE()) const
        {
            if(!_initialized)
            {
                if constexpr(Result_detail::IsReference<Err>)
                {
                    if constexpr(is_const<Err>::value)
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
                    return _err_data;
                }
            }
            else
            {
                fprintf(
                    stderr, "Got type error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
            }
        }
    };

    template <typename Err>
    class Result<void, Err>
    {
    private:
        union {
            Err _err_data;
        };
        
        bool _initialized = false;

    public:
        HSD_CONSTEXPR Result(const Result&) = delete;
        HSD_CONSTEXPR Result(Result&&) = delete;
        HSD_CONSTEXPR Result& operator=(const Result&) = delete;
        HSD_CONSTEXPR Result& operator=(Result&&) = delete;
        constexpr void unwrap_or_default() = delete;
        constexpr void unwrap_or() = delete;

        HSD_CONSTEXPR Result()
            : _initialized{true}
        {}

        template <typename T>
        requires (std::is_convertible_v<T, Err>)
        HSD_CONSTEXPR Result(T&& value)
            : _err_data{forward<Err>(value)}, _initialized{false}
        {}

        HSD_CONSTEXPR ~Result()
        {
            if(!_initialized)
            {
                _err_data.~Err();
            }
        }

        constexpr void unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE()) const
        {
            if(!_initialized)
            {
                if constexpr(requires {_err_data();})
                {
                    static_assert(
                        is_same<const char*, Result_detail::InvokeType<Err>>::value, 
                        "Error: Invoke result type is not same with \'const char*\'"
                    );
                    
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err result value:"
                        " %s\n", file_name, func, line, 
                        _err_data()
                    );
                }
                else if constexpr(Result_detail::IsString<Err>)
                {
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, 
                        _err_data.c_str()
                    );
                }
                else
                {
                    fprintf(
                        stderr, "Got type error in file:"
                        "\n%s\nInvoked from: %s at line:"
                        " %zu\nWith the Err value: %s\n",
                        file_name, func, line, _err_data
                    );
                }
                
                abort();
            }
        }

        constexpr void expect(
            const char* message = "Got an Error instead",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE()) const
        {
            if(!_initialized)
            {
                fprintf(
                    stderr, "Got type error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message
                );
                abort();
            }
        }

        template <typename Func>
        requires (Result_detail::UnwrapInvocable<Func, void>)
        constexpr void unwrap_or_else(Func&& func) const
        {
            if(!_initialized)
            {
                func();
            }
        }

        constexpr decltype(auto) unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE()) const
        {
            if(_initialized)
            {
                fprintf(
                    stderr, "Got type error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\n", file_name, func, line
                );
                abort();
            }
            else
            {
                if constexpr(Result_detail::IsReference<Err>)
                {
                    return static_cast<typename Err::value_type&>(_err_data);
                }
                else
                {
                    return _err_data;
                }
            }
        }

        constexpr decltype(auto) expect_err(
            const char* message = "Got a Value instead",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE()) const
        {
            if(_initialized)
            {
                fprintf(
                    stderr, "Got type error in file:"
                    "\n%s\nInvoked from: %s at line:"
                    " %zu\nWith the message: %s\n",
                    file_name, func, line, message);
                abort();
            }
        }
    };
} // namespace hsd