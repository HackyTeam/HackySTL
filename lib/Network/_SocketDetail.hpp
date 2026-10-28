#pragma once

/// @file _SocketDetail.hpp
/// @brief Internal TCP and UDP socket ownership and I/O implementation.
/// @details Implements native handles, nonblocking setup, polling, accept,
/// child sockets, and platform-specific send/receive behavior.

#include "_NetworkFuncs.hpp"
#include "../Containers/List.hpp"
#include "../Containers/Vector.hpp"

namespace hsd::network_detail
{
    ///
    /// @brief TCP socket handler implementation
    /// 
    /// @tparam Protocol - protocol type
    ///
    template <ip_protocol Protocol>
    class tcp_socket
    {
    private:
        /// Data type for socket handle
        pollfd _data;

        ///
        /// @brief Creates a server socket using the given address
        /// 
        /// @param info - Address info used to create the socket
        /// @return true - Socket created successfully
        /// @return false - Socket creation failed
        ///
        inline bool _handle_server(const addrinfo* const info)
        {
            if (info->ai_socktype == SOCK_STREAM)
            {   
                if (is_valid() == false)
                {
                    return false;
                }
                
                _data.events = POLLIN;
                _data.revents = 0;
                
                // SO_REUSEADDR must be set before bind() to take effect.
                #if defined(HSD_PLATFORM_WINDOWS)
                static ulong _on = 1;
                auto _rc = setsockopt(
                    _data.fd, SOL_SOCKET, SO_REUSEADDR, 
                    reinterpret_cast<char*>(&_on), sizeof(_on)
                );
                #else
                static i32 _on = 1;
                auto _rc = setsockopt(
                    _data.fd, SOL_SOCKET, 
                    SO_REUSEADDR,
                    &_on, sizeof(_on)
                );
                #endif
                
                if (_rc < 0)
                {
                    close();
                    panic("setsockopt() failed");
                }

                if (::bind(_data.fd, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

            
                #if defined(HSD_PLATFORM_WINDOWS)
                _rc = ioctlsocket(_data.fd, FIONBIO, &_on);
                #else
                _rc = ioctl(_data.fd, FIONBIO, &_on);
                #endif

                if (_rc < 0)
                {
                    close();
                    panic("ioctl() failed");
                }

                if (::listen(_data.fd, SOMAXCONN) == -1)
                {
                    return false;
                }

                return true;
            }

            return false;
        }

        ///
        /// @brief Creates a client socket using the given address
        /// 
        /// @param info - Address info used to create the socket
        /// @return true - Socket created successfully
        /// @return false - Socket creation failed
        ///
        inline bool _handle_client(const addrinfo* const info)
        {
            if (info->ai_socktype == SOCK_STREAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }

                _data.events = POLLOUT;
                _data.revents = POLLOUT | POLLIN;
                
                if (::connect(_data.fd, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                return true;
            }

            return false;
        }

    public:
        ///
        /// @brief Deleted copy constructor for TCP socket
        ///
        inline tcp_socket(const tcp_socket&) = delete;

        ///
        /// @brief Deleted copy assignment operator for TCP socket
        ///
        inline tcp_socket& operator=(const tcp_socket&) = delete;
        
        ///
        /// @brief Construct a new tcp socket object
        /// 
        /// @param data - Socket data containing the socket
        /// handle and events type to be monitored
        ///
        inline tcp_socket(pollfd&& data)
            : _data{data}
        {}

        ///
        /// @brief Construct by move a new tcp socket object
        /// 
        /// @param other - socket class to be moved
        ///
        inline tcp_socket(tcp_socket&& other)
            : _data{hsd::move(other._data)}
        {
            other._data.fd = static_cast<network_detail::native_socket_type>(-1);
        }

        ///
        /// @brief Construct a new tcp socket
        /// object for the given address
        /// 
        /// @param addr - Address to be used to create the socket
        /// @param is_server - True if the socket is a
        /// server socket false if otherwise
        ///
        inline tcp_socket(const char* const addr, bool is_server = false)
        {
            _data = pollfd {
                .fd = static_cast<network_detail::native_socket_type>(-1),
                .events = 0,
                .revents = 0
            };

            if (create_socket(addr, is_server) == false)
            {
                fprintf(
                    stderr, "hsd::network_detail::"
                    "tcp_socket::create_socket()"
                    " failed. with error: %s\n",
                    sock_error_msg()
                );
                close();
            }
        }

        ///
        /// @brief Destroy the tcp socket object
        ///
        inline ~tcp_socket()
        {
            close();
        }

        ///
        /// @brief Move attribution operator overload,
        /// swaps the data between 2 tcp_socket objects
        /// 
        /// @param rhs - The object which replaces the left hand one
        /// @return tcp_socket& - The reference to the left hand operand
        ///
        inline tcp_socket& operator=(tcp_socket&& rhs)
        {
            hsd::swap(_data, rhs._data);
            return *this;
        }

        ///
        /// @brief Attributes the socket a new pollfd
        /// value whilst closing the old connection
        /// 
        /// @param rhs - Data structure describing a polling request
        /// @return tcp_socket& - The reference to the left hand operand
        ///
        inline tcp_socket& operator=(pollfd&& rhs)
        {
            close();
            _data = rhs;
            return *this;
        }

        ///
        /// @brief Checks if 2 sockets have the same file descriptor
        /// 
        /// @param rhs - Socket to compare with
        /// @return true - If the sockets share the same file descriptor
        /// @return false - If they are unique sockets
        ///
        inline bool operator==(const tcp_socket& rhs) const
        {
            return _data.fd == rhs._data.fd;
        }

        ///
        /// @brief Checks if 2 sockets have different file descriptor
        /// 
        /// @param rhs - Socket to compare with
        /// @return true - If the sockets are unique relative to each other
        /// @return false - If they are they share the same file descriptor
        ///
        inline bool operator!=(const tcp_socket& rhs) const
        {
            return _data.fd != rhs._data.fd;
        }

        ///
        /// @brief Closes the current socket
        /// and resets the event data stored 
        ///
        inline void close()
        {
            if (is_valid() == true)
            {
                close_socket(_data.fd);
                _data.fd = static_cast<native_socket_type>(-1);
                _data.events = _data.revents = 0;
            }
        }

        ///
        /// @brief Handles the current socket based on the `info`
        /// provided with separate logic for server/client
        /// 
        /// @param info - Address info used to create the socket
        /// @param is_server - If the socket is a server or client
        /// @return true - Socket created successfully
        /// @return false - Failed to create the socket
        ///
        inline bool handle(const addrinfo* info, bool is_server)
        {
            if (is_valid() == false)
            {
                fputs("tcp_socket::handle() called on invalid socket\n", stderr);
                return false;
            } 
            
            return is_server ? _handle_server(info) : _handle_client(info);
        }

        ///
        /// @brief Create a socket object
        /// 
        /// @param addr - IPv4/IPv6 address used to create the socket
        /// @param is_server - If the socket is a server or client
        /// @return true - Socket created successfully
        /// @return false - Failed to create the socket
        ///
        [[nodiscard]] inline bool create_socket(
            const char* addr, bool is_server = false)
        {
            return hsd::network_detail::create_socket(*this, addr, is_server);
        }

        ///
        /// @brief Checks if a socket has a valid native handle.
        /// 
        /// @return true - The socket holds a valid file descriptor/handle.
        /// @return false - The socket handle is -1 (closed or never created).
        ///
        inline bool is_valid() const
        {
            return _data.fd != static_cast<native_socket_type>(-1);
        }

        ///
        /// @brief Checks whether the last poll() reported readable data.
        /// 
        /// @return bool - True if the `POLLIN` bit is set in the last
        /// recorded revents.
        ///
        inline bool is_readable() const
        {
            return _data.revents & POLLIN;
        }

        ///
        /// @brief Checks whether the last poll() reported writability.
        /// 
        /// @return bool - True if the `POLLOUT` bit is set in the last
        /// recorded revents.
        ///
        inline bool is_writable() const
        {
            return _data.revents & POLLOUT;
        }

        ///
        /// @brief Gets the address family this socket was instantiated for.
        /// 
        /// @return auto - `AF_INET`/`AF_INET6` (the underlying value of
        /// `Protocol`) as an `i32`.
        ///
        static constexpr auto ip_protocol()
        {
            return static_cast<i32>(Protocol);
        }

        ///
        /// @brief Gets the socket type used to create this socket.
        /// 
        /// @return auto - `SOCK_STREAM` as an `i32`.
        ///
        static constexpr auto sock_type()
        {
            return static_cast<i32>(socket_type::stream);
        }

        ///
        /// @brief Gets the transport protocol used to create this socket.
        /// 
        /// @return auto - `IPPROTO_TCP` as an `i32`.
        ///
        static constexpr auto net_protocol()
        {
            return static_cast<i32>(socket_protocol::tcp);
        }

        ///
        /// @brief Gets the native socket handle.
        /// 
        /// @return auto - The native handle, or -1 if the socket is invalid.
        ///
        inline auto fd() const
        {
            return _data.fd;
        }

        ///
        /// @brief Sends data on the connected stream socket.
        /// 
        /// @param buffer - Buffer to send.
        /// @param size - Number of bytes to send.
        /// @return isize - Number of bytes sent, or -1 if the socket is
        /// invalid or the underlying `send()` call fails.
        ///
        inline isize send(const void* buffer, usize size)
        {
            if (is_valid() == false)
            {
                return -1;
            }
            
            #if defined(HSD_PLATFORM_WINDOWS)
            return ::send(
                _data.fd, reinterpret_cast<const char*>(buffer), size, 0
            );
            #else
            return ::send(_data.fd, buffer, size, 0);
            #endif
        }

        ///
        /// @brief Receives data from the connected stream socket.
        /// 
        /// @param buffer - Buffer to receive into.
        /// @param size - Maximum number of bytes to receive.
        /// @return isize - Number of bytes received, 0 if the socket is not
        /// currently readable (per the last poll()), or -1 if invalid.
        ///
        inline isize receive(void* buffer, usize size)
        {
            if (is_valid() == false)
            {
                return -1;
            }
            else if (is_readable() == true)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                return ::recv(
                    _data.fd, reinterpret_cast<char*>(buffer), size, 0
                );
                #else
                return ::recv(_data.fd, buffer, size, 0);
                #endif
            }

            return 0;
        }
    };

    ///
    /// @brief Connection accepted by a `tcp_server`, stored in its socket
    /// vector alongside the listening socket. Privately inherits
    /// `tcp_socket` so it can be `poll()`-ed uniformly with the listening
    /// socket while only re-exposing the subset of the API relevant to an
    /// already-connected peer (no `handle()`/`create_socket()`).
    /// 
    /// @tparam Protocol - protocol type
    ///
    template <ip_protocol Protocol>
    class tcp_child_socket
        : private tcp_socket<Protocol>
    {
    public:
        using tcp_socket<Protocol>::tcp_socket;

        ///
        /// @brief Constructs a child socket by taking ownership of an
        /// already-accepted native handle.
        /// 
        /// @param data - Socket data containing the accepted socket
        /// handle and events type to be monitored.
        ///
        inline tcp_child_socket(pollfd&& data)
            : tcp_socket<Protocol>(hsd::move(data))
        {}

        ///
        /// @brief Constructs by move a new tcp_child_socket object.
        /// 
        /// @param other - socket class to be moved.
        ///
        inline tcp_child_socket(tcp_child_socket&& other)
            : tcp_socket<Protocol>(hsd::move(other))
        {}

        ///
        /// @brief Constructs a new tcp_child_socket for the given address.
        /// 
        /// @param addr - Address to be used to create the socket.
        /// @param is_server - True if the socket is a server socket,
        /// false if otherwise.
        ///
        inline tcp_child_socket(const char* addr, bool is_server = false)
            : tcp_socket<Protocol>(addr, is_server)
        {}

        ///
        /// @brief Move assignment operator, swaps ownership with `other`.
        /// 
        /// @param other - The object which replaces the left hand one.
        /// @return tcp_child_socket& - Reference to the left hand operand.
        ///
        inline tcp_child_socket& operator=(tcp_child_socket&& other)
        {
            static_cast<tcp_socket<Protocol>&>(*this) = hsd::move(other);
            return *this;
        }

        ///
        /// @brief Assigns the socket a new pollfd value, closing the
        /// previous connection first.
        /// 
        /// @param rhs - Data structure describing a polling request.
        /// @return tcp_child_socket& - Reference to the left hand operand.
        ///
        inline tcp_child_socket& operator=(pollfd&& rhs)
        {
            static_cast<tcp_socket<Protocol>&>(*this) = hsd::move(rhs);
            return *this;
        }

        /// @brief Re-exposes `tcp_socket::is_valid`.
        using tcp_socket<Protocol>::is_valid;
        /// @brief Re-exposes `tcp_socket::is_readable`.
        using tcp_socket<Protocol>::is_readable;
        /// @brief Re-exposes `tcp_socket::is_writable`.
        using tcp_socket<Protocol>::is_writable;
        /// @brief Re-exposes `tcp_socket::fd`.
        using tcp_socket<Protocol>::fd;
        /// @brief Re-exposes `tcp_socket::ip_protocol`.
        using tcp_socket<Protocol>::ip_protocol;
        /// @brief Re-exposes `tcp_socket::sock_type`.
        using tcp_socket<Protocol>::sock_type;
        /// @brief Re-exposes `tcp_socket::net_protocol`.
        using tcp_socket<Protocol>::net_protocol;
        /// @brief Re-exposes `tcp_socket::send`.
        using tcp_socket<Protocol>::send;
        /// @brief Re-exposes `tcp_socket::receive`.
        using tcp_socket<Protocol>::receive;
    };

    ///
    /// @brief UDP socket handler implementation. Unlike `tcp_socket`, a
    /// single native handle is shared for both binding/connecting and
    /// datagram transfer; the remote peer address is cached in `_addr` and
    /// used as the implicit destination/source for `send()`/`receive()`.
    /// 
    /// @tparam Protocol - protocol type
    ///
    template <ip_protocol Protocol>
    class udp_socket
    {
    private:
        /// Native socket handle, or -1 if invalid/closed.
        native_socket_type _data;
        /// Cached peer address: bound local address (server) or
        /// connected remote address (client).
        sockaddr_storage _addr;
        /// Length in bytes of the valid portion of `_addr`.
        socklen_t _addr_len;

        ///
        /// @brief Binds the socket as a datagram server on the given address.
        /// 
        /// @param info - Address info used to bind the socket.
        /// @return true - Socket bound successfully.
        /// @return false - Binding failed or `info` was not a datagram type.
        ///
        inline bool _handle_server(const addrinfo* info)
        {
            if (info->ai_socktype == SOCK_DGRAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }

                // SO_REUSEADDR must be set before bind() to take effect.
                #if defined(HSD_PLATFORM_WINDOWS)
                static ulong _on = 1;
                auto _rc = setsockopt(
                    _data, SOL_SOCKET, SO_REUSEADDR, 
                    reinterpret_cast<char*>(&_on), sizeof(_on)
                );
                #else
                static i32 _on = 1;
                auto _rc = setsockopt(
                    _data, SOL_SOCKET, 
                    SO_REUSEADDR,
                    &_on, sizeof(_on)
                );
                #endif
                
                if (_rc < 0)
                {
                    close();
                    panic("setsockopt() failed");
                }

                if (::bind(_data, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                memcpy(&_addr, info->ai_addr, info->ai_addrlen);
                _addr_len = info->ai_addrlen;

            
                #if defined(HSD_PLATFORM_WINDOWS)
                _rc = ioctlsocket(_data, FIONBIO, &_on);
                #else
                _rc = ioctl(_data, FIONBIO, &_on);
                #endif

                if (_rc < 0)
                {
                    close();
                    panic("ioctl() failed");
                }

                return true;
            }

            return false;
        }

        ///
        /// @brief Connects the socket as a datagram client to the given
        /// address, fixing the peer used by `send()`/`receive()`.
        /// 
        /// @param info - Address info used to connect the socket.
        /// @return true - Socket connected successfully.
        /// @return false - Connecting failed or `info` was not a datagram type.
        ///
        inline bool _handle_client(const addrinfo* info)
        {
            if (info->ai_socktype == SOCK_DGRAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }

                if (::connect(_data, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                memcpy(&_addr, info->ai_addr, info->ai_addrlen);
                _addr_len = info->ai_addrlen;
                return true;
            }

            return false;
        }

    public:
        ///
        /// @brief Deleted copy constructor for UDP socket.
        ///
        inline udp_socket(const udp_socket& other) = delete;

        ///
        /// @brief Deleted copy assignment operator for UDP socket.
        ///
        inline udp_socket& operator=(const udp_socket& other) = delete;

        ///
        /// @brief Constructs a new udp_socket object for the given address.
        /// Prints an error to stderr and leaves the socket invalid if
        /// creation/binding/connecting fails.
        /// 
        /// @param addr - Address to be used to create the socket.
        /// @param is_server - True if the socket is a server socket (bound),
        /// false if it should connect to `addr` as a client.
        ///
        inline udp_socket(const char* addr, bool is_server = false)
        {
            _data = static_cast<network_detail::native_socket_type>(-1);

            if (create_socket(addr, is_server) == false)
            {
                fprintf(
                    stderr, "hsd::network_detail::"
                    "udp_socket::create_socket() "
                    "failed. With error: %s\n", 
                    sock_error_msg()
                );
                
                close();
            }
        }

        ///
        /// @brief Constructs a udp_socket that adopts an already-bound
        /// native handle and its associated peer address, without creating
        /// a new socket. Used by `udp_server` to reply through its main
        /// socket for a specific tracked peer.
        /// 
        /// @param data - Native socket handle to adopt (not owned exclusively;
        /// the handle is shared with the caller, e.g. the server's main socket).
        /// @param info - Peer address to associate with this socket.
        /// @param addr_len - Length in bytes of the valid portion of `info`.
        ///
        inline udp_socket(native_socket_type data, 
            const sockaddr_storage& info, socklen_t addr_len)
            : _data{data}, _addr{info}, _addr_len{addr_len}
        {}

        ///
        /// @brief Constructs by move a new udp_socket object, leaving
        /// `other` with an invalid handle.
        /// 
        /// @param other - socket class to be moved.
        ///
        inline udp_socket(udp_socket&& other)
            : _data{other._data}, _addr{hsd::move(other._addr)}, _addr_len{other._addr_len}
        {
            other._addr_len = 0;
            other._data = static_cast<network_detail::native_socket_type>(-1);
        }

        ///
        /// @brief Destroys the udp_socket object, closing the native handle.
        ///
        inline ~udp_socket()
        {
            close();
        }

        ///
        /// @brief Assigns the socket a new native handle taken from `rhs`,
        /// closing the previous handle first. Note: unlike the move-assign
        /// overload, this does not update the cached peer address.
        /// 
        /// @param rhs - Data structure describing a polling request; only
        /// its `fd` member is used.
        /// @return udp_socket& - Reference to the left hand operand.
        ///
        inline udp_socket& operator=(pollfd&& rhs)
        {
            close();
            _data = rhs.fd;
            return *this;
        }

        ///
        /// @brief Move assignment operator, closing the previous handle
        /// then taking ownership of `rhs`'s handle and peer address.
        /// 
        /// @param rhs - The object which replaces the left hand one.
        /// @return udp_socket& - Reference to the left hand operand.
        ///
        inline udp_socket& operator=(udp_socket&& rhs)
        {
            close();
            _data = rhs._data;
            _addr_len = rhs._addr_len;
            memcpy(&_addr, &rhs._addr, sizeof(sockaddr_storage));
            rhs._data = static_cast<network_detail::native_socket_type>(-1);
            rhs._addr_len = 0;
            return *this;
        }

        ///
        /// @brief Checks if 2 sockets share the same cached peer address.
        /// 
        /// @param rhs - Socket to compare with.
        /// @return true - If both sockets' peer addresses are identical.
        /// @return false - If they differ.
        ///
        inline bool operator==(const udp_socket& rhs) const
        {
            return memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage)) == 0;
        }

        ///
        /// @brief Checks if 2 sockets have different cached peer addresses.
        /// 
        /// @param rhs - Socket to compare with.
        /// @return true - If the peer addresses differ.
        /// @return false - If they are identical.
        ///
        inline bool operator!=(const udp_socket& rhs) const
        {
            return memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage)) != 0;
        }

