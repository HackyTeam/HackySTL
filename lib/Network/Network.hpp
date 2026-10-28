#pragma once

/// @file Network.hpp
/// @brief Public TCP and UDP client/server wrappers.
/// @details Exposes polling servers, connected clients, peer iteration, and
/// send/receive operations while delegating native socket ownership internally.

#include "_SocketDetail.hpp"
#include "../Containers/UnorderedMap.hpp"

namespace hsd
{
    /// @brief Nonblocking TCP listening server with child-socket iteration.
    /// @tparam Protocol - The IP protocol (ipv4/ipv6) to bind on.
    template <network_detail::ip_protocol Protocol>
    class tcp_server
    {
    public:
        /// @brief Underlying accepted-connection socket type.
        using socket_type = network_detail::tcp_child_socket<Protocol>;

    private:
        /// Listening socket at index 0 followed by accepted child sockets.
        vector<socket_type> _sockets;

    public:
        ///
        /// @brief Constructs a listening TCP server bound to `addr`.
        /// On Windows this also lazily initializes Winsock.
        /// 
        /// @param addr - Address (optionally "host:port") to bind and listen on.
        ///
        inline tcp_server(const char* addr)
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _sockets.emplace_back(addr, true);
        }
        
        ///
        /// @brief Polls the listening socket and all accepted child sockets
        /// for readiness, accepting a new connection if one is pending.
        /// 
        /// @param timeout - Maximum time to wait in milliseconds, or -1 to
        /// block indefinitely.
        ///
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

        ///
        /// @brief Gets the platform error message for the last failed socket call.
        /// 
        /// @return const char* - The error message.
        ///
        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        ///
        /// @brief Gets the platform error code for the last failed socket call.
        /// 
        /// @return auto - The error code.
        ///
        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        ///
        /// @brief Removes an accepted child socket, closing its connection.
        /// 
        /// @param it - Iterator to the child socket to erase.
        /// @return auto - Iterator following the removed element.
        ///
        inline auto erase(socket_type* it)
        {
            return _sockets.erase(it);
        }

        ///
        /// @brief Iterator to the first accepted child socket, skipping the
        /// listening socket stored at index 0.
        /// 
        /// @return auto - Iterator to the first child socket.
        ///
        inline auto begin()
        {
            // Always skip the first element.
            // Note: the first element is
            // always the listening socket.
            return _sockets.begin() + 1;
        }

