// Title: TCP echo on loopback
// Level: Basic
// Shows: tcp_server_v4 and tcp_client_v4 in one process, an ephemeral port, and a server thread joined before printing.

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

#include <cstdio>

using namespace hsd::format_literals;

// Bind a throw-away socket to port 0 so the system picks a free port, read the
// port back, then close the socket so the real server can take that port.
static hsd::u32 pick_free_port()
{
#if defined(HSD_PLATFORM_WINDOWS)
    hsd::network_detail::init_winsock();
#endif

    hsd::network_detail::tcp_socket<hsd::network_detail::ip_protocol::ipv4> probe{
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

    // Creating the server binds and listens, so a client may connect right away.
    hsd::tcp_server_v4 server = address;
    constexpr hsd::usize message_count = 3;
    hsd::usize echoed = 0;

    // The server never prints: it only counts, and main reports after join().
    hsd::thread server_thread{[&]
    {
        while (echoed < message_count)
        {
            // poll() accepts new connections and refreshes what each socket can read.
            server.poll(20);

            for (auto socket = server.begin(); socket != server.end(); ++socket)
            {
                char buffer[64]{};
                auto received = socket->receive(buffer, sizeof(buffer) - 1);

                if (received > 0)
                {
                    socket->send(buffer, static_cast<hsd::usize>(received));
                    echoed++;
                }
            }
        }
    }};

    hsd::tcp_client_v4 client = address;
    const char* messages[message_count] = {"hello", "from", "loopback"};
    char replies[message_count][64]{};
    bool matched[message_count]{};

    for (hsd::usize i = 0; i < message_count; i++)
    {
        auto length = hsd::cstring::length(messages[i]);
        client.send(messages[i], length);

        // The client socket blocks until the echo arrives.
        client.receive(replies[i], sizeof(replies[i]) - 1);
        matched[i] = hsd::cstring::compare(messages[i], replies[i]) == 0;
    }

    server_thread.join().unwrap();

    for (hsd::usize i = 0; i < message_count; i++)
    {
        hsd::println("sent {} -> echoed {} ({})"_fmt,
            messages[i], replies[i], matched[i] ? "match" : "differs");
    }

    hsd::println("server echoed {} messages"_fmt, echoed);
    return 0;
}
