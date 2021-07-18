#pragma once

#include "_SocketDetail.hpp"
#include "Concepts.hpp"
#include "Allocator.hpp"

namespace hsd
{
    namespace network_detail
    {
        template <net::socket_kind, net::socket_type>
        struct send;

        template <net::socket_kind, net::socket_type>
        struct receive;

        template <net::socket_type SockType>
        struct send<net::socket_kind::tcp, SockType>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<
                remove_cvref_t<
                    decltype(socket.get_socket())
                >, net::native_socket_type
            >)
            {
                return ::send(socket.get_socket(), buffer, size, 0);
            }
        };

        template <>
        struct send<net::socket_kind::udp, net::socket_type::direct_server>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<
                remove_cvref_t<
                    decltype(socket.get_socket())
                >, net::native_socket_type
            >)
            {
                return ::sendto(
                    socket.get_socket(), buffer, size, 0, 
                    socket.get_addr(), *socket.get_addr_len()
                );
            }
        };

        template <>
        struct send<net::socket_kind::udp, net::socket_type::client>
        {
            static isize invoke(auto& socket, const auto* buffer, usize size)
            requires (IsSame<
                remove_cvref_t<
                    decltype(socket.get_socket())
                >, net::native_socket_type
            >)
            {
                return ::sendto(
                    socket.get_socket(), buffer, size, 0, nullptr, 0
                );
            }
        };

        template <net::socket_type SockType>
        struct receive<net::socket_kind::tcp, SockType>
        {
            static isize invoke(auto& socket, auto* buffer, usize size)
            requires (IsSame<
                remove_cvref_t<
                    decltype(socket.get_socket())
                >, net::native_socket_type
            >)
            {
                return ::recv(socket.get_socket(), buffer, size, 0);
            }
        };

        template <net::socket_type SockType>
        struct receive<net::socket_kind::udp, SockType>
        {
            static isize invoke(auto& socket, auto* buffer, usize size)
            requires (IsSame<
                remove_cvref_t<
                    decltype(socket.get_socket())
                >, net::native_socket_type
            >)
            {
                return ::recvfrom(
                    socket.get_socket(), buffer, size, 0,
                    socket.get_addr(), socket.get_addr_len()
                );
            }
        };
    } // namespace network_detail

    template <
        net::socket_kind SocketKind, 
        net::ip_protocol_type IpProtocol,
        net::socket_type SocketType, 
        net::socket_option SocketOption
    >
    class socket
        : private socket_detail::socket_base<SocketType>
    {
    private:
        using base_type = socket_detail::socket_base<SocketType>;
        using exposable_type = socket_detail::socket_exposable<SocketType>;
        using buffer_type = hsd::static_sstream<4096>;

        static inline buffer_type _buffer_recv{};
        static inline buffer_type _buffer_send{};
        static inline bool _has_recv{};
        static inline bool _has_send{};

        inline void switch_to(const char* ip_addr, const char* port = nullptr)
        {
            this->close();
            this->get_socket() = network_detail::switch_to<
                SocketKind, IpProtocol, 
                SocketType, SocketOption
            >(ip_addr, port);

            network_detail::new_socket_handler<SocketType, SocketKind>::invoke(
                static_cast<base_type&>(*this)
            );
        }

    public:
        inline socket(
            const char* ip_addr = "0.0.0.0:54000", const char* port = nullptr)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            switch_to(ip_addr, port);
        }

        inline socket(const socket&) = delete;
        inline socket& operator=(const socket&) = delete;

        inline socket(socket&& other)
        {
            this->get_socket() = exchange(
                other.get_socket(), this->get_invalid_socket()
            );
        }

        inline socket& operator=(socket&& rhs)
        {
            this->get_socket() = exchange(
                rhs.get_socket(), this->get_invalid_socket()
            );
            
            return *this;
        }

        inline const char* error_message() const
        {
            return network_detail::error_message();
        }

        inline auto& get()
        {
            return static_cast<exposable_type&>(*this);
        }

        template <hsd::basic_string_literal fmt, typename... Args>
        requires (IsSame<typename decltype(fmt)::char_type, char>)
        inline bool send(Args&&... args)
        {
            _has_recv = false;
            memset(_buffer_send.data(), 0, _buffer_send.capacity());
            _buffer_send.template write_data<fmt>(forward<Args>(args)...);
            
            auto _length = network_detail::send<
                SocketKind, SocketType>::template invoke(
                    static_cast<base_type&>(*this), 
                    _buffer_send.data(), _buffer_send.size()
                );

            if (_length <= 0)
            {
                return false;
            }
            else
            {
                _has_send = true;
                return true;
            }
        }

        inline hsd::pair<bool, buffer_type&> receive()
        {
            _has_send = false;
            memset(_buffer_recv.data(), 0, _buffer_recv.capacity());
            
            isize _size = network_detail::receive<SocketKind, SocketType>::invoke(
                static_cast<base_type&>(*this),
                _buffer_recv.data(), _buffer_recv.capacity()
            );

            if (_size <= 0)
            {
                return {false, _buffer_recv};
            }
            else
            {
                _has_recv = true;
                return {true, _buffer_recv};
            }
        }
    };

    using tcp_dserver_v4 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv4,
        net::socket_type::direct_server,
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

    using udp_dserver_v4 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv4,
        net::socket_type::direct_server,
        net::socket_option::dgram
    >;

    using tcp_dserver_v6 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv6,
        net::socket_type::direct_server,
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

    using udp_dserver_v6 = socket<
        net::socket_kind::udp,
        net::ip_protocol_type::ipv6,
        net::socket_type::direct_server,
        net::socket_option::dgram
    >;

    using tcp_mserver_v4 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv4,
        net::socket_type::multi_server,
        net::socket_option::stream
    >;
    
    using tcp_mserver_v6 = socket<
        net::socket_kind::tcp,
        net::ip_protocol_type::ipv6,
        net::socket_type::multi_server,
        net::socket_option::stream
    >;
} // namespace hsd