        ///
        /// @brief End iterator for the accepted child sockets.
        /// 
        /// @return auto - Iterator one past the last child socket.
        ///
        inline auto end()
        {
            return _sockets.end();
        }
    };

    /// @brief Connected TCP client for a selected IP protocol.
    /// @tparam Protocol - The IP protocol (ipv4/ipv6) to connect with.
    template <network_detail::ip_protocol Protocol>
    class tcp_client
    {
    private:
        /// Native OS socket handle type.
        using underlying_socket = network_detail::native_socket_type;
        /// Connected TCP socket wrapper for the selected protocol.
        using socket_type = network_detail::tcp_socket<Protocol>;
        /// Connected (or failed-to-connect) client socket.
        socket_type _socket;

    public:
        ///
        /// @brief Constructs a TCP client and connects it to `addr`.
        /// On Windows this also lazily initializes Winsock. If the connection
        /// attempt fails, an error is printed to stderr and the socket is left
        /// invalid (the object is otherwise still usable, just non-functional).
        /// 
        /// @param addr - Address (optionally "host:port") to connect to.
        ///
        inline tcp_client(const char* addr)
            : _socket{pollfd{static_cast<underlying_socket>(-1), POLLOUT, 0}}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            if (_socket.create_socket(addr, false) == false)
            {
                fputs("Error in initializing socket", stderr);
            }
        }

        ///
        /// @brief Gets the platform error message for the last failed socket call.
        /// 
        /// @return const char* - The error message.
        ///
        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        ///
        /// @brief Gets the platform error code for the last failed socket call.
        /// 
        /// @return auto - The error code.
        ///
        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        ///
        /// @brief Sends data on the connected socket.
        /// 
        /// @param data - Buffer to send.
        /// @param size - Number of bytes to send.
        /// @return isize - Number of bytes sent, or -1 on failure/invalid socket.
        ///
        inline isize send(const char* data, usize size)
        {
            return _socket.send(data, size);
        }

        ///
        /// @brief Receives data from the connected socket.
        /// 
        /// @param data - Buffer to receive into.
        /// @param size - Maximum number of bytes to receive.
        /// @return isize - Number of bytes received, 0 if the socket is not
        /// currently readable, or -1 on invalid socket.
        ///
        inline isize receive(char* data, usize size)
        {
            return _socket.receive(data, size);
        }
    };

    /// @brief IPv4 TCP server alias.
    using tcp_server_v4 = tcp_server<network_detail::ip_protocol::ipv4>;
    /// @brief IPv6 TCP server alias.
    using tcp_server_v6 = tcp_server<network_detail::ip_protocol::ipv6>;
    /// @brief IPv4 TCP client alias.
    using tcp_client_v4 = tcp_client<network_detail::ip_protocol::ipv4>;
    /// @brief IPv6 TCP client alias.
    using tcp_client_v6 = tcp_client<network_detail::ip_protocol::ipv6>;

    /// @brief UDP server that tracks remote peers discovered by polling.
    /// @tparam Protocol - The IP protocol (ipv4/ipv6) to bind on.
    template <network_detail::ip_protocol Protocol>
    class udp_server
    {
    private:
        /// Native OS socket handle type.
        using underlying_socket = network_detail::native_socket_type;
        /// Per-peer child socket type used to reply to a tracked peer.
        using socket_type = network_detail::udp_child_socket<Protocol>;
        /// Bound socket used to both receive datagrams and reply per-peer.
        network_detail::udp_socket<Protocol> _main_socket;
        /// Peers discovered so far, keyed by their packed address/port string.
        unordered_map<static_string<18>, socket_type> _sockets{};

    public:
        ///
        /// @brief Constructs a UDP server bound to `addr`. On Windows this
        /// also lazily initializes Winsock.
        /// 
        /// @param addr - Address (optionally "host:port") to bind on.
        ///
        inline udp_server(const char* addr)
            : _main_socket{static_cast<underlying_socket>(-1), sockaddr_storage{}, 0}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            _main_socket.create_socket(addr, true);
        }

        ///
        /// @brief Gets the platform error message for the last failed socket call.
        /// 
        /// @return const char* - The error message.
        ///
        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        ///
        /// @brief Gets the platform error code for the last failed socket call.
        /// 
        /// @return auto - The error code.
        ///
        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        ///
        /// @brief Receives one pending datagram (if any) on the bound socket
        /// and appends it to the buffer of the peer it came from, inserting a
        /// new peer entry on first contact. Aborts the process on any receive
        /// error other than a would-block condition.
        ///
        inline void poll()
        {
            thread_local static static_vector<char, 1024> _buffer;
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

        ///
        /// @brief Removes a tracked peer.
        /// 
        /// @param it - Iterator to the peer entry to erase.
        /// @return auto - Iterator following the removed element.
        ///
        inline auto erase(socket_type* it)
        {
            return _sockets.erase(it);
        }

        ///
        /// @brief Iterator to the first tracked peer.
        /// 
        /// @return auto - Iterator to the first peer entry.
        ///
        inline auto begin()
        {
            return _sockets.begin();
        }

        ///
        /// @brief End iterator for tracked peers.
        /// 
        /// @return auto - Iterator one past the last peer entry.
        ///
        inline auto end()
        {
            return _sockets.end();
        }
    };

    /// @brief UDP client for sending and receiving datagrams.
    /// @tparam Protocol - The IP protocol (ipv4/ipv6) to connect with.
    template <network_detail::ip_protocol Protocol>
    class udp_client
    {
    private:
        /// Native OS socket handle type.
        using underlying_socket = network_detail::native_socket_type;
        /// Connected UDP socket wrapper for the selected protocol.
        using socket_type = network_detail::udp_socket<Protocol>;
        /// Connected (or failed-to-connect) client socket.
        socket_type _socket;

    public:
        ///
        /// @brief Constructs a UDP client and connects it to `addr`, fixing
        /// the peer used by send()/receive(). On Windows this also lazily
        /// initializes Winsock. If connecting fails, an error is printed to
        /// stderr and the socket is left invalid.
        /// 
        /// @param addr - Address (optionally "host:port") to connect to.
        ///
        inline udp_client(const char* addr)
            : _socket{static_cast<underlying_socket>(-1), sockaddr_storage{}, 0}
        {
            #if defined(HSD_PLATFORM_WINDOWS)
            network_detail::init_winsock();
            #endif

            if (_socket.create_socket(addr, false) == false)
            {
                fputs("Error in initializing socket", stderr);
            }
        }

        ///
        /// @brief Gets the platform error message for the last failed socket call.
        /// 
        /// @return const char* - The error message.
        ///
        static inline const char* error_message()
        {
            return network_detail::sock_error_msg();
        }

        ///
        /// @brief Gets the platform error code for the last failed socket call.
        /// 
        /// @return auto - The error code.
        ///
        static inline auto error_code()
        {
            return network_detail::sock_error_code();
        }

        ///
        /// @brief Sends data to the connected peer.
        /// 
        /// @param data - Buffer to send.
        /// @param size - Number of bytes to send.
        /// @return isize - Number of bytes sent, or -1 on failure/invalid socket.
        ///
        inline isize send(const char* data, usize size)
        {
            return _socket.send(data, size);
        }

        ///
        /// @brief Receives a datagram from the connected peer.
        /// 
        /// @param data - Buffer to receive into.
        /// @param size - Maximum number of bytes to receive.
        /// @return isize - Number of bytes received, or -1 on invalid socket.
        ///
        inline isize receive(char* data, usize size)
        {
            return _socket.receive(data, size);
        }
    };

    /// @brief IPv4 UDP server alias.
    using udp_server_v4 = udp_server<network_detail::ip_protocol::ipv4>;
    /// @brief IPv6 UDP server alias.
    using udp_server_v6 = udp_server<network_detail::ip_protocol::ipv6>;
    /// @brief IPv4 UDP client alias.
    using udp_client_v4 = udp_client<network_detail::ip_protocol::ipv4>;
    /// @brief IPv6 UDP client alias.
    using udp_client_v6 = udp_client<network_detail::ip_protocol::ipv6>;
} // namespace hsd