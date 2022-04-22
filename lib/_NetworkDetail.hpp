#pragma once

#include "Types.hpp"
#include "_Define.hpp"
#include "String.hpp"
#include <stdio.h>
#include <unistd.h>

#if defined(HSD_PLATFORM_WINDOWS)
#include <ws2tcpip.h>
#include <winsock2.h>
#else
#include <poll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#endif

///
/// @brief Namespace for Network implementation details.
///
namespace hsd::network_detail
{
    #if defined(HSD_PLATFORM_WINDOWS)
    ///
    /// @brief Internal alias for the windows socket handle.
    ///
    using native_socket_type = SOCKET;
    #else
    ///
    /// @brief Internal alias for the unix socket handle.
    ///
    using native_socket_type = i32;
    #endif

    ///
    /// @brief Enumeration for the different socket protocols.
    ///
    enum class socket_protocol
    {
        tcp = IPPROTO_TCP, ///< TCP protocol.
        udp = IPPROTO_UDP  ///< UDP protocol.
    };
    ///
    /// @brief Enumeration for the different address families.
    ///
    enum class ip_protocol
    {
        ipv4   = AF_INET,  ///< IPv4 protocol.
        ipv6   = AF_INET6, ///< IPv6 protocol.
        unspec = AF_UNSPEC ///< Unspecified protocol.
    };
    #if defined(HSD_PLATFORM_WINDOWS)
    ///
    /// @brief Enumeration for the different socket types, used for windows.
    ///
    enum class socket_type
    {
        stream     = SOCK_STREAM,   ///< Stream socket.
        dgram      = SOCK_DGRAM,    ///< Datagram socket.
        raw        = SOCK_RAW,      ///< Raw socket.
        rdm        = SOCK_RDM,      ///< Reliable message datagram socket.
        seq_packet = SOCK_SEQPACKET ///< Sequenced packet socket.
    };
    #else
    ///
    /// @brief Enumeration for the different socket types, used for POSIX.
    ///
    enum class socket_type
    {
        stream     = SOCK_STREAM,   ///< Stream socket.
        clo_exec   = SOCK_CLOEXEC,  ///< Close on exec socket.
        dccp       = SOCK_DCCP,     ///< Datagram Congestion Control Protocol socket.
        dgram      = SOCK_DGRAM,    ///< Datagram socket.
        no_block   = SOCK_NONBLOCK, ///< Non-blocking socket.
        packet     = SOCK_PACKET,   ///< Packet socket.
        raw        = SOCK_RAW,      ///< Raw socket.
        rdm        = SOCK_RDM,      ///< Reliable message datagram socket.
        seq_packet = SOCK_SEQPACKET ///< Sequenced packet socket.
    };
    #endif
} // namespace hsd::network_detail