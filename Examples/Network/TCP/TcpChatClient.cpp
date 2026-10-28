// Title: TCP chat client
// Level: Intermediate
// Manual: needs TcpChatServer running on 127.0.0.1:54000
// Shows: a tcp_client_v4 that sends stdin lines while a thread prints what the server relays.
//
// Start TcpChatServer first, then run this program in one or more terminals.
// Type a line and press Enter to send it. "/quit" or end of input leaves the chat.

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

#include <cstdio>

using namespace hsd::format_literals;

int main()
{
    hsd::tcp_client_v4 client = "127.0.0.1:54000";

    // The reader thread blocks in receive() and prints each relayed line.
    // It stops when the server answers our "/quit" with "/quit".
    hsd::thread reader{[&]
    {
        while (true)
        {
            char buffer[600]{};
            auto received = client.receive(buffer, sizeof(buffer) - 1);

            if (received <= 0 || hsd::cstring::compare(buffer, "/quit") == 0)
                break;

            hsd::print("{}"_fmt, buffer);
        }
    }};

    // Plain C input keeps the loop simple: one fgets() call per typed line.
    // Whatever ends the loop, "/quit" is sent exactly once.
    char text[512]{};

    while (std::fgets(text, sizeof(text), stdin) != nullptr)
    {
        hsd::usize length = hsd::cstring::length(text);

        while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r'))
        {
            text[--length] = '\0';
        }

        if (length == 0)
            continue;

        if (hsd::cstring::compare(text, "/quit") == 0)
            break;

        client.send(text, length);
    }

    client.send("/quit", 5);
    reader.join().unwrap();
    return 0;
}
