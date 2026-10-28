// Title: TCP chat server
// Level: Intermediate
// Manual: needs one or more TcpChatClient programs connecting to 127.0.0.1:54000
// Shows: a polling tcp_server_v4 that relays every line to the other connected clients.
//
// Run the server in one terminal and a client in each of two more:
//     ./TcpChatServer
//     ./TcpChatClient          (type lines, finish with /quit or end of input)

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>

#include <cstdio>

using namespace hsd::format_literals;

int main()
{
    // The first socket in the server is the listening one; begin() skips it.
    hsd::tcp_server_v4 server = "127.0.0.1:54000";
    hsd::println("chat server listening on 127.0.0.1:54000"_fmt);

    while (true)
    {
        server.poll(100);

        for (auto socket = server.begin(); socket != server.end();)
        {
            char buffer[512]{};
            auto received = socket->receive(buffer, sizeof(buffer) - 1);

            if (received <= 0)
            {
                ++socket;
                continue;
            }

            // The descriptor number is a good enough nickname for a demo.
            char line[600]{};
            bool quitting = hsd::cstring::compare(buffer, "/quit") == 0;

            if (quitting)
                std::snprintf(line, sizeof(line), "user%d left\n", static_cast<int>(socket->fd()));
            else
                std::snprintf(line, sizeof(line), "user%d: %s\n", static_cast<int>(socket->fd()), buffer);

            hsd::print("{}"_fmt, line);

            // Relay to everyone except the sender.
            for (auto other = server.begin(); other != server.end(); ++other)
            {
                if (other != socket)
                {
                    other->send(line, hsd::cstring::length(line));
                }
            }

            if (quitting)
            {
                // Tell the leaving client to stop reading, then drop its socket.
                socket->send("/quit", 5);
                socket = server.erase(socket).unwrap();
            }
            else
            {
                ++socket;
            }
        }
    }
}
