#pragma once

/// @file ArgParse.hpp
/// @brief Typed command-line argument registration and dispatch.
/// @details Associates option names with callback functions and parser streams,
/// supporting grouped arguments, help information, and typed extraction.

#include "../Containers/UnorderedMap.hpp"
#include "../Wrappers/Functional.hpp"
#include "_SStreamDetail.hpp"
#include "../Containers/StringView.hpp"
#include "Io.hpp"

namespace hsd
{
    /// @brief Typed view over arguments supplied to an argument callback.
    class parser_stream
    {
    private:
        /// @brief Raw argument tokens collected for the current dispatch.
        vector<string_view> _args_buf;

    public:
        // It's a non-movable and copyable type
        inline parser_stream(const parser_stream&) = delete;
        inline parser_stream(parser_stream&&) = delete;
        inline parser_stream& operator=(const parser_stream&) = delete;
        inline parser_stream& operator=(parser_stream&&) = delete;
        inline parser_stream() = default;
        inline ~parser_stream() = default;

        ///
        /// @brief Parses buffered argument tokens into typed output variables.
        ///
        /// @tparam Args - Destination types, one per positional token.
        /// @param args - Output variables assigned from tokens 0..sizeof...(Args)-1
        /// via `sstream_detail::_parse_from`, in declaration order.
        /// @return No error on success; a `runtime_error` when fewer tokens are
        /// buffered than requested outputs, or when any token fails to parse
        /// (the first parse failure short-circuits the remaining conversions).
        ///
        template <typename... Args>
        inline option_err<runtime_error> set_data(Args&... args)
        {
            if (sizeof...(Args) > _args_buf.size())
            {
                return runtime_error{"Input too small to parse"};
            }
            else
            {
                option_err<runtime_error> _error{};
                tuple<reference<Args>...> _tup = {reference<Args>{args}...};
                
                auto _set_func = [&_error](const auto& data, auto& arg)
                {
                    using sstream_detail::_parse_from;

                    if (_error.is_ok() == true)
                    {
                        _error = _parse_from(data, arg);
                    }
                };

                [&]<usize... Ints>(index_sequence<Ints...>)
                {
                    (_set_func(_args_buf[Ints], _tup.template get<Ints>()), ...);
                }(make_index_sequence<sizeof...(Args)>{});

                if (_error.is_ok() == false)
                {
                    return _error.unwrap_err();
                }
            }

            return {};
        }

        ///
        /// @brief Appends a raw argument token to the buffer.
        ///
        /// @param arg - Null-terminated token to append.
        ///
        inline void emplace(const char* arg)
        {
            _args_buf.emplace_back(arg);
        }

        ///
        /// @brief Clears all buffered argument tokens.
        ///
        inline void clear()
        {
            _args_buf.clear();
        }
    };

    ///
    /// @brief Registers command-line options and dispatches parser streams.
    ///
    class argument_parser
    {
    private:
        /// @brief Callback signature invoked with the tokens for one option.
        using function_type = function<void(parser_stream&)>;
        /// @brief Callback paired with the number of tokens it consumes.
        using action_type = pair<function_type, usize>;
        /// @brief Registered options keyed by their argument name.
        unordered_map<string_view, action_type> _actions;
        /// @brief Help text lines printed by `print_info`, program info first.
        vector<string_view> _informations;

    public:
        ///
        /// @brief Starts a parser with a leading program description line.
        ///
        /// @param info - Program description shown first by `print_info`.
        ///
        inline argument_parser(const string_view& info)
        {
            _informations.emplace_back(info);
        }
        
        inline argument_parser(const argument_parser&) = delete;
        inline argument_parser(argument_parser&&) = delete;
        inline argument_parser& operator=(const argument_parser&) = delete;
        inline argument_parser& operator=(argument_parser&&) = delete;
        inline ~argument_parser() = default;

        ///
        /// @brief Registers an option name with its callback and help text.
        ///
        /// @param argument - Option name used to match a command-line token.
        /// @param num_args - Number of following tokens forwarded to `func`.
        /// @param func - Callback invoked with a `parser_stream` holding the
        /// `num_args` tokens that follow `argument`.
        /// @param help - Help line appended for this option.
        ///
        inline void add(
            const string_view& argument, usize num_args,
            function_type&& func, const string_view& help)
        {
            _actions.emplace(argument, func, num_args);
            _informations.emplace_back(help);
        }

        ///
        /// @brief Prints every registered informational/help line.
        ///
        inline void print_info()
        {
            // Printed through the library's own output, so it stays in order with
            // other hsd output. A view is printed by its length, not as a C string.
            using namespace format_literals;

            for (const auto& _info : _informations)
            {
                hsd::println("{}"_fmt, _info);
            }
        }

        ///
        /// @brief Dispatches registered option callbacks over `argv`.
        ///
        /// @param argc - Argument count, including the program name at index 0.
        /// @param argv - Argument vector; `-h`/`--help` prints info and falls
        /// through to normal dispatch. Each recognized option name consumes
        /// its registered `num_args` following tokens before advancing to the
        /// next option name. An unrecognized option name prints info; lookup
        /// then still proceeds to `unwrap()` the failed action, so an unknown
        /// option aborts rather than being skipped.
        ///
        inline void parse(i32 argc, const char** argv)
        {
            if (argc == 1)
            {
                return;
            }

            // Help is shown only when no option is registered under that name.
            const bool _help_requested =
                cstring::compare(argv[1], "-h") == 0 ||
                cstring::compare(argv[1], "--help") == 0;

            if (_help_requested && !_actions.at(argv[1]).is_ok())
            {
                print_info();
                return;
            }

            i32 _index = 1;
            parser_stream _buf;

            while (_index < argc)
            {
                auto _action = _actions.at(argv[_index]);

                if (!_action.is_ok())
                {
                    print_info();
                    return;
                }

                auto& [_function, _num_args] = _action
                    .unwrap().get();

                // Stop before reading values the command line does not contain.
                if (_index + 1 + static_cast<i32>(_num_args) > argc)
                {
                    print_info();
                    return;
                }

                for (i32 _offset = 1; _offset <= static_cast<i32>(_num_args); _offset++)
                {
                    _buf.emplace(argv[_index + _offset]);
                }

                _function(_buf).unwrap();
                _index += static_cast<i32>(_num_args) + 1;
                _buf.clear();
            }
        }
    };
} // namespace hsd
