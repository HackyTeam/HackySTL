#pragma once

#include "_NetworkDetail.hpp"
#include "Io.hpp"

#ifdef HSD_PLATFORM_LINUX

namespace hsd
{
    namespace udp
    {
        namespace server_detail
        {
            class socket
            {
            private:
                i32 _listening = 0;
                sockaddr_in6 _hintv6{};
                sockaddr_in _hintv4{};
                net::protocol_type _protocol;

            public:
                socket()
                {
                    switch_to(net::protocol_type::ipv4, 54000, "0.0.0.0");
                }

                socket(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    switch_to(protocol, port, ip_addr);
                }

                ~socket()
                {
                    close();
                }

                void close()
                {
                    ::close(_listening);
                }

                i32 get_listening()
                {
                    return _listening;
                }

                void switch_to(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    close();
                    _protocol = protocol;

                    if(_protocol == net::protocol_type::ipv4)
                    {
                        _listening = ::socket(static_cast<i32>(_protocol), net::socket_type::dgram, 0);
                        _hintv4.sin_family = static_cast<u16>(_protocol);
                        _hintv4.sin_port = htons(port);
                        inet_pton(static_cast<i32>(_protocol), ip_addr, &_hintv4.sin_addr);
                        bind(_listening, reinterpret_cast<sockaddr*>(&_hintv4), sizeof(_hintv4));
                    }
                    else
                    {
                        _listening = ::socket(static_cast<i32>(_protocol), net::socket_type::dgram, 0);
                        _hintv6.sin6_family = static_cast<u16>(_protocol);
                        _hintv6.sin6_port = htons(port);
                        inet_pton(static_cast<i32>(_protocol), ip_addr, &_hintv6.sin6_addr);
                        bind(_listening, reinterpret_cast<sockaddr*>(&_hintv6), sizeof(_hintv6));
                    }
                }
            };
        } // namespace server_detail

        class server
        {
        private:
            server_detail::socket _sock;
            sockaddr_in6 _hintv6{};
            sockaddr_in _hintv4{};
            socklen_t _len = sizeof(sockaddr_in);
            net::protocol_type _protocol = net::protocol_type::ipv4;
            hsd::sstream _net_buf{4095};

            void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            server() = default;
            
            ~server()
            {
                respond<"">();
            }

