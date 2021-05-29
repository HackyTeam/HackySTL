#pragma once

#include "_NetworkDetail.hpp"
#include "Concepts.hpp"

namespace hsd
{
    namespace udp
    {
        namespace client_detail
        {
            using protocol_type = hsd::net::protocol_type;

            class socket
            {
            private:
                #if defined(HSD_PLATFORM_POSIX)
                i32 _listening = -1;
                #else
                SOCKET _listening = -1;
                #endif

                sockaddr_storage _peer_addr{};

            public:
                inline socket(protocol_type protocol = protocol_type::ipv4, 
                    const char* ip_addr = "127.0.0.1:54000")
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    network_detail::init_winsock();
                    #endif
                    
                    switch_to(protocol, ip_addr);
                }

                inline ~socket()
                {
                    close();
                }

                inline void close()
                {
                    #if defined(HSD_PLATFORM_POSIX)
                    ::close(_listening);
                    #else
                    ::closesocket(_listening);
                    #endif

                    _listening = -1;
                }

                inline i32 get_listening()
                {
                    return _listening;
                }

                inline sockaddr* get_hint()
                {
                    return bit_cast<sockaddr*>(&_peer_addr);
                }

                inline void switch_to(protocol_type protocol, const char* ip_addr)
                {
                    if (_listening != static_cast<decltype(_listening)>(-1))
                        close();

                    addrinfo* _result = nullptr;
                    addrinfo* _rp = nullptr;

                    auto _hints = addrinfo
                    {
                        .ai_flags = 0,
                        .ai_family = static_cast<i32>(protocol),
                        .ai_socktype = net::socket_type::dgram,
                        .ai_protocol = 0,
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

                    const char* _domain_port = 
                        cstring::find(ip_addr, ':');

                    if (_domain_port != nullptr)
                    {
                        char* _domain_addr = nullptr;
                        auto _domain_len = static_cast<usize>(
                            _domain_port - ip_addr
                        );

                        _domain_addr = new char[_domain_len + 1];
                        cstring::copy(_domain_addr, ip_addr, _domain_len);
                        _domain_addr[_domain_len] = static_cast<char>(0);
                        _domain_port += 1;

                        i32 _error_code = getaddrinfo(
                            _domain_addr, _domain_port, &_hints, &_result
                        );

                        if (_error_code != 0) 
                        {
                            fprintf(
                                stderr, "Error for getaddrinfo, code"
                                ": %s\n", gai_strerror(_error_code)
                            );
                            return;
                        }

                        delete[] _domain_addr;
                    }
                    else
                    {
                        i32 _error_code = getaddrinfo(
                            ip_addr, nullptr, &_hints, &_result
                        );
    
                        if (_error_code != 0) 
                        {
                            fprintf(
                                stderr, "Error for getaddrinfo, code"
                                ": %s\n", gai_strerror(_error_code)
                            );
                            return;
                        }
                    }

                    for (_rp = _result; _rp != nullptr; _rp = _rp->ai_next)
                    {
                        _listening = ::socket(
                            _rp->ai_family, _rp->ai_socktype, _rp->ai_protocol
                        );

                        if (_listening != static_cast<decltype(_listening)>(-1))
                        {
                            if (connect(_listening, _rp->ai_addr, _rp->ai_addrlen) != -1)
                                break;

                            close();
                        }
                    }

                    if (_rp == nullptr)
                        fprintf(stderr, "Could not bind\n");

                    freeaddrinfo(_result);
                }
            };
        } // namespace client_detail

        class client
        {
        private:
            client_detail::socket _sock;
            socklen_t _len = sizeof(sockaddr_storage);
            hsd::sstream _net_buf{4095};

            inline void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            inline client() = default;
            inline ~client() = default;

            inline client(net::protocol_type protocol, const char* ip_addr)
                : _sock{protocol, ip_addr}
            {}

            inline hsd::pair< hsd::sstream&, net::received_state > receive()
            {
                _clear_buf();
                isize _response = recvfrom(
                    _sock.get_listening(), _net_buf.data(), 
                    4096, 0, _sock.get_hint(), &_len
                );

                if (_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in receiving\n">();
                    _clear_buf();
                    return {_net_buf, net::received_state::err};
                }
                if (_response == static_cast<isize>(net::received_state::disconnected))
                {
                    hsd::io::err_print<"Server down\n">();
                    _clear_buf();
                    return {_net_buf, net::received_state::disconnected};
                }

                return {_net_buf, net::received_state::ok};
            }

            template < basic_string_literal fmt, typename... Args >
            requires (IsSame<char, typename decltype(fmt)::char_type>)
            inline net::received_state respond(Args&&... args)
            {
                _clear_buf();
                _net_buf.write_data<fmt>(forward<Args>(args)...);

                isize _response  = sendto(
                    _sock.get_listening(), _net_buf.data(), 
                    _net_buf.size(), 0, _sock.get_hint(), _len
                );
                    
                if(_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in sending\n">();
                    return net::received_state::err;
                }

                return net::received_state::ok;
            }
        };
    } // namespace udp