        ///
        /// @brief Closes the current socket handle if valid.
        ///
        inline void close()
        {
            if (is_valid() == true)
            {
                close_socket(_data);
                _data = static_cast<native_socket_type>(-1);
            }
        }

        ///
        /// @brief Handles the current socket based on the `info` provided,
        /// with separate logic for server (bind) and client (connect).
        /// 
        /// @param info - Address info used to bind/connect the socket.
        /// @param is_server - If the socket is a server or client.
        /// @return true - Socket bound/connected successfully.
        /// @return false - Failed, or called on an invalid socket.
        ///
        inline bool handle(const addrinfo* info, bool is_server)
        {
            if (is_valid() == false)
            {
                fputs("udp_socket::handle() called on invalid socket\n", stderr);
                return false;
            }
            else if (is_server == true)
            {
                return _handle_server(info);
            }
            else
            {
                return _handle_client(info);
            }
        }

        ///
        /// @brief Checks if a socket has a valid native handle.
        /// 
        /// @return true - The socket holds a valid file descriptor/handle.
        /// @return false - The socket handle is -1 (closed or never created).
        ///
        inline bool is_valid() const
        {
            return _data != static_cast<native_socket_type>(-1);
        }

        ///
        /// @brief Gets the address family this socket was instantiated for.
        /// 
        /// @return auto - `AF_INET`/`AF_INET6` (the underlying value of
        /// `Protocol`) as an `i32`.
        ///
        static constexpr auto ip_protocol()
        {
            return static_cast<i32>(Protocol);
        }

