// Title: UDP log sender
// Level: Intermediate
// Manual: needs UdpLogCollector running on 127.0.0.1:54000
// Shows: a udp_client_v4 that sends every line typed on stdin as one datagram.
//
// Start UdpLogCollector first, then run this program and type lines such as
// "INFO service started" or "WARN disk almost full". "/stop" asks the collector
// to exit, and end of input leaves the sender.

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>

#include <cstdio>

using namespace hsd::format_literals;

int main()
{
    // UDP has no connection: creating the client only remembers the address.
    hsd::udp_client_v4 client = "127.0.0.1:54000";
    char text[512]{};
    hsd::usize sent = 0;

    while (std::fgets(text, sizeof(text), stdin) != nullptr)
    {
        hsd::usize length = hsd::cstring::length(text);

        while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r'))
        {
            text[--length] = '\0';
        }

        if (length == 0)
            continue;

        // Send the text plus its terminator so the collector can print it as a C string.
        // UDP does not report whether anybody received the datagram.
        if (client.send(text, length + 1) < 0)
        {
            hsd::println("send failed: {}"_fmt, client.error_message());
            continue;
        }

        sent++;
    }

    hsd::println("sent {} datagrams"_fmt, sent);
    return 0;
}
