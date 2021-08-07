#include "_NetworkFuncs.hpp"
#include "List.hpp"
#include "Vector.hpp"

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
            other._data.fd = static_cast<network_detail::native_socket_type>(-1);
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

        inline auto fd() const
        {
            return _data.fd;
        }

        inline isize send(const void* const buffer, const usize size)
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

        inline isize receive(void* const buffer, const usize size)
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
    class tcp_child_socket
        : private tcp_socket<Protocol>
    {
    public:
        using tcp_socket<Protocol>::tcp_socket;

        inline tcp_child_socket(pollfd&& data)
            : tcp_socket<Protocol>(move(data))
        {}

        inline tcp_child_socket(tcp_child_socket&& other)
            : tcp_socket<Protocol>(move(other))
        {}

        inline tcp_child_socket(const char* const addr, bool is_server = false)
            : tcp_socket<Protocol>(addr, is_server)
        {}

        inline tcp_child_socket& operator=(tcp_child_socket&& other)
        {
            static_cast<tcp_socket<Protocol>&>(*this) = move(other);
            return *this;
        }

        inline tcp_child_socket& operator=(pollfd&& rhs)
        {
            static_cast<tcp_socket<Protocol>&>(*this) = move(rhs);
            return *this;
        }

        using tcp_socket<Protocol>::is_valid;
        using tcp_socket<Protocol>::is_readable;
        using tcp_socket<Protocol>::is_writable;
        using tcp_socket<Protocol>::fd;
        using tcp_socket<Protocol>::ip_protocol;
        using tcp_socket<Protocol>::sock_type;
        using tcp_socket<Protocol>::net_protocol;
        using tcp_socket<Protocol>::send;
        using tcp_socket<Protocol>::receive;
    };

    template <ip_protocol Protocol>
    class udp_socket
    {
    private:
        native_socket_type _data;
        sockaddr_storage _addr;
        socklen_t _addr_len;

        inline bool _handle_server(const addrinfo* const info)
        {
            if (info->ai_socktype == SOCK_DGRAM)
            {
                if (is_valid() == false)
                {
                    return false;
                }

                if (::bind(_data, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                memcpy(&_addr, info->ai_addr, sizeof(sockaddr_storage));
                _addr_len = info->ai_addrlen;

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
                    perror("setsockopt() failed");
                    close(); abort();
                }
            
                #if defined(HSD_PLATFORM_WINDOWS)
                _rc = ioctlsocket(_data, FIONBIO, &_on);
                #else
                _rc = ioctl(_data, FIONBIO, &_on);
                #endif

                if (_rc < 0)
                {
                    perror("ioctl() failed");
                    close(); abort();
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

                if (::connect(_data, info->ai_addr, info->ai_addrlen) == -1)
                {
                    close();
                    return false;
                }

                memcpy(&_addr, info->ai_addr, sizeof(sockaddr_storage));
                _addr_len = info->ai_addrlen;
                return true;
            }

            return false;
        }

    public:
        inline udp_socket(const udp_socket& other) = delete;
        inline udp_socket& operator=(const udp_socket& other) = delete;

        inline udp_socket(const char* const addr, bool is_server = false)
        {
            _data = static_cast<network_detail::native_socket_type>(-1);

            if (switch_to(addr, is_server) == false)
            {
                fputs("hsd::network_detail::udp_socket::switch_to() failed.\n", stderr);
                close();
            }
        }

        inline udp_socket(native_socket_type data, 
            const sockaddr_storage& info, socklen_t addr_len)
            : _data{data}, _addr{info}, _addr_len{addr_len}
        {}

        inline udp_socket(udp_socket&& other)
            : _data{other._data}, _addr{move(other._addr)}, _addr_len{other._addr_len}
        {
            other._addr_len = 0;
            other._data = static_cast<network_detail::native_socket_type>(-1);
        }

        inline ~udp_socket()
        {
            close();
        }

        inline udp_socket& operator=(pollfd&& rhs)
        {
            close();
            _data = rhs.fd;
            return *this;
        }

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

        inline bool operator==(const udp_socket& rhs) const
        {
            return memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage)) == 0;
        }

        inline bool operator!=(const udp_socket& rhs) const
        {
            return memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage)) != 0;
        }

        inline void close()
        {
            if (is_valid() == true)
            {
                close_socket(_data);
                _data = static_cast<native_socket_type>(-1);
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
            return _data != static_cast<native_socket_type>(-1);
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

        inline auto fd()
        {
            return _data;
        }

        inline auto& addr()
        {
            return _addr;
        }

        inline auto& length()
        {
            return _addr_len;
        }

        inline bool switch_to(const char* const addr, bool is_server = false)
        {
            return hsd::network_detail::switch_to(*this, addr, is_server);
        }

        inline isize send(const void* const buffer, const usize size)
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
                reinterpret_cast<sockaddr*>(&_addr), 
                _addr_len
            );
            #endif
        }

        inline isize receive(void* const buffer, const usize size)
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

    template <ip_protocol Protocol>
    struct udp_child_socket
    {
    private:
        native_socket_type _data;
        sockaddr_storage _addr{};
        socklen_t _addr_len{};

        list<static_vector<char, 1024>> _recv_buffer{};

    public:
        inline udp_child_socket() 
            : _data{static_cast<native_socket_type>(-1)}
        {}

        inline udp_child_socket(native_socket_type data,
            const sockaddr_storage& info, socklen_t addr_len)
            : _data{data}, _addr_len{addr_len}
        {
            memcpy(&_addr, &info, sizeof(sockaddr_storage));
        }

        inline udp_child_socket(udp_child_socket&& other)
            : _data{other._data}, _addr_len{other._addr_len}, 
            _recv_buffer{move(other._recv_buffer)}
        {
            memcpy(&_addr, &other._addr, sizeof(sockaddr_storage));
        }

        inline udp_child_socket& operator=(udp_child_socket&& rhs)
        {
            _data = rhs._data;
            _addr_len = rhs._addr_len;
            memcpy(&_addr, &rhs._addr, sizeof(sockaddr_storage));
            _recv_buffer = move(rhs._recv_buffer);

            return *this;
        }

        inline bool operator==(const udp_child_socket& rhs) const
        {
            auto _res = memcmp(&_addr, &rhs._addr, sizeof(sockaddr_storage));
            return _res == 0 && rhs._data == _data;
        }

        inline bool operator!=(const udp_child_socket& rhs) const
        {
            return !(*this == rhs);
        }

        inline bool is_valid() const
        {
            return _data != static_cast<native_socket_type>(-1);
        }

        inline void add(const static_vector<char, 1024>& buffer)
        {
            _recv_buffer.push_back(buffer);
        }

        inline isize send(const void* const buffer, const usize size)
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

        inline isize receive(void* const buffer, const usize size)
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