        ///
        /// @brief Gets the socket type used to create this socket.
        /// 
        /// @return auto - `SOCK_DGRAM` as an `i32`.
        ///
        static constexpr auto sock_type()
        {
            return static_cast<i32>(socket_type::dgram);
        }

        ///
        /// @brief Gets the transport protocol used to create this socket.
        /// 
        /// @return auto - `IPPROTO_UDP` as an `i32`.
        ///
        static constexpr auto net_protocol()
        {
            return static_cast<i32>(socket_protocol::udp);
        }

        ///
        /// @brief Gets the native socket handle.
        /// 
        /// @return auto - The native handle, or -1 if the socket is invalid.
        ///
        inline auto fd()
        {
            return _data;
        }

        ///
        /// @brief Gets the cached peer address (bound local or connected
        /// remote address, depending on how the socket was created).
        /// 
        /// @return auto& - Reference to the stored `sockaddr_storage`.
        ///
        inline auto& addr()
        {
            return _addr;
        }

        ///
        /// @brief Gets the length in bytes of the valid portion of `addr()`.
        /// 
        /// @return auto& - Reference to the stored address length.
        ///
        inline auto& length()
        {
            return _addr_len;
        }

        ///
        /// @brief Create a socket object.
        /// 
        /// @param addr - IPv4/IPv6 address used to create the socket.
        /// @param is_server - If the socket is a server or client.
        /// @return true - Socket created successfully.
        /// @return false - Failed to create the socket.
        ///
        inline bool create_socket(const char* addr, bool is_server = false)
        {
            return hsd::network_detail::create_socket(*this, addr, is_server);
        }

