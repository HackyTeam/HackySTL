// Title: TCP server with several clients
// Level: Advanced
// Shows: one polling server thread serving three client threads at once, erasing finished sockets, and join-then-print reporting.

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>
#include <Containers/Vector.hpp>

#include <cstdio>

using namespace hsd::format_literals;

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

constexpr hsd::usize client_count = 3;

// What one client thread saw. Each thread writes only its own slot.
struct client_report
{
    char first_reply[64]{};
    char second_reply[64]{};
    char goodbye[64]{};
};

int main()
{
    char address[32]{};
    std::snprintf(address, sizeof(address), "127.0.0.1:%u", pick_free_port());

    hsd::tcp_server_v4 server = address;

    // Per-client request counts, indexed by the id that starts every message.
    hsd::usize requests[client_count]{};
    hsd::usize finished = 0;

    hsd::thread server_thread{[&]
    {
        while (finished < client_count)
        {
            server.poll(20);

            for (auto socket = server.begin(); socket != server.end();)
            {
                // A message looks like "<id>:<text>". The reply is "<id>:<TEXT> (<length>)".
                char buffer[64]{};
                auto received = socket->receive(buffer, sizeof(buffer) - 1);

                if (received <= 0)
                {
                    ++socket;
                    continue;
                }

                hsd::usize id = static_cast<hsd::usize>(buffer[0] - '0');
                const char* text = buffer + 2;
                hsd::usize text_length = hsd::cstring::length(text);

                if (hsd::cstring::compare(text, "bye") == 0)
                {
                    char goodbye[16]{};
                    std::snprintf(goodbye, sizeof(goodbye), "%zu:bye", id);
                    socket->send(goodbye, hsd::cstring::length(goodbye));

                    // Done with this client: erase() closes the socket and hands back
                    // the next valid position.
                    socket = server.erase(socket).unwrap();
                    finished++;
                    continue;
                }

                char reply[64]{};
                std::snprintf(reply, sizeof(reply), "%zu:", id);

                for (hsd::usize i = 0; i < text_length; i++)
                {
                    reply[2 + i] = static_cast<char>(text[i] - 'a' + 'A');
                }

                std::snprintf(reply + 2 + text_length, 16, " (%zu)", text_length);
                socket->send(reply, hsd::cstring::length(reply));
                requests[id]++;
                ++socket;
            }
        }
    }};

    hsd::vector<client_report> reports;
    reports.resize(client_count);

    const char* words[client_count][2] = {
        {"red", "apple"}, {"green", "pear"}, {"yellow", "banana"}
    };

    hsd::vector<hsd::thread> clients;

    for (hsd::usize id = 0; id < client_count; id++)
    {
        clients.emplace_back([&, id]
        {
            hsd::tcp_client_v4 client = address;
            char message[64]{};

            std::snprintf(message, sizeof(message), "%zu:%s", id, words[id][0]);
            client.send(message, hsd::cstring::length(message));
            client.receive(reports[id].first_reply, sizeof(reports[id].first_reply) - 1);

            std::snprintf(message, sizeof(message), "%zu:%s", id, words[id][1]);
            client.send(message, hsd::cstring::length(message));
            client.receive(reports[id].second_reply, sizeof(reports[id].second_reply) - 1);

            std::snprintf(message, sizeof(message), "%zu:bye", id);
            client.send(message, hsd::cstring::length(message));
            client.receive(reports[id].goodbye, sizeof(reports[id].goodbye) - 1);
        });
    }

    // Join the clients in order, then the server that was serving them.
    for (auto& client : clients)
    {
        client.join().unwrap();
    }

    server_thread.join().unwrap();

    for (hsd::usize id = 0; id < client_count; id++)
    {
        hsd::println("client {}: {} | {} | {}"_fmt, id,
            reports[id].first_reply, reports[id].second_reply, reports[id].goodbye);
    }

    hsd::println("requests per client: {} {} {}"_fmt, requests[0], requests[1], requests[2]);
    hsd::println("clients finished: {}"_fmt, finished);
    return 0;
}
