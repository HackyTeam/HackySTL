// Title: UDP log collector
// Level: Intermediate
// Manual: needs UdpLogSender running against 127.0.0.1:54000
// Shows: a udp_server_v4 that numbers every datagram it receives, tallies the levels, and stops on a "/stop" datagram.
//
// Start the collector first, then send lines from one or more senders:
//     ./UdpLogCollector
//     ./UdpLogSender           (type lines such as "WARN disk almost full", or "/stop")

#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;

int main()
{
    hsd::udp_server_v4 server = "127.0.0.1:54000";
    hsd::println("log collector listening on 127.0.0.1:54000"_fmt);

    hsd::usize total = 0;
    hsd::usize infos = 0;
    hsd::usize warnings = 0;
    bool running = true;

    while (running)
    {
        // poll() reads at most one datagram, so it is called in a loop.
        server.poll();

        // Datagrams are grouped per sending address. The map key is the raw
        // address bytes rather than text, so it is not printed here.
        for (auto& [sender, socket] : server)
        {
            char line[1024]{};

            while (socket.receive(line, sizeof(line)) > 0)
            {
                if (hsd::cstring::compare(line, "/stop") == 0)
                {
                    running = false;
                    break;
                }

                total++;
                hsd::println("[{}] {}"_fmt, total, line);

                if (hsd::cstring::compare(line, "INFO", 4) == 0)
                    infos++;
                else if (hsd::cstring::compare(line, "WARN", 4) == 0)
                    warnings++;

                // Clear the buffer so a short line never shows the tail of a long one.
                for (char& ch : line)
                    ch = '\0';
            }
        }

        hsd::this_thread::sleep_for(0.01f).unwrap();
    }

    hsd::println("collected {} lines: {} info, {} warnings"_fmt, total, infos, warnings);
    return 0;
}
