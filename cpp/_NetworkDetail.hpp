#pragma once

#include "SStream.hpp"
#include "Io.hpp"

#ifdef __linux__

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <memory.h>

namespace hsd
{
    namespace tcp
    {
        enum class received_state
        {
            err = -1,
            disconnected,
            ok
        };

        enum class network_type
        {
            server,
            client
        };

        enum class protocol_type
        {
            ipv4 = AF_INET,
            ipv6 = AF_INET6
        };

        struct socket_type
        {
            static constexpr size_t stream = SOCK_STREAM;
            static constexpr size_t clo_exec = SOCK_CLOEXEC;
            static constexpr size_t dccp = SOCK_DCCP;
            static constexpr size_t dgram = SOCK_DGRAM;
            static constexpr size_t no_block = SOCK_NONBLOCK;
            static constexpr size_t packet = SOCK_PACKET;
            static constexpr size_t raw = SOCK_RAW;
            static constexpr size_t rdm = SOCK_RDM;
            static constexpr size_t seq_packet = SOCK_SEQPACKET;
        };
    } // namespace tcp
} // namespace hsd

#endif