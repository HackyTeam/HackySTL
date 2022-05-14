#pragma once

#include "Concepts.hpp"
#include "Pair.hpp"

// Required to unlock std::construct_at which
// allows us to in-place construct at compile time 
#if __has_include(<bits/stl_construct.h>)
#include <bits/stl_construct.h>
#endif

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

namespace hsd
{

    namespace result_detail
    {
        template <typename T>
        static consteval pair<const char*, i32> get_type()
        {
            // Use __PRETTY_FUNCTION__ for extracting the type name
            
            const char* _type_name = __PRETTY_FUNCTION__;
            const char* _ptr = _type_name;

            for (; *_type_name != '\0'; ++_type_name)
            {
                // Find the first =

                if (*_type_name == '=')
                {
                    _ptr = _type_name + 2;
                }
            }

            return {_ptr, static_cast<i32>(_type_name - _ptr - 1)};
        }
    } // namespace result_detail

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

    template <typename Ok, typename Err>
    [[noreturn]] static inline void panic_type_err(
        const char* func = __builtin_FUNCTION(),
        const char* file_name = __builtin_FILE(),
        usize line = __builtin_LINE())
    {
        constexpr auto _ok_type = result_detail::get_type<Ok>();
        constexpr auto _err_type = result_detail::get_type<Err>();

        fprintf(
            stderr, "Got an error in file:"
            "\n%s\nInvoked from: %s at line:"
            " %zu\nWith the message: Expected "
            "%.*s, got %.*s\n", file_name, func, 
            line, _ok_type.second, _ok_type.first,
            _err_type.second, _err_type.first
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

        constexpr const char* pretty_error() const
        {
            return _err;
        }
    };

    template <typename Ok, typename Err> requires (!IsSame<Ok, Err>)
    class [[nodiscard("Result type should not be discarded")]] result
    {
    private:
        union
        {
            Ok _ok_data;
            Err _err_data;
        };

        bool _initialized = false;
        
    public:
        constexpr result(const Ok& value)
        requires (CopyConstructible<Ok>)
            : _ok_data{value}, _initialized{true}
        {}

        constexpr result(Ok&& value)
        requires (MoveConstructible<Ok>)
            : _ok_data{move(value)}, _initialized{true}
        {}

        constexpr result(const Err& value)
        requires (CopyConstructible<Err>)
            : _err_data{value}, _initialized{false}
        {}

        constexpr result(Err&& value)
        requires (MoveConstructible<Err>)
            : _err_data{move(value)}, _initialized{false}
        {}

        constexpr result(const result&) = delete;
        constexpr result& operator=(const result&) = delete;

        constexpr result(result&& other)
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

        constexpr result& operator=(result&& rhs)
        {
            result _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                std::construct_at(&rhs._ok_data, move(_ok_data));
            }
            else
            {
                std::construct_at(&rhs._err_data, move(_err_data));
            }

            swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                std::construct_at(&_ok_data, move(_tmp._ok_data));
            }
            else
            {
                std::construct_at(&_err_data, move(_tmp._err_data));
            }

            return *this;
        }

        constexpr ~result()
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

