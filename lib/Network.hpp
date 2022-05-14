#pragma once

#include "_SocketDetail.hpp"
#include "UnorderedMap.hpp"

namespace hsd
{
    template <network_detail::ip_protocol Protocol>
    class tcp_server
    {
    private:
        using socket_type = network_detail::tcp_child_socket<Protocol>;
        vector<socket_type> _sockets;

    public:
        inline tcp_server(const char* const addr)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _sockets.emplace_back(addr, true);
        }
        
        inline void poll(hsd::i32 timeout = -1)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            auto _result = ::WSAPoll(
                reinterpret_cast<pollfd*>(
                    _sockets.data()
                ), _sockets.size(), timeout
            );
            #else
            auto _result = ::poll(
                reinterpret_cast<pollfd*>(
                    _sockets.data()
                ), _sockets.size(), timeout
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
            return network_detail::sock_error_msg();
        }

        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        inline auto begin()
        {
            // Always skip the first element.
            // Note: the first element is
            // always the listening socket.
            return _sockets.begin() + 1;
        }

        inline auto end()
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
            return network_detail::sock_error_msg();
        }

        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        inline isize send(const char* const data, const usize size)
        {
            return _socket.send(data, size);
        }

        inline isize receive(char* const data, const usize size)
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
        using socket_type = network_detail::udp_child_socket<Protocol>;
        network_detail::udp_socket<Protocol> _main_socket;
        unordered_map<static_string<18>, socket_type> _sockets{};

    public:
        inline udp_server(const char* const addr)
            : _main_socket{static_cast<underlying_socket>(-1), sockaddr_storage{}, 0}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _main_socket.switch_to(addr, true);
        }

        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        inline void poll()
        {
            static static_vector<char, 1024> _buffer;
            _buffer.resize(1024);
            memset(&_main_socket.addr(), 0, _main_socket.length());
            auto _result = _main_socket.receive(
                _buffer.data(), _buffer.size()
            );
            
            if (_result < 0)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                if (error_code() != WSAEWOULDBLOCK)
                #else
                if (error_code() != EWOULDBLOCK && error_code() != EAGAIN)
                #endif
                {
                    fprintf(
                        stderr, "udp_server: error in receive"
                        ", error: %s", error_message()
                    );
                    
                    abort();
                }
            }
            else
            {
                auto _key_string = network_detail::to_str(&_main_socket.addr());
                auto& _find_result = _sockets.emplace(
                    _key_string, _main_socket.fd(), 
                    _main_socket.addr(), _main_socket.length()
                ).first->second;

                _buffer.resize(_result);
                _find_result.add(_buffer);
            }
        }

        inline auto begin()
        {
            return _sockets.begin();
        }
        inline auto end()
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
        }

        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        inline isize send(const char* const data, const usize size)
        {
            return _socket.send(data, size);
        }

        inline isize receive(char* const data, const usize size)
        {
            return _socket.receive(data, size);
        }
    };

    using udp_server_v4 = udp_server<network_detail::ip_protocol::ipv4>;
    using udp_server_v6 = udp_server<network_detail::ip_protocol::ipv6>;
    using udp_client_v4 = udp_client<network_detail::ip_protocol::ipv4>;
    using udp_client_v6 = udp_client<network_detail::ip_protocol::ipv6>;
} // namespace hsd