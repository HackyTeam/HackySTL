#pragma once

/// @file HttpResponse.hpp
/// @brief HTTP response status-line and header parser.
/// @details Extracts protocol version, numeric status, reason phrase, headers,
/// and consumed input length from CRLF-terminated responses.

#include "HttpBase.hpp"
#include "HttpHeader.hpp"

namespace hsd::http
{
    /// @brief Parsed HTTP status line and headers.
    /// @details Holds the protocol version, numeric status, reason phrase, and
    /// header block extracted by #parse, along with mutators used to build
    /// an outgoing response.
    class response
    {
    private:
        /// @brief Protocol version, defaults to HTTP/1.1.
        protocol_version _ver = protocol_version::ver_1_1;
        /// @brief Numeric response status code.
        response_status _status{};
        /// @brief Standard reason phrase for #_status, filled in by `set_status_message_spec`.
        string_view _status_message_spec = "Unknown";
        /// @brief Custom reason phrase overriding #_status_message_spec, if set.
        string _status_message{};
        /// @brief Parsed response header fields.
        header _header{};

    public:
        /// @brief Sets the response status code.
        /// @param status New numeric status code.
        /// @return Reference to this response, for call chaining.
        inline auto& set_status(response_status status)
        {
            _status = status;
            return *this;
        }

        /// @brief Returns the response status code.
        /// @return The stored #response_status.
        inline auto get_status() const
        {
            return _status;
        }

        /// @brief Returns the parsed response headers.
        /// @return Reference to the stored #header.
        inline const auto& get_header() const
        {
            return _header;
        }
        
        /// @brief Inserts or overwrites a header field, case-insensitively.
        /// @param key Field name to set (any case).
        /// @param value Field value to associate with `key`.
        /// @return Reference to this response, for call chaining.
        inline auto& set_header_field(
            const string_view& key, const string_view& value)
        {
            _header.set_field(key, value);
            return *this;
        }

        /// @brief Returns the response protocol version.
        /// @return The stored #protocol_version.
        inline auto get_version() const
        {
            return _ver;
        }

        /// @brief Sets the response protocol version.
        /// @param version New protocol version.
        /// @return Reference to this response, for call chaining.
        inline auto& set_version(protocol_version version)
        {
            _ver = version;
            return *this;
        }

        /// @brief Sets a custom reason phrase, overriding the standard one.
        /// @param msg New reason phrase (copied into this response).
        /// @return Reference to this response, for call chaining.
        inline auto& set_message(const string_view& msg)
        {
            _status_message = msg;
            return *this;
        }

        /// @brief Returns the reason phrase for this response.
        /// @details Returns the custom message set via #set_message if one was
        /// set (non-empty); otherwise falls back to the standard reason
        /// phrase for #_status.
        /// @return View of the reason phrase, valid as long as this response
        /// and any custom message set on it remain unchanged.
        inline auto get_message() const
        {
            if (_status_message.length() != 0)
            {
                return static_cast<string_view>(_status_message);
            }
            
            return _status_message_spec;
        }

        /// @brief Parses an HTTP status line and headers from raw wire data.
        /// @details Expects `HTTP/<version> <status> <reason phrase>\r\n`
        /// followed by the header block consumed via `header::parse`. The
        /// protocol token is validated with `check_protocol`, which panics
        /// (aborts) on an unexpected protocol version instead of returning an
        /// error since it unwraps its internal dispatch result. The status
        /// code's standard reason phrase is resolved into
        /// #_status_message_spec via `set_status_message_spec`.
        /// @param resp Buffer positioned at the start of the status line.
        /// @return On success, the total number of bytes consumed from `resp`
        /// (status line plus header block). On failure, a `runtime_error`
        /// describing a missing method separator, a missing status code, an
        /// unknown status code, a missing newline, or a malformed CRLF line
        /// ending; header-parsing errors are unwrapped and therefore also
        /// panic rather than propagating as this return type.
        inline auto parse(const string_view& resp)
            -> result<usize, runtime_error>
        {
            using namespace string_view_literals;
            usize _pos = resp.find(' ');
            usize _begin_pos = 0;

            if (_pos == resp.npos)
            {
                return runtime_error{
                    "Method separator not found"
                };
            }
            else
            {
                string_view _protocol = {
                    resp.data(), _pos
                };

                check_protocol(_protocol, _ver);

                ++_pos;
            }

            if (resp[_pos] < '0' && resp[_pos] > '9')
            {
                return runtime_error{
                    "HTTP responses are required to have a status code"
                };
            }
            else
            {
                _status = static_cast<response_status>(
                    cstring::parse<u16>(&resp[_pos])
                );

                auto _res = set_status_message_spec(
                    _status, _status_message_spec
                );

                if (_res.is_ok() == false)
                {
                    return _res.unwrap_err();
                }

                u16 _status_num = static_cast<u16>(_status);

                if (_status_num < 10)
                {
                    ++_pos;
                }
                else if (_status_num < 100)
                {
                    _pos += 2;
                }
                else
                {
                    _pos += 3;
                }

                if (resp[_pos] == ' ') ++_pos;

                _begin_pos = _pos;
                _pos = resp.find('\n', _pos);
            }

            if (_pos == resp.npos)
            {
                return runtime_error{
                    "Missing newline terminator"
                };
            }
            else
            {
                if (resp[_pos - 1] == '\r')
                {
                    --_pos;
                }
                else
                {
                    return runtime_error{
                        "HTTP requires to end the first line with a CRLF"
                    };
                }

                _status_message = string{
                    &resp[_begin_pos], _pos - _begin_pos + 1
                };
                _pos += 2;
            }

            return _pos + _header.parse({&resp[_pos], resp.length() - _pos}).unwrap();
        }
    };
} // namespace hsd::http