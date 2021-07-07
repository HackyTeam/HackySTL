#pragma once

#include "_NetworkDetail.hpp"
#include "Concepts.hpp"
#include "Allocator.hpp"

namespace hsd
{
    namespace udp
    {
        namespace server_detail
        {
            using protocol_type = net::protocol_type;

            class socket
            {
            private:
                #if defined(HSD_PLATFORM_WINDOWS)
                SOCKET _listening = -1;
                #else
                i32 _listening = -1;
                #endif

                static inline sockaddr_storage _peer_addr{};

            public:
                #if defined(HSD_PLATFORM_WINDOWS)
                static constexpr SOCKET invalid_socket = -1;
                #else
                static constexpr i32 invalid_socket = -1;
                #endif

                inline socket(protocol_type protocol = protocol_type::ipv4, 
                    const char* ip_addr = "0.0.0.0:54000")
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    network_detail::init_winsock();
                    #endif

                    switch_to(protocol, ip_addr);
                }

                inline socket(const socket&) = delete;
                inline socket& operator=(const socket&) = delete;

                inline socket(socket&& other)
                {
                    _listening = exchange(other._listening, invalid_socket);
                }

                inline socket& operator=(socket&& rhs)
                {
                    _listening = exchange(rhs._listening, invalid_socket);
                    return *this;
                }

                inline ~socket()
                {
                    close();
                }

                inline void close()
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    ::closesocket(_listening);
                    #else
                    ::close(_listening);
                    #endif

                    _listening = -1;
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                inline SOCKET get_listening()
                #else
                inline i32 get_listening()
                #endif
                {
                    return _listening;
                }

                static inline sockaddr* get_hint()
                {
                    return reinterpret_cast<sockaddr*>(&_peer_addr);
                }

                inline void switch_to(net::protocol_type protocol, const char* ip_addr)
                {
                    if (_listening != invalid_socket)
                        close();

                    addrinfo* _result = nullptr;
                    addrinfo* _rp = nullptr;

                    auto _hints = addrinfo
                    {
                        .ai_flags = AI_PASSIVE,
                        .ai_family = static_cast<i32>(protocol),
                        .ai_socktype = net::socket_type::dgram,
                        .ai_protocol = IPPROTO_UDP,
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

                        _domain_addr = mallocator::allocate_multiple<
                            char>(_domain_len + 1).unwrap();

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

                        mallocator::deallocate(_domain_addr);
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

                        if (_listening != invalid_socket)
                        {
                            if (bind(_listening, _rp->ai_addr, _rp->ai_addrlen) == 0)
                                break;

                            close();
                        }
                    }

                    if (_rp == nullptr)
                        fprintf(stderr, "Could not bind\n");

                    freeaddrinfo(_result);
                }
            };

            class multi_smart_sock
            {
            private:
                socket _sock;

                static inline hsd::sstream _net_buf{4095};
                static inline socklen_t _len = 
                    sizeof(sockaddr_storage);

                static inline void _clear_buf()
                {
                    memset(_net_buf.data(), '\0', 4096);
                }

            public:
                inline multi_smart_sock() = default;

                inline multi_smart_sock(
                    net::protocol_type protocol, const char* ip_addr)
                    : _sock{protocol, ip_addr}
                {}

                inline multi_smart_sock(multi_smart_sock&& other)
                    : _sock{move(other._sock)}
                {}

                inline multi_smart_sock& operator=(multi_smart_sock&& rhs)
                {
                    _sock = move(rhs._sock);
                    return *this;
                }

                inline void switch_to(
                    net::protocol_type protocol, const char* ip_addr)
                {
                    _sock.switch_to(protocol, ip_addr);
                }

                inline void close_socket()
                {
                    if (_sock.get_listening() != socket::invalid_socket)
                        respond<"">();
                }

                inline ~multi_smart_sock()
                {
                    close_socket();
                }

                inline auto receive()
                    -> hsd::pair< hsd::sstream&, net::received_state >
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
                        hsd::io::err_print<"Client disconnected\n">();
                        _clear_buf();
                        return {_net_buf, net::received_state::disconnected};
                    }

                    return {_net_buf, net::received_state::ok};
                }

