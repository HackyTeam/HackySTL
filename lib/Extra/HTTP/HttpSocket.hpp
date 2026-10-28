#pragma once

/// @file HttpSocket.hpp
/// @brief Buffered HTTP request/response operations over network sockets.
/// @details Coordinates parsed HTTP messages with a connected TCP socket and
/// owns request/response buffers without owning the underlying server.

#include "../../Serialize/SStream.hpp"

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

namespace hsd::http
{
    template <typename SockType>
    /// @brief HTTP request/response state machine over a socket type.
    /// @details Owns the request/response objects and the read/write buffer,
    /// but does not own the underlying socket: #set_current_sock only stores
    /// a non-owning pointer to a caller-managed `SockType`, so the socket's
    /// lifetime and connection state must be managed by the caller.
    /// @tparam SockType Connected socket type exposing `send(data, size)` and
    /// `receive(data, capacity)` member functions (e.g. a TCP socket).
    class socket
    {
    private:
        /// @brief Read/write buffer shared by all send/receive operations.
        sstream _stream;
        /// @brief Non-owning pointer to the currently attached socket; null until #set_current_sock is called.
        SockType* _socket = nullptr;
        /// @brief Request state used by #send_request and #receive_request.
        request _req;
        /// @brief Response state used by #send_response and #receive_response.
        response _resp;

        /// @brief Maps a protocol version to its wire token.
        /// @param ver Protocol version to render.
        /// @return String view of the literal `HTTP/x.y` token; panics
        /// (aborts) if `ver` does not match a known #protocol_version.
        static inline auto get_version_string(protocol_version ver)
        {
            using namespace string_view_literals;

            switch (ver)
            {
                case protocol_version::ver_0_9:
                {
                    return "HTTP/0.9"_sv;
                }
                case protocol_version::ver_1_0:
                {
                    return "HTTP/1.0"_sv;
                }
                case protocol_version::ver_1_1:
                {
                    return "HTTP/1.1"_sv;
                }
                case protocol_version::ver_2_0:
                {
                    return "HTTP/2.0"_sv;
                }
                case protocol_version::ver_3_0:
                {
                    return "HTTP/3.0"_sv;
                }
                default:
                {
                    panic("Unknown Version");
                }
            }
        }

        /// @brief Maps a request method to its wire token.
        /// @details Unlike #get_version_string, this switch has no default
        /// branch; every #request_method enumerator is handled explicitly.
        /// @param method Request method to render.
        /// @return String view of the literal method token (e.g. "GET").
        static inline auto get_request_string(request_method method)
        {
            using namespace string_view_literals;

            switch (method)
            {
                case request_method::get:
                {
                    return "GET"_sv;
                }
                case request_method::head:
                {
                    return "HEAD"_sv;
                }
                case request_method::post:
                {
                    return "POST"_sv;
                }
                case request_method::put:
                {
                    return "PUT"_sv;
                }
                case request_method::del:
                {
                    return "DELETE"_sv;
                }
                case request_method::connect:
                {
                    return "CONNECT"_sv;
                }
                case request_method::options:
                {
                    return "OPTIONS"_sv;
                }
                case request_method::trace:
                {
                    return "TRACE"_sv;
                }
                case request_method::patch:
                {
                    return "PATCH"_sv;
                }
            }
        }

    public:
        /// @brief Reserves capacity in the internal read/write buffer.
        /// @param new_cap Minimum buffer capacity to reserve, in bytes.
        /// @return Reference to this socket, for call chaining.
        inline auto& reserve_buffer(usize new_cap)
        {
            _stream.reserve(new_cap);
            return *this;
        }

        /// @brief Sets the protocol version used for both the request and the response.
        /// @param ver New protocol version.
        /// @return Reference to this socket, for call chaining.
        inline auto& set_version(protocol_version ver)
        {
            _req.set_version(ver);
            _resp.set_version(ver);
            return *this;
        }

        /// @brief Attaches the socket used by subsequent send/receive calls.
        /// @details Stores a non-owning pointer; `sock` must outlive its use
        /// through this object, and this object does not close or otherwise
        /// manage `sock`'s lifetime.
        /// @param sock Connected socket to send/receive through.
        /// @return Reference to this socket, for call chaining.
        inline auto& set_current_sock(SockType& sock)
        {
            _socket = &sock;
            return *this;
        }

        /// @brief Returns the request state.
        /// @return Reference to the stored #request.
        inline auto& get_request()
        {
            return _req;
        }

        /// @brief Returns the response state.
        /// @return Reference to the stored #response.
        inline auto& get_response()
        {
            return _resp;
        }

