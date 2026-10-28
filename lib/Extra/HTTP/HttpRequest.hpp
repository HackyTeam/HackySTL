#pragma once

/// @file HttpRequest.hpp
/// @brief HTTP request-line and header parser.
/// @details Extracts method, path, protocol version, headers, and consumed
/// length from an HTTP request while reporting malformed syntax as errors.

#include "HttpBase.hpp"
#include "HttpHeader.hpp"

namespace hsd::http
{
    /// @brief Parsed HTTP request line and headers.
    /// @details Holds the method, target path, protocol version, and header
    /// block extracted by #parse, along with mutators used to build an
    /// outgoing request.
    class request
    {
    private:
        /// @brief Protocol version, defaults to HTTP/1.1.
        protocol_version _ver = protocol_version::ver_1_1;
        /// @brief Request method (verb).
        request_method _method{};
        /// @brief Request target path.
        string _path{};
        /// @brief Parsed request header fields.
        header _header{};

    public:
        /// @brief Returns the parsed request headers.
        /// @return Reference to the stored #header.
        inline const auto& get_header() const
        {
            return _header;
        }

        /// @brief Resets the path and headers to their empty state.
        /// @details Does not reset #_method or #_ver.
        /// @return Reference to this request, for call chaining.
        inline auto& clear()
        {
            _path.clear();
            _header.clear();
            return *this;
        }

        /// @brief Inserts or overwrites a header field, case-insensitively.
        /// @param key Field name to set (any case).
        /// @param value Field value to associate with `key`.
        /// @return Reference to this request, for call chaining.
        inline auto& set_header_field(
            const string_view& key, const string_view& value)
        {
            _header.set_field(key, value);
            return *this;
        }

        /// @brief Returns the request target path.
        /// @return Reference to the stored path.
        inline const auto& get_path() const
        {
            return _path;
        }

        /// @brief Sets the request target path.
        /// @param path New target path (copied into this request).
        /// @return Reference to this request, for call chaining.
        inline auto& set_path(const string_view& path)
        {
            _path = path;
            return *this;
        }

        /// @brief Returns the request method.
        /// @return The stored #request_method.
        inline auto get_method() const
        {
            return _method;
        }

        /// @brief Sets the request method.
        /// @param method New request method.
        /// @return Reference to this request, for call chaining.
        inline auto& set_method(request_method method)
        {
            _method = method;
            return *this;
        }

        /// @brief Returns the request protocol version.
        /// @return The stored #protocol_version.
        inline auto get_version() const
        {
            return _ver;
        }

        /// @brief Sets the request protocol version.
        /// @param version New protocol version.
        /// @return Reference to this request, for call chaining.
        inline auto& set_version(protocol_version version)
        {
            _ver = version;
            return *this;
        }

        /// @brief Parses an HTTP request line and headers from raw wire data.
        /// @details Expects `<METHOD> /<path> HTTP/<version>\r\n` followed by
        /// the header block consumed via `header::parse`. The method token is
        /// resolved with `check_request_method` and the protocol token is
        /// validated with `check_protocol`; either an unrecognized method or
        /// an unexpected protocol version panics (aborts) instead of
        /// returning an error, since both helpers unwrap their internal
        /// dispatch result.
        /// @param req Buffer positioned at the start of the request line.
        /// @return On success, the total number of bytes consumed from `req`
        /// (request line plus header block). On failure, a `runtime_error`
        /// describing a missing method separator, an absent/unterminated
        /// path, a missing protocol terminator, or a malformed CRLF line
        /// ending; header-parsing errors are unwrapped and therefore also
        /// panic rather than propagating as this return type.
        inline auto parse(const string_view& req)
            -> result<usize, runtime_error>
        {
            using namespace string_view_literals;
            usize _pos = req.find(' ');
            usize _begin_pos = 0;

            if (_pos == req.npos)
            {
                return runtime_error{
                    "Method separator not found"
                };
            }
            else
            {
                string_view _req_method = {
                    req.data(), _pos
                };

                check_request_method(_req_method, _method);
            }

            if (req[++_pos] != '/')
            {
                return runtime_error{
                    "Path for the requested resource is absent"
                };
            }
            else
            {
                _begin_pos = _pos;
                _pos = req.find(' ', _pos);
            }

            if (_pos == req.npos)
            {
                return runtime_error{
                    "Path separator not found"
                };
            }
            else
            {
                _path = string_view{
                    &req[_begin_pos], _pos - _begin_pos
                };

                _begin_pos = ++_pos;
                _pos = req.find('\n', _pos);
            }

            if (_pos == req.npos)
            {
                return runtime_error{
                    "Protocol terminator not found"
                };
            }
            else
            {
                if (req[_pos - 1] == '\r')
                {
                    --_pos;
                }
                else
                {
                    return runtime_error{
                        "HTTP requires to end the first line with a CRLF"
                    };
                }

                string_view _protocol = {
                    &req[_begin_pos], _pos - _begin_pos
                };

                check_protocol(_protocol, _ver);

                _pos += 2;
                return _pos + _header.parse({&req[_pos], req.length() - _pos}).unwrap();
            }
        } 
    };
} // namespace hsd::http