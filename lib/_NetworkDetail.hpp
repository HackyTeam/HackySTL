#pragma once

#include "Types.hpp"
#include "_Define.hpp"
#include "String.hpp"
#include <stdio.h>

#if defined(HSD_PLATFORM_POSIX)
#include <poll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#else
#include <unistd.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#endif

namespace hsd
{
    namespace network_detail
    {
        #if defined(HSD_PLATFORM_WINDOWS)
        using native_socket_type = SOCKET;
        #else
        using native_socket_type = i32;
        #endif

        enum class socket_protocol
        {
            tcp = IPPROTO_TCP,
            udp = IPPROTO_UDP
        };

        enum class ip_protocol
        {
            ipv4 = AF_INET,
            ipv6 = AF_INET6,
            unspec = AF_UNSPEC
        };

        #if defined(HSD_PLATFORM_WINDOWS)
        enum class socket_type
        {
            stream = SOCK_STREAM,
            dgram = SOCK_DGRAM,
            raw = SOCK_RAW,
            rdm = SOCK_RDM,
            seq_packet = SOCK_SEQPACKET
        };
        #else
        enum class socket_type
        {
            stream = SOCK_STREAM,
            clo_exec = SOCK_CLOEXEC,
            dccp = SOCK_DCCP,
            dgram = SOCK_DGRAM,
            no_block = SOCK_NONBLOCK,
            packet = SOCK_PACKET,
            raw = SOCK_RAW,
            rdm = SOCK_RDM,
            seq_packet = SOCK_SEQPACKET
        };
        #endif
    } // namespace network_detail
} // namespace hsd