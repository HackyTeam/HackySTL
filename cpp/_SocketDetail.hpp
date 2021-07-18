#include "_NetworkDetail.hpp"

namespace hsd
{
    namespace socket_detail
    {
        template <net::socket_type SocketType>
        struct socket_exposable
        {
            virtual ~socket_exposable() {}

            virtual inline bool is_valid() const = 0;
            virtual inline bool is_invalid() const = 0;
            virtual inline void close() = 0;
        };

        template <>
        struct socket_exposable<net::socket_type::multi_server>
            : public socket_exposable<net::socket_type::direct_server>
        {
            virtual ~socket_exposable() {}

            virtual inline usize get_size() const = 0;
            virtual inline bool is_empty() const = 0;
            virtual inline bool check_current_index() = 0;
            virtual inline void advance() = 0;
            virtual inline i32 poll(i32 timeout_ms) = 0;
            virtual inline bool check_and_add_socket() = 0;
        };

        template <net::socket_type SocketType>
        class socket_base
            : public socket_exposable<SocketType>
        {
        private:
            using sock_value_t = tuple<net::native_socket_type, sockaddr_storage, socklen_t>;
            static constexpr net::native_socket_type invalid_socket = -1;
            static constexpr socklen_t invalid_addr_len = 0;
            static constexpr sockaddr_storage invalid_addr = {};

            sock_value_t _sock_info;
            
        public:
            inline socket_base()
                : _sock_info{invalid_socket, invalid_addr, invalid_addr_len}
            {}

            inline ~socket_base()
            {
                close();
            }

            inline socket_base(const socket_base&) = delete;
            inline socket_base& operator=(const socket_base&) = delete; 

            inline socket_base(socket_base&& other)
                : _sock_info{move(other)}
            {}

            inline socket_base& operator=(socket_base&& rhs)
            {
                _sock_info = move(rhs._sock_info);
                return *this;
            }

            virtual inline bool is_valid() const override
            {
                return _sock_info.get<0>() != invalid_socket;
            }

            virtual inline bool is_invalid() const override
            {
                return _sock_info.get<0>() == invalid_socket;
            }

            virtual inline void close() override
            {
                if (is_valid())
                {
                    network_detail::close_socket(_sock_info.get<0>());
                    _sock_info = hsd::make_tuple(
                        invalid_socket, invalid_addr, invalid_addr_len
                    );
                }
            }

            static inline auto get_invalid_socket()
            {
                return invalid_socket;
            }
            
            inline auto& get_socket()
            {
                return _sock_info.get<0>();
            }

            inline auto* get_addr()
            {
                return reinterpret_cast<sockaddr*>(&_sock_info.get<1>());
            }
            
            inline auto* get_addr_len()
            {
                return &_sock_info.get<2>();
            }
        };

        template <>
        class socket_base<net::socket_type::multi_server>
            : public socket_exposable<net::socket_type::multi_server>
        {
        private:
            static constexpr net::native_socket_type invalid_socket = -1;
            socklen_t _addr_len = sizeof(sockaddr_storage);
            sockaddr_storage _addr = {};
            
            net::native_socket_type _server_sock = invalid_socket;
            vector<pollfd> _sock_infos;
            usize _current_index = 0;

        public:
            inline socket_base()
            {
                _sock_infos.push_back(pollfd {
                    .fd = invalid_socket, 
                    .events = POLLIN, 
                    .revents = 0
                });
            }

            inline ~socket_base()
            {
                for (auto& _sock_info : _sock_infos)
                {
                    if (_sock_info.fd != invalid_socket)
                        network_detail::close_socket(_sock_info.fd);
                }
            }

            inline bool is_valid() const
            {
                return _sock_infos[_current_index].fd != invalid_socket;
            }

            inline bool is_invalid() const
            {
                return _sock_infos[_current_index].fd == invalid_socket;
            }

            virtual inline void close() override
            {
                if (is_valid())
                {
                    network_detail::close_socket(_sock_infos[_current_index].fd);
                    _sock_infos.erase(_sock_infos.begin() + _current_index).unwrap();
                }
            }

            static inline auto get_invalid_socket()
            {
                return invalid_socket;
            }

            inline auto& get_server_socket()
            {
                return _server_sock;
            }

            inline auto& get_socket()
            {
                return _sock_infos[_current_index].fd;
            }

            inline auto* get_addr()
            {
                return reinterpret_cast<sockaddr*>(&_addr);
            }

            inline auto* get_addr_len()
            {
                return &_addr_len;
            }

            virtual inline usize get_size() const override
            {
                return _sock_infos.size();
            }

            virtual inline bool is_empty() const
            {
                return _sock_infos.size() == 0;
            }

            virtual inline bool check_current_index() override
            {
                if (_current_index >= _sock_infos.size())
                {
                    _current_index = 0;
                    return false;
                }

                return true;
            }

            virtual inline void advance() override
            {
                ++_current_index;
            }

            virtual inline i32 poll(i32 timeout_ms) override
            {
                return ::poll(_sock_infos.data(), _sock_infos.size(), timeout_ms);
            }

            virtual inline bool check_and_add_socket() override
            {
                if (_sock_infos[_current_index].fd == _server_sock)
                {
                    auto _new_sock = accept(_server_sock, nullptr, nullptr);

                    if (_new_sock != invalid_socket)
                    {
                        _sock_infos.push_back(pollfd {
                            .fd = _new_sock, 
                            .events = POLLIN,
                            .revents = 0
                        });
                    }

                    return true;
                }

                return false;
            }
        };  
    } // namespace socket_detail
} // namespace hsd