        ///
        /// @brief Sends a datagram to the cached peer address.
        /// 
        /// @param buffer - Buffer to send.
        /// @param size - Number of bytes to send.
        /// @return isize - Number of bytes sent, or -1 if invalid.
        ///
        inline isize send(const void* buffer, usize size)
        {
            if (is_valid() == false)
            {
                return -1;
            }
                
            #if defined(HSD_PLATFORM_WINDOWS)
            return ::sendto(
                _data, reinterpret_cast<const char*>(buffer), 
                static_cast<i32>(size), 0, 
                reinterpret_cast<const sockaddr*>(&_addr), _addr_len
            );
            #else
            return ::sendto(
                _data, buffer, size, 0, 
                reinterpret_cast<const sockaddr*>(&_addr), 
                _addr_len
            );
            #endif
        }

        ///
        /// @brief Receives a datagram, overwriting the cached peer address
        /// (`_addr`/`_addr_len`) with the sender's address.
        /// 
        /// @param buffer - Buffer to receive into.
        /// @param size - Maximum number of bytes to receive.
        /// @return isize - Number of bytes received, or -1 if invalid.
        ///
        inline isize receive(void* buffer, usize size)
        {
            if (is_valid() == false)
            {
                return -1;
            }

            #if defined(HSD_PLATFORM_WINDOWS)
            return ::recvfrom(
                _data, reinterpret_cast<char*>(buffer),
                static_cast<i32>(size), 0,
                reinterpret_cast<sockaddr*>(&_addr), &_addr_len
            );
            #else
            return ::recvfrom(
                _data, buffer, size, 0,
                reinterpret_cast<sockaddr*>(&_addr), &_addr_len
            );
            #endif
        }
    };

