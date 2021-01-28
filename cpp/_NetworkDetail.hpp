#pragma once

#include "Io.hpp"
#include "_Define.hpp"

#ifdef HSD_PLATFORM_POSIX

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>

namespace hsd
{
    namespace net
    {
        enum class received_state
        {
            err = -1,
            disconnected,
            ok
        };

        enum class protocol_type
        {
            ipv4 = AF_INET,
            ipv6 = AF_INET6
        };

        struct socket_type
        {
            static constexpr usize stream = SOCK_STREAM;
            static constexpr usize clo_exec = SOCK_CLOEXEC;
            static constexpr usize dccp = SOCK_DCCP;
            static constexpr usize dgram = SOCK_DGRAM;
            static constexpr usize no_block = SOCK_NONBLOCK;
            static constexpr usize packet = SOCK_PACKET;
            static constexpr usize raw = SOCK_RAW;
            static constexpr usize rdm = SOCK_RDM;
            static constexpr usize seq_packet = SOCK_SEQPACKET;
        };
    } // namespace net
} // namespace hsd

#endif