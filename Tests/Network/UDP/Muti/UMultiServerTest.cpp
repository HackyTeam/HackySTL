#include <NetworkServer.hpp>

int main()
{
    hsd::udp::multi_server srv;
    char raw_buf[1024];

    srv.add(hsd::net::protocol_type::ipv4, "0.0.0.0:32000");
    srv.add(hsd::net::protocol_type::ipv4, "0.0.0.0:48000");
    srv.add(hsd::net::protocol_type::ipv4, "0.0.0.0:54000");
    srv.add(hsd::net::protocol_type::ipv4, "0.0.0.0:65000");

    while (true)
    {
        for (hsd::usize client_index = 0; auto& server_sock : srv)
        {
            auto [buf, code] = server_sock.receive();

            if (code == hsd::net::received_state::ok)
            {
                hsd::io::print<"CLIENT {}> {}\n">(client_index, buf.data());

                // Copy the data to a buffer
                hsd::cstring::copy(raw_buf, buf.c_str());
                server_sock.respond<"{}">(raw_buf);
            }

            client_index++;
        }
    }
}