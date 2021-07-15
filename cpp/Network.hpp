#pragma once

#include "_NetworkDetail.hpp"
#include "Concepts.hpp"
#include "Allocator.hpp"

namespace hsd
{
    namespace network_core
    {
        template <net::socket_kind, net::socket_type>
        struct send;

        template <net::socket_kind, net::socket_type>
        struct receive;

        template <net::socket_type SockType>
        struct send<net::socket_kind::tcp, SockType>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<decltype(socket.get()), net::native_socket_type>)
            {
                return ::send(socket.get(), buffer, size, 0);
            }
        };

        template <>
        struct send<net::socket_kind::udp, net::socket_type::server>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<decltype(socket.get()), net::native_socket_type>)
            {
                return ::sendto(
                    socket.get(), buffer, size, 0, 
                    socket.get_addr(), *socket.get_addr_len()
                );
            }
        };

        template <>
        struct send<net::socket_kind::udp, net::socket_type::client>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<decltype(socket.get()), net::native_socket_type>)
            {
                return ::sendto(
                    socket.get(), buffer, size, 0, nullptr, 0
                );
            }
        };

        template <net::socket_type SockType>
        struct receive<net::socket_kind::tcp, SockType>
        {
            static isize invoke(auto& socket, auto* buffer, usize size)
            requires (IsSame<decltype(socket.get()), net::native_socket_type>)
            {
                return ::recv(socket.get(), buffer, size, 0);
            }
        };

        template <net::socket_type SockType>
        struct receive<net::socket_kind::udp, SockType>
        {
            static isize invoke(auto& socket, auto* buffer, usize size)
            requires (IsSame<decltype(socket.get()), net::native_socket_type>)
            {
                return ::recvfrom(
                    socket.get(), buffer, size, 0,
                    socket.get_addr(), socket.get_addr_len()
                );
            }
        };
    } // namespace network_core
    
    template <
        net::socket_kind SocketKind, 
        net::ip_protocol_type IpProtocol,
        net::socket_type SocketType, 
        net::socket_option SocketOption
    >
    class socket
    {
    private:
        using sock_type = network_core::socket<
            SocketKind, IpProtocol,
            SocketType, SocketOption
        >;

        sock_type _sock{};

        using buffer_type = hsd::static_sstream<4096>;
        static thread_local inline buffer_type _buffer_recv{};
        static thread_local inline buffer_type _buffer_send{};

    public:
        inline socket(const char* addr)
            : _sock{addr}
        {}

        inline socket(const char* addr, const char* port)
            : _sock{addr, port}
        {}

        const char* error_message() const
        {
            return strerror(errno);
        }

        template <hsd::basic_string_literal fmt, typename... Args>
        requires (IsSame<typename decltype(fmt)::char_type, char>)
        inline isize send(Args&&... args)
        {
            memset(_buffer_send.data(), 0, _buffer_send.capacity());
            _buffer_send.template write_data<fmt>(forward<Args>(args)...);
            
            return network_core::send<SocketKind, SocketType>::template invoke(
                _sock, _buffer_send.data(), _buffer_send.size()
            );
        }

        inline hsd::pair<isize, buffer_type&> receive()
        {
            memset(_buffer_recv.data(), 0, _buffer_recv.capacity());
            isize _size = network_core::receive<SocketKind, SocketType>::invoke(
                _sock, _buffer_recv.data(), _buffer_recv.capacity()
            );
            
            return {_size, _buffer_recv};
        }
    };

    using tcp_server_v4 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv4,
        net::socket_type::server,
        net::socket_option::stream
    >;

    using tcp_client_v4 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv4,
        net::socket_type::client,
        net::socket_option::stream
    >;

    using udp_client_v4 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv4,
        net::socket_type::client,
        net::socket_option::dgram
    >;

    using udp_server_v4 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv4,
        net::socket_type::server,
        net::socket_option::dgram
    >;

    using tcp_server_v6 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv6,
        net::socket_type::server,
        net::socket_option::stream
    >;

    using tcp_client_v6 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv6,
        net::socket_type::client,
        net::socket_option::stream
    >;

    using udp_client_v6 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv6,
        net::socket_type::client,
        net::socket_option::dgram
    >;

    using udp_server_v6 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv6,
        net::socket_type::server,
        net::socket_option::dgram
    >;
} // namespace hsd