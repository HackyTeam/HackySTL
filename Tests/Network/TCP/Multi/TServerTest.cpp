#include <Network.hpp>

int main()
{
    hsd::tcp_mserver_v4 server{"0.0.0.0:54000"};

    while (true)
    {
        auto& poll_server = server.get();
        auto poll_result = poll_server.poll(4);

        if (poll_result > 0)
        {
            while (poll_server.check_current_index())
            {
                if (!poll_server.check_and_add_socket() && poll_server.is_readable())
                {
                    // This means we need to receive data and send it back
                    auto [receive_status, buf] = server.receive();
                    bool send_status = server.send<"{}">(buf.c_str());
                }

                poll_server.advance();
            }
        }
    }
}