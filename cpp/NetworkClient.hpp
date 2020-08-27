#pragma once

#include "_NetworkDetail.hpp"
#include "Io.hpp"

#ifdef __linux__

namespace hsd
{
    namespace tcp
    {
        namespace client_detail
        {
            class socket
            {
            private:
                int _sock = 0;
                sockaddr_in6 _hintv6;
                sockaddr_in _hintv4;
                protocol_type _protocol;
                friend class tcp_client;

            public:
                socket()
                {
                    switch_to(protocol_type::ipv4, 54000, "127.0.0.1");
                }

                socket(protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    switch_to(protocol, port, ip_addr);
                }

                ~socket()
                {
                    close();
                }

                void close()
                {
                    ::close(_sock);
                }

                int get_sock()
                {
                    return _sock;
                }

                void switch_to(protocol_type protocol, uint16_t port, const char* ip_addr)
                {
                    close();
                    _protocol = protocol;

                    if(_protocol == protocol_type::ipv4)
                    {
                        _sock = ::socket(AF_INET, SOCK_STREAM, 0);
                        _hintv4.sin_family = AF_INET;
                        _hintv4.sin_port = htons(port);
                        inet_pton(AF_INET, ip_addr, &_hintv4.sin_addr);
                        connect(_sock, reinterpret_cast<sockaddr*>(&_hintv4), sizeof(_hintv4));
                    }
                    else
                    {
                        _sock = ::socket(AF_INET6, SOCK_STREAM, 0);
                        _hintv6.sin6_family = AF_INET6;
                        _hintv6.sin6_port = htons(port);
                        inet_pton(AF_INET6, ip_addr, &_hintv4.sin_addr);
                        connect(_sock, reinterpret_cast<sockaddr*>(&_hintv6), sizeof(_hintv6));
                    }
                }
            };
        } // namespace client_detail

        class client
        {
        private:
            client_detail::socket _sock;
            hsd::u8sstream _net_buf{4096};

        public:
            client() = default;

            client(protocol_type protocol, uint16_t port, const char* ip_addr)
                : _sock{protocol, port, ip_addr}
            {}

            hsd::pair< hsd::u8string, received_state > receive()
            {
                long _response = recv(_sock.get_sock(), 
                    _net_buf.data(), _net_buf.size(), 0);

                if (_response == static_cast<long>(received_state::err))
                {
                    hsd::io::err_print("Error in receiving\n");
                    _net_buf.reset_data();
                    return {"", received_state::err};
                }
                if (_response == static_cast<long>(received_state::disconnected))
                {
                    hsd::io::err_print("Server down\n");
                    _net_buf.reset_data();
                    return {"", received_state::disconnected};
                }

                return {{_net_buf.data(), static_cast<size_t>(_response) - 1}, received_state::ok};
            }

            template< size_t N, typename... Args >
            received_state respond(const char (&fmt)[N], Args&&... args)
            {
                hsd::vector<hsd::u8string> _fmt_buf = io_detail::split(fmt, N - 1);
                hsd::vector<hsd::u8string> _args_buf = {
                    hsd::forward<hsd::u8string>(hsd::u8string::to_string(args))...
                };
                u8string _send_buf;

                if(_args_buf.size() != _fmt_buf.size() && _args_buf.size() + 1 != _fmt_buf.size())
                {
                    throw std::runtime_error("Arguments don\'t match");
                }
                else
                {
                    size_t index = 0;

                    for(; index < _args_buf.size(); index++)
                    {
                        _send_buf += _fmt_buf[index] + _args_buf[index];
                    }
                    if(_fmt_buf.size() != _args_buf.size())
                    {
                        _send_buf += _fmt_buf[index];
                    }

                    long _response = send(_sock.get_sock(), 
                        _send_buf.data(), _send_buf.size() + 1, 0);

                    if(_response == static_cast<long>(received_state::err))
                    {
                        hsd::io::err_print("Error in sending\n");
                        return received_state::err;
                    }
                }

                return received_state::ok;
            }
        };
    } // namespace tcp
} // namespace hsd

#endif