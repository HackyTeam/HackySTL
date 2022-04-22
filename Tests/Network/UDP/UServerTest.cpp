#include <Network.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::udp_server_v4 server = "0.0.0.0:54000";
    char send_buffer[1024]{};
    char recv_buffer[1024]{};

    while (true)
    {
        server.poll();

        for (auto& [_, socket] : server)
        {
            auto recv_state = socket.receive(recv_buffer, 1024);
            
            if (recv_state > 0)
            {
                hsd::println("CLIENT> {}"_fmt, recv_buffer);
                hsd::cstring::copy(send_buffer, recv_buffer);
                auto send_state = socket.send(send_buffer, 1024);

                if (send_state < 0)
                {
                    hsd::println("Error: {}"_fmt, server.error_message());
                }
            }
            else if (recv_state < 0)
            {
                hsd::println_err("Error: {}"_fmt, server.error_message());
            }
        }
    }
}