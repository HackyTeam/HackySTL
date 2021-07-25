#include "_NetworkFuncs.hpp"

namespace hsd::network_detail
{
    template <ip_protocol Protocol>
    class tcp_socket
    {
    private:
        pollfd _data;

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
                
                if (::bind(_data.fd, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

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
                    perror("setsockopt() failed");
                    close(); abort();
                }
            
                #if defined(HSD_PLATFORM_WINDOWS)
                _rc = ioctlsocket(_data.fd, FIONBIO, &_on);
                #else
                _rc = ioctl(_data.fd, FIONBIO, &_on);
                #endif

                if (_rc < 0)
                {
                    perror("ioctl() failed");
                    close(); abort();
                }

                if (::listen(_data.fd, SOMAXCONN) == -1)
                {
                    return false;
                }

                return true;
            }

            return false;
        }

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
        inline tcp_socket(const tcp_socket& other) = delete;
        inline tcp_socket& operator=(const tcp_socket& other) = delete;
        
        inline tcp_socket(pollfd&& data)
            : _data(data)
        {}

        inline tcp_socket(tcp_socket&& other)
            : _data(move(other._data))
        {
            other._data.fd = static_cast<network_detail::native_socket_type>(-1);
        }
    
        inline tcp_socket(const char* const addr, bool is_server = false)
        {
            _data = pollfd {
                .fd = static_cast<network_detail::native_socket_type>(-1),
                .events = 0,
                .revents = 0
            };

            if (switch_to(addr, is_server) == false)
            {
                fputs("hsd::network_detail::tcp_socket::switch_to() failed.\n", stderr);
                close();
            }
        }

        inline ~tcp_socket()
        {
            close();
        }

        inline tcp_socket& operator=(tcp_socket&& other)
        {
            close();
            _data = other._data;
            other.close();
            return *this;
        }

        inline tcp_socket& operator=(pollfd&& rhs)
        {
            close();
            _data = rhs;
            return *this;
        }

        inline bool operator==(const tcp_socket& rhs) const
        {
            return _data.fd == rhs._data.fd;
        }

        inline bool operator!=(const tcp_socket& rhs) const
        {
            return _data.fd != rhs._data.fd;
        }

        inline void close()
        {
            if (is_valid() == true)
            {
                close_socket(_data.fd);
                _data.fd = static_cast<native_socket_type>(-1);
                _data.events = _data.revents = 0;
            }
        }

        inline bool handle(const addrinfo* const info, bool is_server)
        {
            if (is_valid() == false)
            {
                fputs("tcp_socket::handle() called on invalid socket\n", stderr);
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

        [[nodiscard]] inline bool switch_to(const char* const addr, bool is_server = false)
        {
            return hsd::network_detail::switch_to(*this, addr, is_server);
        }

        inline bool is_valid() const
        {
            return _data.fd != static_cast<native_socket_type>(-1);
        }

        inline bool is_readable() const
        {
            return _data.revents & POLLIN;
        }

        inline bool is_writable() const
        {
            return _data.revents & POLLOUT;
        }

        static consteval auto ip_protocol()
        {
            return static_cast<i32>(Protocol);
        }

        static consteval auto sock_type()
        {
            return static_cast<i32>(socket_type::stream);
        }

        static consteval auto net_protocol()
        {
            return static_cast<i32>(socket_protocol::tcp);
        }

        inline const auto& fd()
        {
            return _data.fd;
        }

        inline isize send(const void* const buffer, const usize size) const
        {
            if (is_valid() == false || size > 65635)
            {
                return -1;
            }
            
            #if defined(HSD_PLATFORM_WINDOWS)
            return ::send(
                _data.fd, reinterpret_cast<const char* const>(buffer), size, 0
            );
            #else
            return ::send(_data.fd, buffer, size, 0);
            #endif
        }

        inline isize receive(void* const buffer, const usize size) const
        {
            if (is_valid() == false)
            {
                return -1;
            }
            else if (is_readable() == true)
            {
                #if defined(HSD_PLATFORM_WINDOWS)
                return ::recv(
                    _data.fd, reinterpret_cast<char* const>(buffer), size, 0
                );
                #else
                return ::recv(_data.fd, buffer, size, 0);
                #endif
            }

            return 0;
        }
    };

    template <ip_protocol Protocol>
    class udp_socket
    {
    private:
        pollfd _data;

        inline bool _handle_server(const addrinfo* const info)
        {
            if (info->ai_socktype == SOCK_DGRAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }
                
                _data.events = POLLIN;
                _data.revents = 0;

                if (::bind(_data.fd, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                return true;
            }

            return false;
        }

        inline bool _handle_client(const addrinfo* const info)
        {
            if (info->ai_socktype == SOCK_DGRAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }

                _data.events = POLLOUT;
                _data.revents = 0;

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
        inline udp_socket(const udp_socket& other) = delete;
        inline udp_socket& operator=(const udp_socket& other) = delete;

        inline udp_socket(pollfd&& data)
            : _data{data}
        {}

        inline udp_socket(udp_socket&& other)
            : _data{move(other._data)}
        {
            other._data.fd = static_cast<network_detail::native_socket_type>(-1);
        }

        inline ~udp_socket()
        {
            close();
        }

        inline udp_socket& operator=(udp_socket&& other)
        {
            close();
            _data = other._data;
            other.close();
            return *this;
        }

        inline udp_socket& operator=(pollfd&& rhs)
        {
            close();
            _data = rhs;
            return *this;
        }

        inline bool operator==(const udp_socket& rhs) const
        {
            return _data.fd == rhs._data.fd;
        }

        inline bool operator!=(const udp_socket& rhs) const
        {
            return _data.fd != rhs._data.fd;
        }

        inline void close()
        {
            if (is_valid() == true)
            {
                close_socket(_data.fd);
                _data.fd = static_cast<native_socket_type>(-1);
                _data.events = _data.revents = 0;
            }
        }

        inline bool handle(const addrinfo* const info, bool is_server)
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

        inline bool is_valid() const
        {
            return _data.fd != static_cast<native_socket_type>(-1);
        }

        inline bool is_readable() const
        {
            return _data.revents & POLLIN;
        }

        inline bool is_writable() const
        {
            return _data.revents & POLLOUT;
        }

        static consteval auto ip_protocol()
        {
            return static_cast<i32>(Protocol);
        }

        static consteval auto sock_type()
        {
            return static_cast<i32>(socket_type::dgram);
        }

        static consteval auto net_protocol()
        {
            return static_cast<i32>(socket_protocol::udp);
        }

        inline const auto& fd()
        {
            return _data.fd;
        }

        inline bool switch_to(const char* const addr, bool is_server = false)
        {
            return hsd::network_detail::switch_to(*this, addr, is_server);
        }

        inline isize send(const void* const buffer, const usize size) const
        {
            if (is_valid() == false)
            {
                return -1;
            }
                
            return ::sendto(_data.fd, buffer, size, 0, nullptr, 0);
        }

        inline isize receive(void* const buffer, const usize size) const
        {
            if (is_valid() == false)
            {
                return -1;
            }
            else if (is_readable() == true)
            {
                return ::recvfrom(_data.fd, buffer, size, 0, nullptr, nullptr);
            }
            
            return 0;
        }
    };
} // namespace hsd::network_detail