    ///
    /// @brief Represents a remote peer discovered by a `udp_server`. Does
    /// not own a native handle exclusively (`_data` is a copy of the
    /// server's shared bound socket, used only to send replies via
    /// `sendto()`); received datagrams are queued per-peer in
    /// `_recv_buffer` by the server's poll loop and drained by `receive()`.
    /// 
    /// @tparam Protocol - protocol type
    ///
    template <ip_protocol Protocol>
    struct udp_child_socket
    {
    private:
        /// Native handle shared with the owning server's main socket.
        native_socket_type _data;
        /// Cached remote address this peer sends to/receives from.
        sockaddr_storage _addr{};
        /// Length in bytes of the valid portion of `_addr`.
        socklen_t _addr_len{};

        /// Queue of datagrams received from this peer, awaiting `receive()`.
        list<static_vector<char, 1024>> _recv_buffer{};

    public:
        ///
        /// @brief Constructs an invalid udp_child_socket (used as the
        /// unordered_map's default-constructed placeholder).
        ///
        inline udp_child_socket() 
            : _data{static_cast<native_socket_type>(-1)}
        {}

        ///
        /// @brief Constructs a udp_child_socket for a newly discovered peer.
        /// 
        /// @param data - Native handle of the owning server's bound socket.
        /// @param info - Address of the remote peer.
        /// @param addr_len - Length in bytes of the valid portion of `info`.
        ///
        inline udp_child_socket(native_socket_type data,
            const sockaddr_storage& info, socklen_t addr_len)
            : _data{data}, _addr_len{addr_len}
        {
            memcpy(&_addr, &info, sizeof(sockaddr_storage));
        }

