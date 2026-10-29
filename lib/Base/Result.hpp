#pragma once

/// @file Result.hpp
/// @brief Move-oriented success, optional, and error value types.
/// @details Defines result, option, option_err, runtime errors, panic helpers,
/// and unwrap/recovery operations used instead of exception-based control flow.

#include "../Wrappers/Pair.hpp"
#include "SourceLocation.hpp"
#include "Concepts.hpp"

#include <stdlib.h>
#include <string.h>

#if defined(HSD_PLATFORM_WINDOWS)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace hsd
{
    namespace result_detail
    {
        /// @brief Counts the characters before the terminating zero.
        /// @param text Null-terminated text.
        /// @return Number of characters in `text`.
        static consteval usize text_length(const char* text)
        {
            usize _length = 0;

            while (text[_length] != '\0')
            {
                ++_length;
            }

            return _length;
        }

        /// @brief Reports whether `prefix` occurs in `text` at position `pos`.
        /// @param text Null-terminated text to search.
        /// @param pos Position in `text` where the match must start.
        /// @param prefix Null-terminated text to match.
        /// @return True when every character of `prefix` matches.
        static consteval bool matches_at(const char* text, usize pos, const char* prefix)
        {
            for (usize _index = 0; prefix[_index] != '\0'; ++_index)
            {
                if (text[pos + _index] != prefix[_index])
                {
                    return false;
                }
            }

            return true;
        }

        /// @brief Finds the type name in a GCC or Clang signature.
        /// @details GCC prints `[with T = int]` and Clang prints `[T = int]`. The name
        /// starts after the first `= ` inside the first `[`, and ends before the final `]`.
        /// @param signature Pretty function name of a `get_type` instance.
        /// @return Pointer into `signature` and the length of the name.
        static consteval pair<const char*, i32> bracket_name(const char* signature)
        {
            const usize _size = text_length(signature);
            usize _begin = 0;

            while (_begin < _size && signature[_begin] != '[')
            {
                ++_begin;
            }

            while (_begin < _size && signature[_begin] != '=')
            {
                ++_begin;
            }

            // Skip the "= " that comes before the name.
            _begin += 2;

            return {signature + _begin, static_cast<i32>(_size - 1 - _begin)};
        }

        /// @brief Finds the type name in an MSVC signature.
        /// @details MSVC prints `get_type<int>(void)`, so the name lies between
        /// `get_type<` and the last `>` of the signature.
        /// @param signature Function signature of a `get_type` instance.
        /// @return Pointer into `signature` and the length of the name.
        static consteval pair<const char*, i32> angle_name(const char* signature)
        {
            const usize _size = text_length(signature);
            const usize _marker_size = text_length("get_type<");
            usize _begin = 0;

            while (_begin < _size && matches_at(signature, _begin, "get_type<") == false)
            {
                ++_begin;
            }

            _begin += _marker_size;

            usize _end = _size;

            while (_end > _begin && signature[_end - 1] != '>')
            {
                --_end;
            }

            return {signature + _begin, static_cast<i32>(_end - 1 - _begin)};
        }

        template <typename T>
        /// @brief Extracts a readable name of `T` from the compiler's signature for this function.
        /// @return Pointer and length of the extracted type name.
        static consteval pair<const char*, i32> get_type()
        {
            #if defined(HSD_COMPILER_MSVC)
            return angle_name(__FUNCSIG__);
            #else
            return bracket_name(__PRETTY_FUNCTION__);
            #endif
        }

        /// @brief Writes raw bytes to the standard error descriptor.
        /// @details Writes to the descriptor rather than through `stdio`, so the
        /// error paths in this header do not depend on `<stdio.h>`. A failed write
        /// stops quietly, because the caller aborts right after.
        /// @param text Bytes to write.
        /// @param size Number of bytes in `text`.
        static inline void write_error_bytes(const char* text, usize size)
        {
            while (size > 0)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                // The C runtime's descriptor writer. Text mode turns "\n" into "\r\n", as stderr did.
                const isize _written = _write(2, text, static_cast<unsigned int>(size));
                #else
                const isize _written = ::write(2, text, size);
                #endif

                if (_written <= 0)
                {
                    return;
                }

                text += _written;
                size -= static_cast<usize>(_written);
            }
        }

        /// @brief Writes a null-terminated string to the standard error descriptor.
        /// @param text Text to write; a null pointer writes nothing.
        static inline void write_error_piece(const char* text)
        {
            if (text != nullptr)
            {
                write_error_bytes(text, strlen(text));
            }
        }

        /// @brief Writes a string of another character type through its bytes.
        /// @details Stops at the first zero byte, as the `%s` conversion did before.
        /// @param text Null-terminated text of any character type.
        template <typename CharT>
        static inline void write_error_piece(const CharT* text)
        {
            write_error_piece(reinterpret_cast<const char*>(text));
        }

        /// @brief Writes an unsigned decimal number to the standard error descriptor.
        /// @param value Number to write.
        static inline void write_error_piece(u64 value)
        {
            // A u64 has at most 20 decimal digits.
            char _digits[20]{};
            usize _pos = sizeof(_digits);

            do
            {
                _digits[--_pos] = static_cast<char>('0' + value % 10);
                value /= 10;
            } while (value != 0);

            write_error_bytes(_digits + _pos, sizeof(_digits) - _pos);
        }

        /// @brief Writes each piece, in order, to the standard error descriptor.
        /// @param pieces Null-terminated strings and unsigned integers.
        template <typename... Pieces>
        static inline void write_error(const Pieces&... pieces)
        {
            (write_error_piece(pieces), ...);
        }
    } // namespace result_detail

    /// @brief Prints a source-aware diagnostic and terminates the process.
    /// @param message Human-readable failure description.
    /// @param loc Call-site source location attached to the diagnostic.
    [[noreturn]] static inline void panic(
        const char* message, source_location loc = source_location::current())
    {
        result_detail::write_error(
            "Got an error in file:\n", loc.file_name(), "\nInvoked from: ",
            loc.function_name(), " at line: ", static_cast<u64>(loc.line()),
            "\nWith the message: ", message, "\n"
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

        result_detail::write_error(
            "Got an error in file:\n", loc.file_name(), "\nInvoked from: ",
            loc.function_name(), " at line: ", static_cast<u64>(loc.line()),
            "\nWith the message: Expected "
        );
        result_detail::write_error_bytes(_ok_type.first, static_cast<usize>(_ok_type.second));
        result_detail::write_error(", got ");
        result_detail::write_error_bytes(_err_type.first, static_cast<usize>(_err_type.second));
        result_detail::write_error("\n");

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
