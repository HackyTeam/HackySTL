// Title: UDP request and reply
// Level: Advanced
// Shows: a key-value server thread answering text commands from a UDP client, with all output printed after join.

#include <Network/Network.hpp>
#include <Containers/String.hpp>
#include <Containers/UnorderedMap.hpp>
#include <Containers/Vector.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

#include <cstdio>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

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

// Commands look like "SET key value", "GET key", "DEL key" and "QUIT".
// Returns false once the server should stop.
static bool execute(hsd::unordered_map<hsd::string, hsd::string>& store,
    hsd::string_view command, hsd::string& reply)
{
    auto first_space = command.find(' ');
    auto verb = first_space == hsd::string_view::npos
        ? command : command.sub_string(0, first_space).unwrap();

    if (verb == "QUIT"_sv)
    {
        reply = "BYE";
        return false;
    }

    if (first_space == hsd::string_view::npos)
    {
        reply = "ERROR missing key";
        return true;
    }

    auto rest = command.sub_string(first_space + 1, command.length() - first_space - 1).unwrap();
    auto second_space = rest.find(' ');
    auto key = hsd::string{second_space == hsd::string_view::npos
        ? rest : rest.sub_string(0, second_space).unwrap()};

    if (verb == "SET"_sv && second_space != hsd::string_view::npos)
    {
        auto value = rest.sub_string(second_space + 1, rest.length() - second_space - 1).unwrap();
        store[key] = hsd::string{value};
        reply = "OK";
    }
    else if (verb == "GET"_sv)
    {
        auto found = store.at(key);

        if (found.is_ok())
        {
            reply = "VALUE ";
            reply += found.unwrap().get();
        }
        else
        {
            reply = "MISSING";
        }
    }
    else if (verb == "DEL"_sv)
    {
        bool removed = false;

        for (auto entry = store.begin(); entry != store.end(); ++entry)
        {
            if (entry->first == key)
            {
                removed = store.erase(entry).is_ok();
                break;
            }
        }

        reply = removed ? "OK" : "MISSING";
    }
    else
    {
        reply = "ERROR unknown command";
    }

    return true;
}

int main()
{
    char address[32]{};
    std::snprintf(address, sizeof(address), "127.0.0.1:%u", pick_free_port());

    hsd::udp_server_v4 server = address;
    hsd::udp_client_v4 client = address;
    hsd::usize served = 0;

    hsd::thread server_thread{[&]
    {
        hsd::unordered_map<hsd::string, hsd::string> store;
        bool running = true;

        while (running)
        {
            server.poll();

            for (auto& [sender, socket] : server)
            {
                char request[1024]{};

                if (socket.receive(request, sizeof(request)) <= 0)
                    continue;

                // A char array would convert to a view of all 1024 bytes, so go
                // through a pointer to get a view that ends at the terminator.
                const char* text = request;
                hsd::string reply;
                running = execute(store, hsd::string_view{text}, reply);
                socket.send(reply.c_str(), reply.length() + 1);
                served++;
            }

            hsd::this_thread::sleep_for(0.001f).unwrap();
        }
    }};

    const char* commands[] = {
        "SET color blue", "SET size large", "GET color", "GET weight",
        "DEL size", "GET size", "DEL size", "PING", "QUIT"
    };
    constexpr hsd::usize command_count = sizeof(commands) / sizeof(commands[0]);

    // The client sends one request at a time and waits for its answer, so the
    // replies line up with the commands and nothing needs to be numbered.
    hsd::vector<hsd::string> replies;
    replies.resize(command_count);

    for (hsd::usize i = 0; i < command_count; i++)
    {
        client.send(commands[i], hsd::cstring::length(commands[i]));

        char answer[128]{};
        client.receive(answer, sizeof(answer) - 1);
        replies[i] = answer;
    }

    server_thread.join().unwrap();

    for (hsd::usize i = 0; i < command_count; i++)
    {
        hsd::println("{} -> {}"_fmt, commands[i], replies[i]);
    }

    hsd::println("requests served: {}"_fmt, served);
    return 0;
}
