#include <NetworkServer.hpp>

int main()
{
    hsd::udp::server server{hsd::net::protocol_type::ipv4, 54000, "0.0.0.0"};

    while(true)
    {
        auto [buf, code] = server.receive();
        
        if(code == hsd::net::received_state::ok)
        {
            hsd::io::print<"CLIENT> {}">(buf.data());
            server.respond<"Good\n">();
        }
        
        // because newline will be sent as well
        if(buf.to_string() == "exit\n")
            break;

        if(code != hsd::net::received_state::ok)
            break;
    }
}