        constexpr auto unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                if constexpr (requires (Err _err) {_err.pretty_error() -> template IsSame<const char*>;})
                {
                    panic(_err_data.pretty_error(), func, file_name, line);
                }
                else
                {
                    panic_type_err<Ok, Err>(func, file_name, line);
                }
            }
        }

        constexpr auto expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (Constructible<Ok, Args...>)
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
        
        constexpr auto unwrap_or_default()
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
        constexpr auto unwrap_or_else(Func&& func)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return func();
            }
        }

        constexpr auto unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                return release(_err_data);
            }
            else
            {
                panic_type_err<Err, Ok>(func, file_name, line);
            }
        }

        constexpr auto expect_err(
            const char* message = "Object was initialized",
            const char* func = __builtin_FUNCTION(), 
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                return release(_err_data);
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }
    };

    template <typename Ok>
    class [[nodiscard("Option type should not be discarded")]] option
    {
    private:
        union
        {
            Ok _ok_data;
        };

        bool _initialized = false;
        
    public:
        constexpr option(const Ok& value)
        requires (CopyConstructible<Ok>)
            : _ok_data{value}, _initialized{true}
        {}

        constexpr option(Ok&& value)
        requires (MoveConstructible<Ok>)
            : _ok_data{move(value)}, _initialized{true}
        {}

        constexpr option(err_value = {})
            : _initialized{false}
        {}

        constexpr option(option&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                std::construct_at(&_ok_data, move(other._ok_data));
            }
        }

        constexpr option(const option&) = delete;
        constexpr option& operator=(const option&) = delete;

        constexpr option& operator=(option&& rhs)
        {
            option _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                std::construct_at(&rhs._ok_data, move(_ok_data));
            }

            swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                std::construct_at(&_ok_data, move(_tmp._ok_data));
            }

            return *this;
        }

        constexpr ~option()
        {
            if (_initialized)
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

        constexpr auto unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic_type_err<Ok, void>(func, file_name, line);
            }
        }

        constexpr auto expect(
            const char* message = "Object was not initialized",
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic(message, func, file_name, line);
            }
        }

        template <typename... Args>
        constexpr auto unwrap_or(Args&&... args)
        requires (Constructible<Ok, Args...>)
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
        
        constexpr auto unwrap_or_default()
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
        constexpr auto unwrap_or_else(Func&& func)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return func();
            }
        }

        constexpr auto unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic_type_err<void, Ok>(func, file_name, line);
            }
        }

        constexpr auto expect_err(
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

    template <typename Err>
    class [[nodiscard("Option type should not be discarded")]] option_err
    {
    private:
        union
        {
            Err _err_data;
        };

        bool _initialized = false;
        
    public:
        constexpr option_err(const Err& value)
        requires (CopyConstructible<Err>)
            : _err_data{value}, _initialized{false}
        {}

        constexpr option_err(Err&& value)
        requires (MoveConstructible<Err>)
            : _err_data{move(value)}, _initialized{false}
        {}

        constexpr option_err(ok_value = {})
            : _initialized{true}
        {}

        constexpr option_err(option_err&& other)
            : _initialized{other._initialized}
        {
            if(!_initialized)
            {
                std::construct_at(&_err_data, move(other._err_data));
            }
        }

        constexpr option_err(const option_err&) = delete;
        constexpr option_err& operator=(const option_err&) = delete;

        constexpr option_err& operator=(option_err&& rhs)
        {
            option_err _tmp = move(rhs);
            swap(_initialized, rhs._initialized);

            if (!rhs._initialized)
            {
                std::construct_at(&rhs._err_data, move(_err_data));
            }

            swap(_initialized, _tmp._initialized);

            if (!_initialized)
            {
                std::construct_at(&_err_data, move(_tmp._err_data));
            }

            return *this;
        }

        constexpr ~option_err()
        {
            if (!_initialized)
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

        constexpr auto unwrap(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(), 
            usize line = __builtin_LINE())
        {
            if (!_initialized)
            {
                if constexpr (requires (Err _err) {_err.pretty_error() -> template IsSame<const char*>;})
                {
                    panic(_err_data.pretty_error(), func, file_name, line);
                }
                else
                {
                    panic_type_err<void, Err>(func, file_name, line);
                }
            }
        }

        constexpr auto expect(
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

        constexpr auto unwrap_err(
            const char* func = __builtin_FUNCTION(),
            const char* file_name = __builtin_FILE(),
            usize line = __builtin_LINE())
        {
            if (_initialized)
            {
                panic_type_err<Err, void>(func, file_name, line);
            }
            else
            {
                return release(_err_data);
            }
        }

        constexpr auto expect_err(
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
                return release(_err_data);
            }
        }
    };
} // namespace hsd
