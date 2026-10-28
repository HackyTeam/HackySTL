#pragma once

/// @file Result.hpp
/// @brief Move-oriented success, optional, and error value types.
/// @details Defines result, option, option_err, runtime errors, panic helpers,
/// and unwrap/recovery operations used instead of exception-based control flow.

#include "../Wrappers/Pair.hpp"
#include "SourceLocation.hpp"
#include "Concepts.hpp"

#include <stdio.h>
#include <stdlib.h>

namespace hsd
{
    namespace result_detail
    {
        template <typename T>
        /// @brief Extracts a readable type name from the current function signature.
        /// @return Pointer and length of the extracted type name.
        static consteval pair<const char*, i32> get_type()
        {
            // Use source_location::current().function_name() for extracting the type name
            
            const char* _type_name = source_location::current().function_name();
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

    /// @brief Prints a source-aware diagnostic and terminates the process.
    /// @param message Human-readable failure description.
    /// @param loc Call-site source location attached to the diagnostic.
    [[noreturn]] static inline void panic(
        const char* message, source_location loc = source_location::current())
    {
        fprintf(
            stderr, "Got an error in file:"
            "\n%s\nInvoked from: %s at line:"
            " %u\nWith the message: %s\n", 
            loc.file_name(), loc.function_name(), loc.line(), message
        );

        abort();
    }

    /// @brief Terminates with a diagnostic describing an unexpected result type.
    /// @tparam Ok Expected successful type.
    /// @tparam Err Actual error type.
    /// @param loc Call-site source location attached to the diagnostic.
    template <typename Ok, typename Err>
    [[noreturn]] static inline void panic_type_err(
        source_location loc = source_location::current())
    {
        constexpr auto _ok_type = result_detail::get_type<Ok>();
        constexpr auto _err_type = result_detail::get_type<Err>();

        fprintf(
            stderr, "Got an error in file:"
            "\n%s\nInvoked from: %s at line:"
            " %u\nWith the message: Expected "
            "%.*s, got %.*s\n", loc.file_name(), loc.function_name(), 
            loc.line(), _ok_type.second, _ok_type.first,
            _err_type.second, _err_type.first
        );

        abort();
    }

    /// @brief Terminates with the standard unimplemented-operation diagnostic.
    /// @param loc Call-site source location attached to the diagnostic.
    [[noreturn]] static inline void unimplemented(
        source_location loc = source_location::current())
    {
        panic("This funcion is unimplemented", hsd::move(loc));
    }

    /// @brief Tag selecting the successful empty option state.
    struct ok_value {};
    /// @brief Tag selecting the unsuccessful empty option state.
    struct err_value {};
    
    /// @brief Lightweight project error carrying a stable message.
    class runtime_error
    {
    protected:
        /// @brief Stable diagnostic text owned by the caller or a string literal.
        const char* _err = nullptr;
        
    public:
        /// @brief Constructs and exposes the runtime error message.
        /// @param error Diagnostic text retained by the error object.
        constexpr runtime_error(const char* error)
            : _err{error}
        {}

        /// @brief Returns the stable diagnostic message.
        /// @return The retained diagnostic text.
        constexpr const char* pretty_error() const
        {
            return _err;
        }
    };

    /// @brief Discriminated success/error value with move-based extraction.
    /// @tparam Ok Successful value type.
    /// @tparam Err Error value type.
    template <typename Ok, typename Err> requires (!IsSame<Ok, Err>)
    class [[nodiscard("Result type should not be discarded")]] result
    {
    private:
        union
        {
            Ok _ok_data;
            Err _err_data;
        };

        /// @brief Selects whether the union contains `_ok_data` or `_err_data`.
        bool _initialized = false;
        
    public:
        /// @brief Constructs, inspects, unwraps, and recovers result values.
        /// @brief Constructs a successful result by copying `value`.
        /// @param value Successful value copied into the result.
        constexpr result(const Ok& value)
        requires (CopyConstructible<Ok>)
            : _ok_data{value}, _initialized{true}
        {}

        /// @brief Constructs a successful result by moving `value`.
        /// @param value Successful value moved into the result.
        constexpr result(Ok&& value)
        requires (MoveConstructible<Ok>)
            : _ok_data{hsd::move(value)}, _initialized{true}
        {}

        /// @brief Constructs an error result by copying `value`.
        /// @param value Error value copied into the result.
        constexpr result(const Err& value)
        requires (CopyConstructible<Err>)
            : _err_data{value}, _initialized{false}
        {}

        /// @brief Constructs an error result by moving `value`.
        /// @param value Error value moved into the result.
        constexpr result(Err&& value)
        requires (MoveConstructible<Err>)
            : _err_data{hsd::move(value)}, _initialized{false}
        {}

        constexpr result(const result&) = delete;
        constexpr result& operator=(const result&) = delete;

        /// @brief Moves the active alternative and its initialized state.
        /// @param other Result whose active alternative is moved.
        constexpr result(result&& other)
        {
            hsd::swap(_initialized, other._initialized);

            if(_initialized)
            {
                std::construct_at(&_ok_data, hsd::move(other._ok_data));
            }
            else
            {
                std::construct_at(&_err_data, hsd::move(other._err_data));
            }
        }

        /// @brief Replaces this result with a moved result state.
        /// @param rhs Result whose active value is moved.
        /// @return `*this` after replacement.
        constexpr result& operator=(result&& rhs)
        {
            result _tmp{hsd::move(rhs)};
            hsd::swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                std::construct_at(&rhs._ok_data, hsd::move(_ok_data));
            }
            else
            {
                std::construct_at(&rhs._err_data, hsd::move(_err_data));
            }

            hsd::swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                std::construct_at(&_ok_data, hsd::move(_tmp._ok_data));
            }
            else
            {
                std::construct_at(&_err_data, hsd::move(_tmp._err_data));
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

        /// @brief Reports whether the successful alternative is active.
        /// @return True when the result contains an `Ok` value.
        constexpr bool is_ok() const
        {
            return _initialized;
        }

        /// @brief Converts to the success-state boolean.
        /// @return True when the result contains an `Ok` value.
        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        /// @brief Moves out the success value or terminates on error.
        /// @param loc Source location used if termination is required.
        /// @return The moved success value.
        constexpr Ok unwrap(
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                if constexpr (requires (Err _err) {{_err.pretty_error()} -> IsSame<const char*>;})
                {
                    panic(_err_data.pretty_error(), hsd::move(loc));
                }
                else
                {
                    panic_type_err<Ok, Err>(hsd::move(loc));
                }
            }
        }

        /// @brief Moves out success or terminates with a caller-provided message.
        /// @param message Diagnostic used when the result contains an error.
        /// @param loc Source location used by the diagnostic.
        /// @return The moved success value.
        constexpr Ok expect(
            const char* message = "Object was not initialized",
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic(message, hsd::move(loc));
            }
        }

