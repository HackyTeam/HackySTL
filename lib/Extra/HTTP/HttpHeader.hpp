#pragma once

/// @file HttpHeader.hpp
/// @brief Case-insensitive HTTP header storage and parsing.
/// @details Parses CRLF-delimited fields into an unordered map and exposes
/// normalized lookup and mutation operations.

#include "../../Containers/UnorderedMap.hpp"
#include "../../Containers/String.hpp"

namespace hsd::http
{
    /// @brief Case-insensitive storage and CRLF parser for HTTP header fields.
    /// @details Field names are lower-cased before being stored or looked up,
    /// so lookups are insensitive to the case used on the wire.
    class header
    {
    private:
        /// @brief Lower-cased field name to raw field value.
        unordered_map<string, string> _fields{};
        /// @brief Shared scratch buffer reused to lower-case a lookup/insert key.
        /// @details Static (shared across all `header` instances) and mutated
        /// on every #get_field/#set_field call, so concurrent access to
        /// different `header` objects from multiple threads is not safe.
        static inline string _buffer{};

        /// @brief Error raised when a header block fails to parse.
        class header_error
        {
        private:
            /// @brief Formatted "message line-number" diagnostic text.
            string _err;

        public:
            /// @brief Builds the diagnostic by appending the failing line number.
            /// @param msg Human-readable description of the parse failure.
            /// @param line 1-based header line number where the failure occurred.
            inline header_error(const string_view& msg, usize line)
                : _err{msg + (" " + to_string(line))}
            {}

            /// @brief Returns the formatted diagnostic message.
            /// @return Null-terminated "message line-number" text.
            const char* pretty_error() const
            {
                return _err.c_str();
            }
        };

    public:
        /// @brief Looks up a header field by name, case-insensitively.
        /// @details Lower-cases `key` into the shared scratch buffer before
        /// the lookup, so `key` itself is left unmodified.
        /// @param key Field name to look up (any case).
        /// @return Reference to the stored field value.
        inline auto get_field(const string_view& key) const
        {
            // Because HTTP header fields are case-insensitive for some reason
            _buffer = key;
            cstring::lower(_buffer.data());

            return _fields.at(_buffer);
        }

        /// @brief Inserts or overwrites a header field, case-insensitively.
        /// @details Lower-cases `key` into the shared scratch buffer before
        /// storing, so lookups with any casing of the same name resolve to
        /// this entry.
        /// @param key Field name to set (any case).
        /// @param value Field value to associate with `key`.
        /// @return Reference to this header, for call chaining.
        inline header& set_field(const string_view& key, const string_view& value)
        {
            // Because HTTP header fields are case-insensitive for some reason
            _buffer = key;
            cstring::lower(_buffer.data());

            _fields[_buffer] = value;
            return *this;
        }

        /// @brief Removes all stored header fields.
        /// @return Reference to this header, for call chaining.
        inline auto& clear()
        {
            _fields.clear();
            return *this;
        }

        /// @brief Returns an iterator to the first stored field.
        /// @return Const iterator over `(name, value)` pairs.
        inline auto begin() const
        {
            return _fields.begin();
        }

        /// @brief Returns the past-the-end iterator for the stored fields.
        /// @return Const iterator marking the end of the `(name, value)` pairs.
        inline auto end() const
        {
            return _fields.end();
        }

        /// @brief Parses CRLF-delimited `name: value` lines into #_fields.
        /// @details Reads consecutive lines of the form `name: value\r\n`,
        /// lower-casing each name before storing it, until it reaches a line
        /// that starts with `\r` or `\n` (the blank line terminating the
        /// header block). Leading whitespace after the `:` separator is
        /// skipped. Each line is required to end with `\r\n`; a bare `\n` is
        /// rejected.
        /// @param data Buffer positioned at the start of the header block.
        /// @return On success, the offset into `data` of the blank line that
        /// terminates the header block. On failure, a #header_error
        /// describing a missing `:` separator, a missing newline, or a line
        /// not terminated with CRLF.
        inline result<usize, header_error> parse(const string_view& data)
        {
            using namespace string_view_literals;
            usize _line_start = 0, _num_lines = 1;

            while (data[_line_start] != '\r' && data[_line_start] != '\n')
            {
                auto _colum_pos = data.find(':', _line_start);
                auto _value_pos = _colum_pos + 1;

                if (_colum_pos == data.npos)
                {
                    return header_error{
                        "Could not find filed separator at line"_sv, _num_lines
                    };
                }

                for (; cstring::iswhitespace(data[_value_pos]); ++_value_pos)
                    ;

                auto _end_pos = data.find('\n', _value_pos);

                if (_end_pos == data.npos)
                {
                    return header_error{
                        "Missing newline terminator at line"_sv, _num_lines
                    };
                }

                if (data[_end_pos - 1] == '\r')
                {
                    --_end_pos;
                }
                else
                {
                    return header_error{
                        "HTTP requires headers to end the line with a CRLF"_sv, _num_lines
                    };
                }

                string _key{
                    &data[_line_start], _colum_pos - _line_start + 1
                };
                cstring::lower(_key.data());

                _fields.emplace(
                    hsd::move(_key), string{
                        &data[_value_pos], _end_pos - _value_pos + 1
                    }
                );

                ++_num_lines;
                _line_start = _end_pos + 2;
            }

            return _line_start;
        }
    };
} // namespace hsd::http