                template< basic_string_literal fmt, typename... Args >
                requires (IsSame<char, typename decltype(fmt)::char_type>)
                inline net::received_state respond(Args&&... args)
                {
                    _clear_buf();
                    _net_buf.write_data<fmt>(forward<Args>(args)...);

                    isize _response = sendto(
                        _sock.get_listening(), _net_buf.data(), 
                        _net_buf.size(), 0, _sock.get_hint(), _len
                    );

                    if (_response == static_cast<isize>(net::received_state::err))
                    {
                        hsd::io::err_print<"Error in sending\n">();
                        return net::received_state::err;
                    }

                    return net::received_state::ok;
                }
            };
        } // namespace server_detail

        class server
        {
        private:
            server_detail::socket _sock;

            socklen_t _len = sizeof(sockaddr_storage);
            hsd::sstream _net_buf{4095};

            inline void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            inline server() = default;

            inline server(net::protocol_type protocol, const char* ip_addr)
                : _sock{protocol, ip_addr}
            {}

            inline server(server&& other)
                : _sock{move(other._sock)}
            {}

            inline server& operator=(server&& rhs)
            {
                _sock = move(rhs._sock);
                return *this;
            }

            inline ~server()
            {
                respond<"">();
            }

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
                    hsd::io::err_print<"Client disconnected\n">();
                    _clear_buf();
                    return {_net_buf, net::received_state::disconnected};
                }

                return {_net_buf, net::received_state::ok};
            }

            template< basic_string_literal fmt, typename... Args >
            requires (IsSame<char, typename decltype(fmt)::char_type>)
            inline net::received_state respond(Args&&... args)
            {
                _clear_buf();
                _net_buf.write_data<fmt>(forward<Args>(args)...);

                isize _response = sendto(
                    _sock.get_listening(), _net_buf.data(), 
                    _net_buf.size(), 0, _sock.get_hint(), _len
                );

                if (_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in sending\n">();
                    return net::received_state::err;
                }

                return net::received_state::ok;
            }
        };

        class multi_server
        {
        private:
            using socket_type = 
                server_detail::multi_smart_sock;
            vector<socket_type> _socks;

        public:
            inline multi_server() = default;

            inline multi_server(multi_server&& other)
                : _socks{move(other._socks)}
            {}

            inline multi_server& operator=(multi_server&& rhs)
            {
                _socks = move(rhs._socks);
                return *this;
            }

            inline auto& get()
            {
                return _socks;
            }

            inline auto& operator[](usize index)
            {
                return _socks[index];
            }

            inline void add(net::protocol_type protocol, const char* ip_addr)
            {
                _socks.emplace_back(protocol, ip_addr);
            }

            inline usize size()
            {
                return _socks.size();
            }

            inline auto begin()
            {
                return _socks.begin();
            }

            inline auto end()
            {
                return _socks.end();
            }

            inline auto rbegin()
            {
                return _socks.rbegin();
            }

            inline auto rend()
            {
                return _socks.rend();
            }
        };
    } // namespace udp

    namespace tcp
    {
        namespace server_detail
        {
            using protocol_type = net::protocol_type;

            class socket_support
            {
            private:
                #if defined(HSD_PLATFORM_WINDOWS)
                SOCKET _listening = -1;
                #else
                i32 _listening = -1;
                #endif

            public:
                #if defined(HSD_PLATFORM_WINDOWS)
                static constexpr SOCKET invalid_socket = -1;
                #else
                static constexpr i32 invalid_socket = -1;
                #endif

                inline socket_support(protocol_type protocol = protocol_type::ipv4, 
                    const char* ip_addr = "0.0.0.0:54000")
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    network_detail::init_winsock();
                    #endif
                    
                    switch_to(protocol, ip_addr);
                }

                inline socket_support(const socket_support&) = delete;
                inline socket_support& operator=(const socket_support&) = delete;

                inline socket_support(socket_support&& other)
                {
                    _listening = exchange(other._listening, invalid_socket);
                }

                inline socket_support& operator=(socket_support&& rhs)
                {
                    _listening = exchange(rhs._listening, invalid_socket);
                    return *this;
                }

                inline ~socket_support()
                {
                    close();
                }

                inline void close()
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    ::closesocket(_listening);
                    #else
                    ::close(_listening);
                    #endif

                    _listening = -1;
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                inline SOCKET get_listener()
                #else
                inline i32 get_listener()
                #endif
                {
                    return _listening;
                }

                inline void switch_to(net::protocol_type protocol, const char* ip_addr)
                {      
                    if (_listening != invalid_socket)
                        close();

                    addrinfo* _result = nullptr;
                    addrinfo* _rp = nullptr;

                    auto _hints = addrinfo
                    {
                        .ai_flags = AI_PASSIVE,
                        .ai_family = static_cast<i32>(protocol),
                        .ai_socktype = net::socket_type::stream,
                        .ai_protocol = IPPROTO_TCP,
                        .ai_addrlen = 0,
                        #if defined(HSD_PLATFORM_WINDOWS)
                        .ai_canonname = nullptr,
                        .ai_addr = nullptr,
                        #else
                        .ai_addr = nullptr,
                        .ai_canonname = nullptr,
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

                        _domain_addr = mallocator::allocate_multiple<
                            char>(_domain_len + 1).unwrap();

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

                        mallocator::deallocate(_domain_addr);
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

                        if (_listening != invalid_socket)
                        {
                            if (bind(_listening, _rp->ai_addr, _rp->ai_addrlen) == 0)
                                break;

                            close();
                        }
                    }

                    if (_rp == nullptr)
                    {
                        fprintf(stderr, "Could not bind\n");
                        freeaddrinfo(_result);
                        return;
                    }

                    listen(_listening, SOMAXCONN);
                    freeaddrinfo(_result);
                    return;
                }
            };

            class socket
            {
            private:
                #if defined(HSD_PLATFORM_WINDOWS)
                SOCKET _sock_value = 0;
                #else
                i32 _sock_value = 0;
                #endif

                socket_support _sock;
                sockaddr_storage _sock_hint{};

                inline sockaddr* _get_hint()
                {
                    return reinterpret_cast<sockaddr*>(&_sock_hint);
                }

            public:
                inline socket()
                {
                    switch_to(net::protocol_type::ipv4, "0.0.0.0:54000");
                }

                inline socket(net::protocol_type protocol, const char* ip_addr)
                {
                    switch_to(protocol, ip_addr);
                }

                inline socket(socket&& other)
                    : _sock{move(other._sock)}
                {
                    _sock_value = exchange(other._sock_value, socket_support::invalid_socket);
                }

                inline socket& operator=(socket&& rhs)
                {
                    _sock = move(rhs._sock);
                    _sock_value = exchange(rhs._sock_value, socket_support::invalid_socket);
                    return *this;
                }

                inline ~socket()
                {
                    close();
                }

                inline void close()
                {
                    #if defined(HSD_PLATFORM_WINDOWS)
                    ::closesocket(_sock_value);
                    #else
                    ::close(_sock_value);
                    #endif
                }

                #if defined(HSD_PLATFORM_WINDOWS)
                inline SOCKET get_sock()
                #else
                inline i32 get_sock()
                #endif
                {
                    return _sock_value;
                }

                inline void switch_to(net::protocol_type protocol, const char* ip_addr)
                {
                    socklen_t _size = sizeof(sockaddr_storage);
                    _sock.switch_to(protocol, ip_addr);
                    _sock_value = accept(_sock.get_listener(), _get_hint(), &_size);
                    _sock.close();
                }
            };
        } // namespace server_detail

        class server
        {
        private:
            server_detail::socket _sock;
            hsd::sstream _net_buf{4095};

            inline void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            inline server() = default;
            inline ~server() = default;

            inline server(server&& other)
                : _sock{move(other._sock)}
            {}

            inline server& operator=(server&& rhs)
            {
                _sock = move(rhs._sock);
                return *this;
            }

            inline server(net::protocol_type protocol, const char* ip_addr)
                : _sock{protocol, ip_addr}
            {}

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
                    hsd::io::err_print<"Client disconnected\n">();
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

                if (_response == static_cast<isize>(net::received_state::err))
                {
                    hsd::io::err_print<"Error in sending\n">();
                    return net::received_state::err;
                }

                return net::received_state::ok;
            }
        };
    } // namespace tcp
} // namespace hsd