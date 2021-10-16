#pragma once

#include "_NetworkDetail.hpp"

namespace hsd::network_detail
{
    #if defined(HSD_PLATFORM_WINDOWS)
    static inline void init_winsock()
    {
        static WSAData _data;
        static WORD _ver = MAKEWORD(2, 2);
        static i32 _ws_result = WSAStartup(_ver, &_data);

        if (_ws_result != 0)
        {
            printf(
                "Can't start Winsock, here's"
                " a irrelevant number #%d\n",
                _ws_result
            );
            
            return;
        }
    }
    #endif

    static inline void close_socket(native_socket_type& socket)
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        ::closesocket(socket);
        #else
        ::close(socket);
        #endif

        socket = static_cast<native_socket_type>(-1);
    }

    bool get_addr_info(
        const char* name, const char* service,
        const addrinfo* hints, addrinfo** result)
    {
        i32 _error_code = getaddrinfo(
            name, service, hints, result
        );

        if (_error_code != 0) 
        {
            fprintf(
                stderr, "Error for getaddrinfo, code"
                ": %s\n", gai_strerror(_error_code)
            );

            return false;
        }

        return true;
    }

    static inline const char* error_message()
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        static char _msg_buf[256]{};
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, WSAGetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
            _msg_buf, sizeof(_msg_buf), nullptr
        );
        
        return _msg_buf;
        #else
        return strerror(errno);
        #endif
    }

    static inline auto error_code()
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        return WSAGetLastError();
        #else
        return errno;
        #endif
    }

    inline auto to_str(const sockaddr_storage* addr)
    {
        if (addr == nullptr)
        {
            hsd_fputs_check(
                stderr, "Error at to_str, addr is nullptr"
            );

            abort();
        }

        if (addr->ss_family == AF_INET)
        {
            auto* _addr = reinterpret_cast<const sockaddr_in*>(addr);
            static_string<18> _ip_str;
            
            _ip_str.push_back(_addr->sin_addr.s_addr & 0xff);
            _ip_str.push_back((_addr->sin_addr.s_addr >> 8) & 0xff);
            _ip_str.push_back((_addr->sin_addr.s_addr >> 16) & 0xff);
            _ip_str.push_back((_addr->sin_addr.s_addr >> 24) & 0xff);
            _ip_str.push_back(_addr->sin_port & 0xff);
            _ip_str.push_back((_addr->sin_port >> 8) & 0xff);

            return _ip_str;
        }
        else if (addr->ss_family == AF_INET6)
        {
            auto* _addr = reinterpret_cast<const sockaddr_in6*>(addr);
            static_string<18> _ip_str;
            
            for (i32 _index = 0; _index < 16; _index++)
                _ip_str.push_back(_addr->sin6_addr.s6_addr[_index]);

            _ip_str.push_back(_addr->sin6_port & 0xff);
            _ip_str.push_back((_addr->sin6_port >> 8) & 0xff);
            
            return _ip_str;
        }
        else
        {
            hsd_fputs_check(
                stderr, "Error at to_str, unknown family"
            );

            abort();
        }
    }

    template <typename SocketType>
    static inline bool switch_to(
        SocketType& new_socket, const char* ip_addr, bool is_server)
    {
        addrinfo* _result = nullptr;
        addrinfo* _rp = nullptr;

        auto _hints = addrinfo
        {
            .ai_flags = AI_PASSIVE,
            .ai_family = new_socket.ip_protocol(),
            .ai_socktype = new_socket.sock_type(),
            .ai_protocol = new_socket.net_protocol(),
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

        static string _domain_addr{};
        auto* _port = cstring::find_rev(ip_addr, ':');

        if (_port != nullptr)
        {
            _domain_addr = string_view {
                ip_addr, static_cast<usize>(_port - ip_addr)
            };
            
            _port += 1;

            if (get_addr_info(_domain_addr.c_str(), _port, &_hints, &_result) == false)
            {
                return false;
            }
        }
        else
        {
            if (get_addr_info(ip_addr, nullptr, &_hints, &_result) == false)
            {
                return false;
            }
        }
        for (_rp = _result; _rp != nullptr; _rp = _rp->ai_next)
        {
            new_socket = pollfd {
                .fd = ::socket(
                    _rp->ai_family, _rp->ai_socktype, _rp->ai_protocol
                ),
                .events = POLLIN | POLLOUT,
                .revents = 0
            };

            if (new_socket.is_valid() == true)
            {
                if (new_socket.handle(_rp, is_server))
                    break;

                new_socket.close();
            }
        }

        bool _expr_res = _rp != nullptr;
        freeaddrinfo(_result);
        return _expr_res; 
    }
} // namespace hsd::network_detail
