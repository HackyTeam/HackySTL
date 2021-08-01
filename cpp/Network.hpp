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

        static inline auto error_code()
        {
            return network_detail::error_code();
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
        using underlying_socket = network_detail::native_socket_type;
        using socket_type = network_detail::tcp_socket<Protocol>;
        socket_type _socket;

    public:
        inline tcp_client(const char* const addr)
            : _socket{pollfd{static_cast<underlying_socket>(-1), POLLOUT, 0}}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            if (_socket.switch_to(addr, false) == false)
            {
                fputs("Error in initializing socket", stderr);
            }
        }

        static inline const char* error_message()
        {
            return network_detail::error_message();
        }

        static inline auto error_code()
        {
            return network_detail::error_code();
        }

        inline isize send(const char* const data, const usize size) const
        {
            return _socket.send(data, size);
        }

        inline isize receive(char* const data, const usize size) const
        {
            return _socket.receive(data, size);
        }
    };

    using tcp_server_v4 = tcp_server<network_detail::ip_protocol::ipv4>;
    using tcp_server_v6 = tcp_server<network_detail::ip_protocol::ipv6>;
    using tcp_client_v4 = tcp_client<network_detail::ip_protocol::ipv4>;
    using tcp_client_v6 = tcp_client<network_detail::ip_protocol::ipv6>;

    template <network_detail::ip_protocol Protocol>
    class udp_server
    {
    private:
        using underlying_socket = network_detail::native_socket_type;
        using socket_type = network_detail::udp_socket<Protocol>;
        vector<socket_type> _sockets;

    public:
        inline udp_server(const char* const addr)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _sockets.emplace_back(addr, true);
        }

        static inline const char* error_message()
        {
            return network_detail::error_message();
        }

        static inline auto error_code()
        {
            return network_detail::error_code();
        }

        inline void poll()
        {
            static char _dummy = '\0';
            static sockaddr_storage _addr{};
            static socklen_t _size = sizeof(_addr);

            auto _result = recvfrom(
                _sockets.front().fd(), &_dummy, 0, 0,
                reinterpret_cast<sockaddr*>(&_addr),
                &_size
            );
            
            if (_result < 0)
            {
                if (error_code() != WSAEWOULDBLOCK)
                {
                    fputs("recvfrom error", stderr);
                    abort();
                }
            }
            else
            {
                _sockets.emplace_back(
                    _sockets.front().fd(), _addr, _size
                );

                memset(&_addr, 0, sizeof(_addr));
            }
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
    class udp_client
    {
    private:
        using underlying_socket = network_detail::native_socket_type;
        using socket_type = network_detail::udp_socket<Protocol>;
        socket_type _socket;

    public:
        inline udp_client(const char* const addr)
            : _socket{static_cast<underlying_socket>(-1), sockaddr_storage{}, 0}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            if (_socket.switch_to(addr, false) == false)
            {
                fputs("Error in initializing socket", stderr);
            }

            // Send a dummy packet to make the
            // server know that we are connected.
            _socket.send("", 0);
        }

        static inline const char* error_message()
        {
            return network_detail::error_message();
        }

        static inline auto error_code()
        {
            return network_detail::error_code();
        }

        inline isize send(const char* const data, const usize size) const
        {
            return _socket.send(data, size);
        }

        inline isize receive(char* const data, const usize size) const
        {
            return _socket.receive(data, size);
        }
    };

    using udp_server_v4 = udp_server<network_detail::ip_protocol::ipv4>;
    using udp_server_v6 = udp_server<network_detail::ip_protocol::ipv6>;
    using udp_client_v4 = udp_client<network_detail::ip_protocol::ipv4>;
    using udp_client_v6 = udp_client<network_detail::ip_protocol::ipv6>;
} // namespace hsd