    namespace tcp
    {
        namespace client_detail
        {
            using protocol_type = net::protocol_type;

            class socket
            {
            private:
                #if defined(HSD_PLATFORM_POSIX)
                i32 _sock = -1;
                #else
                SOCKET _sock = -1;
                #endif

            public:
                inline socket(protocol_type protocol = protocol_type::ipv4, 
                    const char* ip_addr = "127.0.0.1:54000")
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    network_detail::init_winsock();
                    #endif
                    
                    switch_to(protocol, ip_addr);
                }

                inline ~socket()
                {
                    close();
                }

                inline void close()
                {
                    #if defined(HSD_PLATFORM_POSIX)
                    ::close(_sock);
                    #else
                    ::closesocket(_sock);
                    #endif

                    _sock = -1;
                }

                inline i32 get_sock()
                {
                    return _sock;
                }

                inline i32 switch_to(net::protocol_type protocol, const char* ip_addr)
                {
                    if (_sock != static_cast<decltype(_sock)>(-1))
                        close();

                    addrinfo* _result = nullptr;
                    addrinfo* _rp = nullptr;

                    auto _hints = addrinfo
                    {
                        .ai_flags = 0,
                        .ai_family = static_cast<i32>(protocol),
                        .ai_socktype = net::socket_type::stream,
                        .ai_protocol = 0,
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

                    const char* _domain_port = 
                        cstring::find(ip_addr, ':');

                    if (_domain_port != nullptr)
                    {
                        char* _domain_addr = nullptr;
                        auto _domain_len = static_cast<usize>(
                            _domain_port - ip_addr
                        );

                        _domain_addr = new char[_domain_len + 1];
                        cstring::copy(_domain_addr, ip_addr, _domain_len);
                        _domain_addr[_domain_len] = static_cast<char>(0);
                        _domain_port += 1;

                        i32 _error_code = getaddrinfo(
                            _domain_addr, _domain_port, &_hints, &_result
                        );

                        if (_error_code != 0) 
                        {
                            fprintf(
                                stderr, "Error for getaddrinfo, code"
                                ": %s\n", gai_strerror(_error_code)
                            );
                            return -1;
                        }

                        delete[] _domain_addr;
                    }
                    else
                    {
                        i32 _error_code = getaddrinfo(
                            ip_addr, nullptr, &_hints, &_result
                        );
    
                        if (_error_code != 0) 
                        {
                            fprintf(
                                stderr, "Error for getaddrinfo, code"
                                ": %s\n", gai_strerror(_error_code)
                            );
                            return -1;
                        }
                    }

                    for (_rp = _result; _rp != nullptr; _rp = _rp->ai_next)
                    {
                        _sock = ::socket(
                            _rp->ai_family, _rp->ai_socktype, _rp->ai_protocol
                        );

                        if (_sock != static_cast<decltype(_sock)>(-1))
                        {
                            if (connect(_sock, _rp->ai_addr, _rp->ai_addrlen) != -1)
                                break;

                            close();
                        }
                    }

                    if (_rp == nullptr)
                    {
                        fprintf(stderr, "Could not bind\n");
                        freeaddrinfo(_result);
                        return -1;
                    }

                    freeaddrinfo(_result);
                    return 0;
                }
            };
        } // namespace client_detail

        class client
        {
        private:
            client_detail::socket _sock;
            hsd::sstream _net_buf{4095};

            inline void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            inline client() = default;
            inline ~client() = default;

            inline client(net::protocol_type protocol, const char* ip_addr)
                : _sock{protocol, ip_addr}
            {}

            inline bool switch_to(net::protocol_type protocol, const char* ip_addr)
            {
                return _sock.switch_to(protocol, ip_addr) != -1;
            }

            inline hsd::pair< hsd::sstream&, net::received_state > receive()
            {
                _clear_buf();
                isize _response = recv(
                    _sock.get_sock(), _net_buf.data(), 4096, 0
                );

                if (_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in receiving\n">();
                    _clear_buf();
                    return {_net_buf, net::received_state::err};
                }
                if (_response == static_cast<isize>(net::received_state::disconnected))
                {
                    hsd::io::err_print<"Server down\n">();
                    _clear_buf();
                    return {_net_buf, net::received_state::disconnected};
                }

                return {_net_buf, net::received_state::ok};
            }

            template < basic_string_literal fmt, typename... Args >
            requires (IsSame<char, typename decltype(fmt)::char_type>)
            inline net::received_state respond(Args&&... args)
            {
                _clear_buf();
                _net_buf.write_data<fmt>(forward<Args>(args)...);

                isize _response = send(_sock.get_sock(), 
                    _net_buf.data(), _net_buf.size(), 0);

                if(_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in sending\n">();
                    return net::received_state::err;
                }

                return net::received_state::ok;
            }
        };
    } // namespace tcp
} // namespace hsd