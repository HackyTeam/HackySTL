#pragma once

/// @file _NetworkDetail.hpp
/// @brief Internal network protocol, address, and native-handle definitions.
/// @details Provides platform-neutral enums and socket metadata shared by the
/// public network wrappers and low-level socket implementation.

#include "../Base/Types.hpp"

#if defined(HSD_PLATFORM_WINDOWS)
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>
#endif

///
/// @brief Namespace for Network implementation details.
///
namespace hsd::network_detail
{
    #if defined(HSD_PLATFORM_WINDOWS)
    ///
    /// @brief Internal handle for the socket.
    ///
    using native_socket_type = SOCKET;

    #else
    ///
    /// @brief Internal handle for the socket.
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