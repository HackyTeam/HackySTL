#pragma once

#include "Io.hpp"
#include "Types.hpp"
#include "_Define.hpp"

#if defined(HSD_PLATFORM_POSIX)
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#else
#include <unistd.h>
#include <ws2tcpip.h>
#endif

namespace hsd
{
    namespace net
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        using native_socket_type = SOCKET;
        #else
        using native_socket_type = i32;
        #endif

        enum class received_state
        {
            error = -1,
            disconnected,
            ok
        };

        enum class socket_type
        {
            server,
            client
        };

        enum class socket_kind
        {
            tcp = IPPROTO_TCP,
            udp = IPPROTO_UDP
        };

        enum class ip_protocol_type
        {
            ipv4 = AF_INET,
            ipv6 = AF_INET6,
            unspec = AF_UNSPEC
        };

        #if defined(HSD_PLATFORM_POSIX)
        enum class socket_option
        {
            stream = SOCK_STREAM,
            clo_exec = SOCK_CLOEXEC,
            dccp = SOCK_DCCP,
            dgram = SOCK_DGRAM,
            no_block = SOCK_NONBLOCK,
            packet = SOCK_PACKET,
            raw = SOCK_RAW,
            rdm = SOCK_RDM,
            seq_packet = SOCK_SEQPACKET
        };
        #else
        enum class socket_option
        {
            stream = SOCK_STREAM,
            dgram = SOCK_DGRAM,
            raw = SOCK_RAW,
            rdm = SOCK_RDM,
            seq_packet = SOCK_SEQPACKET
        };
        #endif
    } // namespace net

    namespace network_detail
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        static inline void init_winsock()
        {
            static bool _is_init = false;
            
            if (_is_init == false)
            {
                WSAData _data;
	            WORD _ver = MAKEWORD(2, 2);
	            i32 _ws_result = WSAStartup(_ver, &_data);
    
                if (_ws_result != 0)
	            {
	            	hsd::io::err_print<
                        "Can't start Winsock, here's"
                        " a irrelevant number #{}\n"
                    >(_ws_result);
                    return;
	            }

                _is_init = true;
            }
        }
        #endif
    } // namespace network_detail

    namespace network_core
    {
        static inline void close_socket(net::native_socket_type& socket)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            ::closesocket(socket);
            #else
            ::close(socket);
            #endif

            socket = -1;
        }

        template <net::socket_type>
        static inline bool use_socket(net::native_socket_type, addrinfo*) = delete;

        template <>
        inline bool use_socket<net::socket_type::server>(
            net::native_socket_type sock, addrinfo* rp)
        {
            return bind(sock, rp->ai_addr, rp->ai_addrlen) != -1;
        }

        template <>
        inline bool use_socket<net::socket_type::client>(
            net::native_socket_type sock, addrinfo* rp)
        {
            return connect(sock, rp->ai_addr, rp->ai_addrlen) != -1;
        }

        template <net::socket_type, net::socket_kind>
        static inline void handle_new_socket(
            net::native_socket_type&, sockaddr*, socklen_t*) {}

        template <>
        inline void handle_new_socket<
            net::socket_type::server, net::socket_kind::tcp
        >(
            net::native_socket_type& new_socket, 
            sockaddr* sock_hint, socklen_t* hint_size)
        {
            auto _sock_copy = new_socket;
            listen(_sock_copy, SOMAXCONN);
            new_socket = accept(_sock_copy, sock_hint, hint_size);
            close_socket(_sock_copy);
        }
        
        bool get_addr_info(
            const char* name, const char* service,
            const addrinfo* hints, addrinfo** result)
        {
            i32 _error_code = getaddrinfo(
                name, service, hints, result
            );

            if (_error_code != 0) 
            {
                fprintf(
                    stderr, "Error for getaddrinfo, code"
                    ": %s\n", gai_strerror(_error_code)
                );

                return false;
            }

            return true;
        }

        template <
            net::socket_kind SocketKind, 
            net::ip_protocol_type IpProtocol,
            net::socket_type SocketType, 
            net::socket_option SocketOption
        >
        static inline auto switch_to(const char* ip_addr, const char* port = nullptr)
        {
            net::native_socket_type _new_sock = -1;
            addrinfo* _result = nullptr;
            addrinfo* _rp = nullptr;

            auto _hints = addrinfo
            {
                .ai_flags = AI_PASSIVE,
                .ai_family = static_cast<i32>(IpProtocol),
                .ai_socktype = static_cast<i32>(SocketOption),
                .ai_protocol = static_cast<i32>(SocketKind),
                .ai_addrlen = 0,
                #if defined(HSD_PLATFORM_POSIX)
                .ai_addr = nullptr,
                .ai_canonname = nullptr,
                #else
                .ai_canonname = nullptr,
                .ai_addr = nullptr,
                #endif
                .ai_next = nullptr
            };

            if (port == nullptr)
            {
                port = cstring::find(ip_addr, ':');

                if (port != nullptr)
                {
                    char* _domain_addr = nullptr;
                    auto _domain_len = static_cast<usize>(
                        port - ip_addr
                    );

                    _domain_addr = mallocator::allocate_multiple<
                        char>(_domain_len + 1).unwrap();

                    cstring::copy(_domain_addr, ip_addr, _domain_len);
                    _domain_addr[_domain_len] = static_cast<char>(0);
                    port += 1;

                    if (get_addr_info(_domain_addr, port, &_hints, &_result) == false)
                    {
                        mallocator::deallocate(_domain_addr);
                        return _new_sock;
                    }

                    mallocator::deallocate(_domain_addr);
                }
                else
                {
                    if (get_addr_info(ip_addr, nullptr, &_hints, &_result) == false)
                    {
                        return _new_sock;
                    }
                }
            }
            else
            {
                if (get_addr_info(ip_addr, port, &_hints, &_result) == false)
                {
                    return _new_sock;
                }
            }
            for (_rp = _result; _rp != nullptr; _rp = _rp->ai_next)
            {
                _new_sock = ::socket(
                    _rp->ai_family, _rp->ai_socktype, _rp->ai_protocol
                );

                if (_new_sock != static_cast<decltype(_new_sock)>(-1))
                {
                    if (use_socket<SocketType>(_new_sock, _rp))
                        break;

                    close_socket(_new_sock);
                }
            }

            if (_rp == nullptr)
                fprintf(stderr, "Could not bind\n");

            freeaddrinfo(_result);
            return _new_sock;
        }

        template <
            net::socket_kind SocketKind, 
            net::ip_protocol_type IpProtocol,
            net::socket_type SocketType, 
            net::socket_option SocketOption
        >
        class socket
        {
        private:
            net::native_socket_type _socket_value = -1;
            sockaddr_storage _local_addr{};
            socklen_t _local_addr_len = sizeof(sockaddr_storage);

        public:
            static constexpr net::native_socket_type invalid_socket = -1;

            inline socket(const char* ip_addr = "0.0.0.0:54000", const char* port = nullptr)
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
                _socket_value = exchange(other._socket_value, invalid_socket);
            }

            inline socket& operator=(socket&& rhs)
            {
                _socket_value = exchange(rhs._socket_value, invalid_socket);
                return *this;
            }

            inline ~socket()
            {
                close();
            }

            inline bool is_valid() const
            {
                return _socket_value != invalid_socket;
            }

            inline bool is_invalid() const
            {
                return _socket_value == invalid_socket;
            }

            inline void close()
            {
                if (is_valid())
                    close_socket(_socket_value);
            }

            inline auto* get_addr()
            {
                return reinterpret_cast<sockaddr*>(&_local_addr);
            }

            inline auto* get_addr_len()
            {
                return &_local_addr_len;
            }

            inline auto get() const
            {
                return _socket_value;
            }

            inline void switch_to(const char* ip_addr, const char* port = nullptr)
            {
                close();
                _socket_value = network_core::switch_to<
                    SocketKind, IpProtocol, 
                    SocketType, SocketOption
                >(ip_addr, port);

                handle_new_socket<SocketType, SocketKind>(
                    _socket_value, get_addr(), get_addr_len()
                );
            }
        };
    } // network_core
} // namespace hsd