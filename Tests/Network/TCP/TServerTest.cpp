#include <Network.hpp>
#include <Io.hpp>

int main()
{
    hsd::tcp_server_v4 server{"0.0.0.0:54000"};
    char send_buffer[1024]{};
    char recv_buffer[1024]{};

    while (true)
    {
        server.poll();

        for (auto& socket : server)
        {
            auto recv_state = socket.receive(recv_buffer, 1024);
            
            if (recv_state > 0)
            {
                hsd_println("CLIENT> {}", recv_buffer);
                hsd::cstring::copy(send_buffer, recv_buffer);
                auto send_state = socket.send(send_buffer, 1024);

                if (send_state < 0)
                {
                    hsd_println("Error: {}", strerror(send_state));
                }
            }
            else if (recv_state < 0)
            {
                hsd_println_err("Error: {}", server.error_message());
            }
        }
    }
}