        ///
        /// @brief Constructs by move a new udp_child_socket object.
        /// 
        /// @param other - socket class to be moved.
        ///
        inline udp_child_socket(udp_child_socket&& other)
            : _data{other._data}, _addr_len{other._addr_len}, 
            _recv_buffer{hsd::move(other._recv_buffer)}
        {
            memcpy(&_addr, &other._addr, sizeof(sockaddr_storage));
        }

        ///
        /// @brief Move assignment operator.
        /// 
        /// @param rhs - The object which replaces the left hand one.
        /// @return udp_child_socket& - Reference to the left hand operand.
        ///
        inline udp_child_socket& operator=(udp_child_socket&& rhs)
        {
            _data = rhs._data;
            _addr_len = rhs._addr_len;
            memcpy(&_addr, &rhs._addr, sizeof(sockaddr_storage));
            _recv_buffer = hsd::move(rhs._recv_buffer);

            return *this;
        }

        ///
        /// @brief Checks if 2 peers share the same address and native handle.
        /// 
        /// @param rhs - Peer to compare with.
        /// @return true - If the address and handle are identical.
        /// @return false - If they differ.
        ///
        inline bool operator==(const udp_child_socket& rhs) const
        {
            auto _res = memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage));
            return _res == 0 && rhs._data == _data;
        }

        ///
        /// @brief Checks if 2 peers have a different address or native handle.
        /// 
        /// @param rhs - Peer to compare with.
        /// @return true - If they differ.
        /// @return false - If the address and handle are identical.
        ///
        inline bool operator!=(const udp_child_socket& rhs) const
        {
            return !(*this == rhs);
        }

        ///
        /// @brief Checks if this peer has a valid native handle.
        /// 
        /// @return true - The handle is valid.
        /// @return false - The handle is -1 (default-constructed placeholder).
        ///
        inline bool is_valid() const
        {
            return _data != static_cast<native_socket_type>(-1);
        }

        ///
        /// @brief Queues a received datagram for later retrieval by `receive()`.
        /// 
        /// @param buffer - Datagram bytes to enqueue (copied).
        ///
        inline void add(const static_vector<char, 1024>& buffer)
        {
            _recv_buffer.push_back(buffer);
        }

        ///
        /// @brief Sends a datagram to this peer's cached address.
        /// 
        /// @param buffer - Buffer to send.
        /// @param size - Number of bytes to send.
        /// @return isize - Number of bytes sent, or -1 if invalid.
        ///
        inline isize send(const void* buffer, usize size)
        {
            if (is_valid() == false)
            {
                return -1;
            }

            return ::sendto(
                _data, reinterpret_cast<const char*>(buffer),
                static_cast<i32>(size), 0,
                reinterpret_cast<const sockaddr*>(&_addr), _addr_len
            );
        }

        ///
        /// @brief Dequeues the oldest queued datagram into `buffer`.
        /// Pre-existing bug: this always copies exactly 1024 bytes from the
        /// stored `static_vector<char, 1024>`'s backing storage and reports
        /// 1024 as the number of bytes received, even when the original
        /// datagram (as resized in `udp_server::poll()`) was smaller; callers
        /// receive trailing bytes left over from the fixed-size buffer
        /// instead of the true datagram length.
        /// 
        /// @param buffer - Buffer to receive into; must be at least 1024 bytes.
        /// @param size - Capacity of `buffer`; must be >= 1024 or the call fails.
        /// @return isize - 1024 on success, 0 if no datagram is queued, or -1
        /// if invalid or `size` is less than 1024.
        ///
        inline isize receive(void* buffer, usize size)
        {
            if (is_valid() == false || size < 1024)
            {
                return -1;
            }
            else if (_recv_buffer.empty())
            {
                return 0;
            }

            auto _min_size = size < 1024 ? size : 1024;
            memcpy(buffer, _recv_buffer.front().data(), _min_size);
            _recv_buffer.pop_front();
            return _min_size;
        }
    };
} // namespace hsd::network_detail