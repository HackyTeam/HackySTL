#pragma once

#include "Io.hpp"
#include "Types.hpp"
#include "_Define.hpp"

#if defined(HSD_PLATFORM_POSIX)
#include <poll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
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
#include <winsock2.h>
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
            direct_server,
            multi_server,
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

        static inline void close_socket(net::native_socket_type& socket)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            ::closesocket(socket);
            #else
            ::close(socket);
            #endif

            socket = -1;
        }

        static inline const char* error_message()
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            static char _msg_buf[256];
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, WSAGetLastError(), 
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                _msg_buf, sizeof(_msg_buf), nullptr
            );

            return _msg_buf;
            #else
            return strerror(errno);
            #endif
        }

        template <net::socket_type>
        static inline bool use_socket(net::native_socket_type, addrinfo*) = delete;

        template <>
        inline bool use_socket<net::socket_type::direct_server>(
            net::native_socket_type sock, addrinfo* rp)
        {
            return bind(sock, rp->ai_addr, rp->ai_addrlen) != -1;
        }

        template <>
        inline bool use_socket<net::socket_type::multi_server>(
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
        struct new_socket_handler
        {
            static inline void invoke(auto&) {}
        };

        template <>
        struct new_socket_handler<net::socket_type::direct_server, net::socket_kind::tcp>
        {
            static inline void invoke(auto& new_socket)
            {
                auto _sock_copy = new_socket.get_socket();
                listen(_sock_copy, SOMAXCONN);
                new_socket.get_socket() = accept(
                    _sock_copy, new_socket.get_addr(), new_socket.get_addr_len()
                );
                
                close_socket(_sock_copy);
            }
        };

        template <>
        struct new_socket_handler<net::socket_type::multi_server, net::socket_kind::tcp>
        {
            static inline void invoke(auto& new_socket)
            {
                auto _listening = new_socket.get_socket();

                #if defined(HSD_PLATFORM_WINDOWS)
                static ulong _on = 1;
                auto _rc = setsockopt(
                    _listening, SOL_SOCKET, SO_REUSEADDR, 
                    reinterpret_cast<char*>(&_on), sizeof(_on)
                );
                #else
                static i32 _on = 1;
                auto _rc = setsockopt(
                    _listening, SOL_SOCKET, 
                    SO_REUSEADDR,
                    &_on, sizeof(_on)
                );
                #endif
                
                if (_rc < 0)
                {
                    perror("setsockopt() failed");
                    close_socket(_listening);
                    abort();
                }
            
                #if defined(HSD_PLATFORM_WINDOWS)
                _rc = ioctlsocket(_listening, FIONBIO, &_on);
                #else
                _rc = ioctl(_listening, FIONBIO, &_on);
                #endif

                if (_rc < 0)
                {
                    perror("ioctl() failed");
                    close_socket(_listening);
                    abort();
                }

                new_socket.get_server_socket() = _listening;
                listen(_listening, SOMAXCONN);
            }
        };
        
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
    } // namespace network_detail
} // namespace hsd