        /// @brief Moves out success or constructs a fallback value.
        /// @param args Arguments forwarded to the fallback `Ok` constructor.
        /// @return The stored success value or a constructed fallback.
        template <typename... Args>
        constexpr Ok unwrap_or(Args&&... args)
        requires (Constructible<Ok, Args...>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{hsd::forward<Args>(args)...};
            }
        }
        
        /// @brief Moves out success or value-initializes the fallback type.
        /// @return The stored success value or `Ok{}`.
        constexpr Ok unwrap_or_default()
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

        /// @brief Moves out success or invokes a lazy fallback function.
        /// @param func Callable invoked only when the result contains an error.
        /// @return The stored success value or the callable's fallback.
        template <typename Func>
        requires (InvocableRet<Ok, Func>)
        constexpr Ok unwrap_or_else(Func&& func)
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

        /// @brief Moves out the error value or terminates on success.
        /// @param loc Source location used if termination is required.
        /// @return The stored error value.
        constexpr Err unwrap_err(
            source_location loc = source_location::current())
        {
            if (!_initialized)
            {
                return release(_err_data);
            }
            else
            {
                panic_type_err<Err, Ok>(hsd::move(loc));
            }
        }

        /// @brief Moves out error or terminates with a caller-provided message.
        /// @param message Diagnostic used when the result contains success.
        /// @param loc Source location used by the diagnostic.
        /// @return The stored error value.
        constexpr Err expect_err(
            const char* message = "Object was initialized",
            source_location loc = source_location::current())
        {
            if (!_initialized)
            {
                return release(_err_data);
            }
            else
            {
                panic(message, hsd::move(loc));
            }
        }
    };

    /// @brief Optional successful value with an empty error state.
    /// @tparam Ok Stored successful value type.
    template <typename Ok>
    class [[nodiscard("Option type should not be discarded")]] option
    {
    private:
        union
        {
            Ok _ok_data;
        };

        /// @brief Selects whether `_ok_data` contains an engaged value.
        bool _initialized = false;
        
    public:
        /// @brief Constructs, inspects, unwraps, and recovers optional values.
        /// @brief Constructs an engaged option by copying a value.
        /// @param value Value copied into the option.
        constexpr option(const Ok& value)
        requires (CopyConstructible<Ok>)
            : _ok_data{value}, _initialized{true}
        {}

        /// @brief Constructs an engaged option by moving a value.
        /// @param value Value moved into the option.
        constexpr option(Ok&& value)
        requires (MoveConstructible<Ok>)
            : _ok_data{hsd::move(value)}, _initialized{true}
        {}

        /// @brief Constructs a disengaged option.
        constexpr option(err_value = {})
            : _initialized{false}
        {}

        /// @brief Moves the optional value and engagement state.
        /// @param other Option whose value and engagement state are moved.
        constexpr option(option&& other)
            : _initialized{other._initialized}
        {
            if(_initialized)
            {
                std::construct_at(&_ok_data, hsd::move(other._ok_data));
            }
        }

        constexpr option(const option&) = delete;
        constexpr option& operator=(const option&) = delete;

        /// @brief Replaces this option with a moved option state.
        /// @param rhs Option whose state is moved.
        /// @return `*this` after replacement.
        constexpr option& operator=(option&& rhs)
        {
            option _tmp = hsd::move(rhs);
            hsd::swap(_initialized, rhs._initialized);

            if (rhs._initialized)
            {
                std::construct_at(&rhs._ok_data, hsd::move(_ok_data));
            }

            hsd::swap(_initialized, _tmp._initialized);

            if (_initialized)
            {
                std::construct_at(&_ok_data, hsd::move(_tmp._ok_data));
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

        /// @brief Reports whether a value is engaged.
        /// @return True when the option contains a value.
        constexpr bool is_ok() const
        {
            return _initialized;
        }

        /// @brief Converts to the engagement boolean.
        /// @return True when the option contains a value.
        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        /// @brief Moves out the value or terminates if disengaged.
        /// @param loc Source location used if termination is required.
        /// @return The stored value.
        constexpr Ok unwrap(
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic_type_err<Ok, void>(hsd::move(loc));
            }
        }

        /// @brief Moves out the value or terminates with a custom message.
        /// @param message Diagnostic used when the option is disengaged.
        /// @param loc Source location used by the diagnostic.
        /// @return The stored value.
        constexpr Ok expect(
            const char* message = "Object was not initialized",
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                panic(message, hsd::move(loc));
            }
        }

        template <typename... Args>
        /// @brief Moves out the value or constructs a fallback.
        /// @param args Arguments forwarded to the fallback `Ok` constructor.
        /// @return The stored value or a constructed fallback.
        constexpr Ok unwrap_or(Args&&... args)
        requires (Constructible<Ok, Args...>)
        {
            if (_initialized)
            {
                return release(_ok_data);
            }
            else
            {
                return Ok{hsd::forward<Args>(args)...};
            }
        }
        
        /// @brief Moves out the value or default-constructs a fallback.
        /// @return The stored value or `Ok{}`.
        constexpr Ok unwrap_or_default()
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

        /// @brief Moves out the value or invokes a lazy fallback.
        /// @param func Callable invoked only when the option is disengaged.
        /// @return The stored value or the callable's fallback.
        template <typename Func>
        requires (InvocableRet<Ok, Func>)
        constexpr Ok unwrap_or_else(Func&& func)
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

        /// @brief Terminates if this option is engaged.
        /// @param loc Source location used by the diagnostic.
        constexpr void unwrap_err(
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                panic_type_err<void, Ok>(hsd::move(loc));
            }
        }

        /// @brief Terminates with a message if this option is engaged.
        /// @param message Diagnostic used when the option is engaged.
        /// @param loc Source location used by the diagnostic.
        constexpr void expect_err(
            const char* message = "Object was initialized",
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                panic(message, hsd::move(loc));
            }
        }
    };

    /// @brief Optional error value with an empty successful state.
    /// @tparam Err Stored error type.
    template <typename Err>
    class [[nodiscard("Option type should not be discarded")]] option_err
    {
    private:
        union
        {
            Err _err_data;
        };

        /// @brief Selects whether the object contains success or `_err_data`.
        bool _initialized = false;
        
    public:
        /// @brief Constructs an error-bearing option by copying an error.
        /// @brief Constructs an error-bearing option by copying `value`.
        /// @param value Error copied into the option.
        constexpr option_err(const Err& value)
        requires (CopyConstructible<Err>)
            : _err_data{value}, _initialized{false}
        {}

        /// @brief Constructs an error-bearing option by moving an error.
        /// @brief Constructs an error-bearing option by moving `value`.
        /// @param value Error moved into the option.
        constexpr option_err(Err&& value)
        requires (MoveConstructible<Err>)
            : _err_data{hsd::move(value)}, _initialized{false}
        {}

        /// @brief Constructs a successful option with no error payload.
        /// @brief Constructs a successful option_err with no error payload.
        constexpr option_err(ok_value = {})
            : _initialized{true}
        {}

        /// @brief Moves the error payload and success state.
        /// @brief Moves the error payload and success state.
        /// @param other Option whose error payload and state are moved.
        constexpr option_err(option_err&& other)
            : _initialized{other._initialized}
        {
            if(!_initialized)
            {
                std::construct_at(&_err_data, hsd::move(other._err_data));
            }
        }

        constexpr option_err(const option_err&) = delete;
        constexpr option_err& operator=(const option_err&) = delete;

        /// @brief Replaces this state with a moved option_err.
        /// @brief Replaces this option_err with a moved state.
        /// @param rhs Option whose state is moved.
        /// @return `*this` after replacement.
        constexpr option_err& operator=(option_err&& rhs)
        {
            option_err _tmp = hsd::move(rhs);
            hsd::swap(_initialized, rhs._initialized);

            if (!rhs._initialized)
            {
                std::construct_at(&rhs._err_data, hsd::move(_err_data));
            }

            hsd::swap(_initialized, _tmp._initialized);

            if (!_initialized)
            {
                std::construct_at(&_err_data, hsd::move(_tmp._err_data));
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

        /// @brief Reports whether no error is stored.
        /// @brief Reports whether no error is stored.
        /// @return True when the option represents successful completion.
        constexpr bool is_ok() const
        {
            return _initialized;
        }

        /// @brief Converts to the no-error boolean.
        /// @return True when no error is stored.
        explicit constexpr operator bool() const
        {
            return _initialized;
        }

        /// @brief Terminates if an error is stored.
        /// @param loc Source location used by the diagnostic.
        constexpr void unwrap(
            source_location loc = source_location::current())
        {
            if (!_initialized)
            {
                if constexpr (requires (Err _err) {{_err.pretty_error()} -> IsSame<const char*>;})
                {
                    panic(_err_data.pretty_error(), hsd::move(loc));
                }
                else
                {
                    panic_type_err<void, Err>(hsd::move(loc));
                }
            }
        }

        /// @brief Terminates with a custom message if an error is stored.
        /// @param message Diagnostic used when an error is stored.
        /// @param loc Source location used by the diagnostic.
        constexpr void expect(
            const char* message = "Object was not initialized",
            source_location loc = source_location::current())
        {
            if (!_initialized)
            {
                panic(message, hsd::move(loc));
            }
        }

        /// @brief Moves out the error or terminates when successful.
        /// @brief Moves out the error or terminates when successful.
        /// @param loc Source location used by the diagnostic.
        /// @return The stored error value.
        constexpr Err unwrap_err(
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                panic_type_err<Err, void>(hsd::move(loc));
            }
            else
            {
                return release(_err_data);
            }
        }

        /// @brief Moves out the error or terminates with a custom message.
        /// @brief Moves out the error or terminates with a custom message.
        /// @param message Diagnostic used when the option is successful.
        /// @param loc Source location used by the diagnostic.
        /// @return The stored error value.
        constexpr Err expect_err(
            const char* message = "Object was initialized",
            source_location loc = source_location::current())
        {
            if (_initialized)
            {
                panic(message, hsd::move(loc));
            }
            else
            {
                return release(_err_data);
            }
        }
    };
} // namespace hsd
