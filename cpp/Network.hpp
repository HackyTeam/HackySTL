#pragma once

#include "_SocketDetail.hpp"
#include "Concepts.hpp"
#include "Vector.hpp"

namespace hsd
{
    template <network_detail::ip_protocol Protocol>
    class tcp_server
    {
    private:
        using socket_type = network_detail::tcp_socket<Protocol>;
        vector<socket_type> _sockets;

    public:
        inline tcp_server(const char* const addr)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _sockets.emplace_back(addr, true);
        }
        
        inline void poll()
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            auto _result = ::WSAPoll(
                reinterpret_cast<pollfd*>(
                    _sockets.data()
                ), _sockets.size(), -1
            );
            #else
            auto _result = ::poll(
                reinterpret_cast<pollfd*>(
                    _sockets.data()
                ), _sockets.size(), -1
            );
            #endif

            if (_result < 0)
            {
                fputs("poll error", stderr);
                abort();
            }

            if (_sockets.front().is_readable())
            {
                auto _new_socket = accept(
                    _sockets.front().fd(), nullptr, nullptr
                );
                _sockets.emplace_back(
                    pollfd{_new_socket, POLLIN | POLLOUT, 0}
                );
            }
        }

        static inline const char* error_message()
        {
            return network_detail::error_message();
        }

        inline auto begin() const
        {
            // Always skip the first element.
            return _sockets.begin() + 1;
        }

        inline auto end() const
        {
            return _sockets.end();
        }
    };

    template <network_detail::ip_protocol Protocol>
    class tcp_client
    {
    private:
        using socket_type = network_detail::tcp_socket<Protocol>;
        socket_type _socket;

    public:
        inline tcp_client(const char* const addr)
            : _socket{pollfd{-1, POLLOUT, 0}}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            if (_socket.switch_to(addr, false) == false)
            {
                fputs("Error in initializing socket", stderr);
            }
        }

        inline isize send(const char* const data, const usize size) const
        {
            return _socket.send(data, size);
        }

        inline isize receive(char* const data, const usize size) const
        {
            return _socket.receive(data, size);
        }

        static inline const char* error_message()
        {
            return network_detail::error_message();
        }
    };

    using tcp_server_v4 = tcp_server<network_detail::ip_protocol::ipv4>;
    using tcp_server_v6 = tcp_server<network_detail::ip_protocol::ipv6>;
    using tcp_client_v4 = tcp_client<network_detail::ip_protocol::ipv4>;
    using tcp_client_v6 = tcp_client<network_detail::ip_protocol::ipv6>;
} // namespace hsd