            server(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                : _sock{protocol, port, ip_addr}, _protocol{protocol}
            {
                if(_protocol == net::protocol_type::ipv6)
                    _len = sizeof(sockaddr_in6);
            }

            hsd::pair< hsd::sstream&, net::received_state > receive()
            {
                _clear_buf();
                isize _response = 0;
                
                if(_protocol == net::protocol_type::ipv4)
                {
                    _response = recvfrom(_sock.get_listening(), _net_buf.data(), 
                        4096, 0, reinterpret_cast<sockaddr*>(&_hintv4), &_len);
                }
                else
                {
                    _response = recvfrom(_sock.get_listening(), _net_buf.data(), 
                        4096, 0, reinterpret_cast<sockaddr*>(&_hintv6), &_len);
                }
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

            template< string_literal fmt, typename... Args >
            net::received_state respond(Args&&... args)
            {
                _clear_buf();
                _net_buf.write_data<fmt>(forward<Args>(args)...);

                isize _response = 0;

                if(_protocol == net::protocol_type::ipv4)
                {
                    _response = sendto(_sock.get_listening(), _net_buf.data(), 
                        _net_buf.size(), 0, reinterpret_cast<sockaddr*>(&_hintv4), _len);
                }
                else
                {
                    _response = sendto(_sock.get_listening(), _net_buf.data(), 
                        _net_buf.size(), 0, reinterpret_cast<sockaddr*>(&_hintv6), _len);
                }
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
        namespace server_detail
        {
            class socket
            {
            private:
                i32 _listening = 0;
                sockaddr_in6 _hintv6;
                sockaddr_in _hintv4;
                net::protocol_type _protocol;

            public:
                socket()
                {
                    switch_to(net::protocol_type::ipv4, 54000, "0.0.0.0");
                }

                socket(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    switch_to(protocol, port, ip_addr);
                }

                ~socket()
                {
                    close();
                }

                void close()
                {
                    ::close(_listening);
                }

                i32 get_listener()
                {
                    return _listening;
                }

                void switch_to(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    close();
                    _protocol = protocol;

                    if(_protocol == net::protocol_type::ipv4)
                    {
                        _listening = ::socket(static_cast<i32>(_protocol), net::socket_type::stream, 0);
                        _hintv4.sin_family = static_cast<u16>(_protocol);
                        _hintv4.sin_port = htons(port);
                        inet_pton(static_cast<i32>(_protocol), ip_addr, &_hintv4.sin_addr);
                        bind(_listening, reinterpret_cast<sockaddr*>(&_hintv4), sizeof(_hintv4));
                        listen(_listening, SOMAXCONN);
                    }
                    else
                    {
                        _listening = ::socket(static_cast<i32>(_protocol), net::socket_type::stream, 0);
                        _hintv6.sin6_family = static_cast<u16>(_protocol);
                        _hintv6.sin6_port = htons(port);
                        inet_pton(static_cast<i32>(_protocol), ip_addr, &_hintv4.sin_addr);
                        bind(_listening, reinterpret_cast<sockaddr*>(&_hintv6), sizeof(_hintv6));
                        listen(_listening, SOMAXCONN);
                    }
                }
            };

            class socket2
            {
            private:
                socket _sock;
                i32 _socket2_sock = 0;
                sockaddr_in _socket2v4;
                sockaddr_in6 _socket2v6;
                socklen_t _size = 0;
                string _host{1024};
                string _service{31};

            public:
                socket2()
                {
                    switch_to(net::protocol_type::ipv4, 54000, "0.0.0.0");
                }

                socket2(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    switch_to(protocol, port, ip_addr);
                }

                ~socket2()
                {
                    close();
                }

                void close()
                {
                    ::close(_socket2_sock);
                }

                i32 get_sock()
                {
                    return _socket2_sock;
                }

                void switch_to(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    _sock.switch_to(protocol, port, ip_addr);
                    memset(_host.data(), '\0', NI_MAXHOST);
                    memset(_service.data(), '\0', NI_MAXSERV);

                    if(protocol == net::protocol_type::ipv4)
                    {
                        _size = sizeof(_socket2v4);
                        _socket2_sock = accept(_sock.get_listener(), reinterpret_cast<sockaddr*>(&_socket2v4), &_size);

                        if(getnameinfo(reinterpret_cast<sockaddr*>(&_socket2v4), sizeof(_socket2v4), 
                            _host.data(), NI_MAXHOST, _service.data(), NI_MAXSERV, 0) == 0)
                        {
                            io::print<"{} connected on port {}\n">(ip_addr, ntohs(_socket2v4.sin_port));
                        }
                        else
                        {
                            inet_ntop(static_cast<i32>(protocol), &_socket2v4.sin_addr, _host.data(), NI_MAXHOST);
                            hsd::io::print<"{} connected on port {}\n">(ip_addr, ntohs(_socket2v4.sin_port));
                        }
                    }
                    else
                    {
                        _size = sizeof(_socket2v6);
                        _socket2_sock = accept(_sock.get_listener(), reinterpret_cast<sockaddr*>(&_socket2v6), &_size);

                        if(getnameinfo(reinterpret_cast<sockaddr*>(&_socket2v6), sizeof(_socket2v6), 
                                _host.data(), NI_MAXHOST, _service.data(), NI_MAXSERV, 0) == 0)
                        {
                            io::print<"{} connected on port {}\n">(ip_addr, _service.data());
                        }
                        else
                        {
                            inet_ntop(static_cast<i32>(protocol), &_socket2v6.sin6_addr, _host.data(), NI_MAXHOST);
                            hsd::io::print<"{} connected on port {}\n">(ip_addr, ntohs(_socket2v6.sin6_port));
                        }
                    }

                    _sock.close();
                }
            };
        } // namespace server_detail

        class server
        {
        private:
            server_detail::socket2 _sock;
            hsd::sstream _net_buf{4095};

            void _clear_buf()
            {
                memset(_net_buf.data(), '\0', 4096);
            }

        public:
            server() = default;
            ~server() = default;

            server(net::protocol_type protocol, uint16_t port, const char* ip_addr)
                : _sock{protocol, port, ip_addr}
            {}

            hsd::pair< hsd::sstream&, net::received_state > receive()
            {
                _clear_buf();
                isize _response = recv(_sock.get_sock(), 
                    _net_buf.data(), 4096, 0);

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

            template < string_literal fmt, typename... Args >
            net::received_state respond(Args&&... args)
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

#endif