        /// @brief Serializes the current response and sends it, followed by a body.
        /// @details Writes the status line, all response headers, a blank
        /// line, then `data` verbatim, into the internal buffer before
        /// handing it to the attached socket's `send`. The buffer is not
        /// cleared afterward.
        /// @param data Response body to append after the headers.
        /// @return An empty `option_err` on success, or a `runtime_error` if
        /// no socket has been attached via #set_current_sock.
        inline auto send_response(const string_view& data) -> option_err<runtime_error>
        {
            using namespace hsd::format_literals;

            if (_socket == nullptr) [[unlikely]]
            {
                return runtime_error {
                    "No socket to refer to"
                };
            }

            _stream.write_data(
                "{} {} {}\r\n"_fmt,
                get_version_string(_resp.get_version()), 
                static_cast<u16>(_resp.get_status()),
                _resp.get_message()
            );

            for (const auto& [_field, _value] : _resp.get_header())
            {
                _stream.append_data("{}: {}\r\n"_fmt, _field, _value);
            }

            _stream.append_data("\r\n{}"_fmt, data);
            _socket->send(_stream.data(), _stream.size());
        
            return {};
        }

        /// @brief Serializes the current request and sends it.
        /// @details Writes the request line and all request headers followed
        /// by a blank line into the internal buffer, hands it to the attached
        /// socket's `send`, then clears the buffer.
        /// @return An empty `option_err` on success, or a `runtime_error` if
        /// no socket has been attached via #set_current_sock.
        inline auto send_request() -> option_err<runtime_error>
        {
            using namespace hsd::format_literals;

            if (_socket == nullptr) [[unlikely]]
            {
                return runtime_error {
                    "No socket to refer to"
                };
            }

            _stream.write_data(
                "{} {} {}\r\n"_fmt,
                get_request_string(_req.get_method()),
                _req.get_path(),
                get_version_string(_req.get_version()) 
            );

            for (const auto& [_field, _value] : _req.get_header())
            {
                _stream.append_data("{}: {}\r\n"_fmt, _field, _value);
            }

            _stream.append_data("\r\n"_fmt);
            _socket->send(_stream.data(), _stream.size());
            _stream.clear();
        
            return {};
        }

        /// @brief Reads and parses one request from the attached socket.
        /// @details Clears the internal buffer, fills it via the attached
        /// socket's `receive`, then parses it into #get_request. If nothing
        /// was received, returns an empty view rather than an error.
        /// @return On success, a view over any bytes remaining after the
        /// parsed request line and headers (e.g. a request body), backed by
        /// the internal buffer. On failure, a `runtime_error` from a missing
        /// socket, the underlying `receive` call, or request parsing.
        inline auto receive_request() -> result<string_view, runtime_error>
        {
            if (_socket == nullptr) [[unlikely]]
            {
                return runtime_error {
                    "No socket to refer to"
                };
            }

            _stream.clear();

            auto _req_res = _stream.buffer_write(
                [this](char* data, usize capacity) {
                    return _socket->receive(data, capacity);
                }
            );

            if (!_req_res.is_ok())
            {
                return _req_res.unwrap_err();
            }

            if (_stream.size() == 0)
            {
                // Not undefined, let the user
                using namespace hsd::string_view_literals;
                
                return ""_sv;
            }

            auto _parse_res = _req.parse({_stream.data(), _stream.size()});
        
            if (!_parse_res.is_ok())
            {
                return _parse_res.unwrap_err();
            }

            auto _pos = _parse_res.unwrap();

            return string_view {
                _stream.data() + _pos, _stream.size() - _pos
            };
        }
        
        /// @brief Reads and parses one response from the attached socket.
        /// @details Fills the internal buffer via the attached socket's
        /// `receive`, without clearing it first, then parses it into
        /// #get_response.
        /// @return On success, a view over any bytes remaining after the
        /// parsed status line and headers (e.g. a response body), backed by
        /// the internal buffer. On failure, a `runtime_error` from a missing
        /// socket, the underlying `receive` call, or response parsing.
        inline auto receive_response() -> result<string_view, runtime_error>
        {
            if (_socket == nullptr) [[unlikely]]
            {
                return runtime_error {
                    "No socket to refer to"
                };
            }

            auto _req_res = _stream.buffer_write(
                [this](char* data, usize capacity) {
                    return _socket->receive(data, capacity);
                }
            );

            if (!_req_res.is_ok())
            {
                return _req_res.unwrap_err();
            }

            auto _parse_res = _resp.parse({_stream.data(), _stream.size()});
        
            if (!_parse_res.is_ok())
            {
                return _parse_res.unwrap_err();
            }

            auto _pos = _parse_res.unwrap();

            return string_view {
                _stream.data() + _pos, _stream.size() - _pos
            };
        }
    };
} // namespace hsd::http