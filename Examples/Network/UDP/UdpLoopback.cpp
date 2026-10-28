// Title: UDP loopback
// Level: Basic
// Shows: udp_server_v4 and udp_client_v4 in one thread, an ephemeral port, and one reply per datagram.

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

#include <cstdio>

using namespace hsd::format_literals;

// Bind a throw-away socket to port 0 so the system picks a free port, read the
// port back, then close it so the real server can take that port.
static hsd::u32 pick_free_port()
{
#if defined(HSD_PLATFORM_WINDOWS)
    hsd::network_detail::init_winsock();
#endif

    hsd::network_detail::udp_socket<hsd::network_detail::ip_protocol::ipv4> probe{
        "127.0.0.1:0", true
    };

    sockaddr_in local{};
#if defined(HSD_PLATFORM_WINDOWS)
    int length = sizeof(local);
#else
    socklen_t length = sizeof(local);
#endif
    ::getsockname(probe.fd(), reinterpret_cast<sockaddr*>(&local), &length);
    return ntohs(local.sin_port);
}

int main()
{
    char address[32]{};
    std::snprintf(address, sizeof(address), "127.0.0.1:%u", pick_free_port());

    hsd::udp_server_v4 server = address;
    hsd::udp_client_v4 client = address;

    const char* messages[] = {"alpha", "beta", "gamma"};

    for (const char* message : messages)
    {
        // Each send() is one datagram. The kernel keeps it until the server reads it.
        client.send(message, hsd::cstring::length(message));

        char request[1024]{};
        bool received = false;

        // The UDP server is non-blocking: poll() takes at most one datagram,
        // sorts it under the sender's address, and returns immediately.
        for (int attempt = 0; attempt < 2000 && !received; attempt++)
        {
            server.poll();

            for (auto& [sender, socket] : server)
            {
                // Every child socket buffers its datagrams and hands them out
                // through a 1024-byte receive.
                if (socket.receive(request, sizeof(request)) > 0)
                {
                    char reply[64]{};
                    std::snprintf(reply, sizeof(reply), "got %zu bytes", hsd::cstring::length(request));
                    socket.send(reply, hsd::cstring::length(reply) + 1);
                    received = true;
                }
            }

            if (!received)
                hsd::this_thread::sleep_for(0.001f).unwrap();
        }

        char answer[64]{};
        client.receive(answer, sizeof(answer) - 1);
        hsd::println("{} -> server saw '{}' -> client got '{}'"_fmt, message, request, answer);
    }

    return 0;
}
