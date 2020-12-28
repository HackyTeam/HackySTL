#include "../../../cpp/NetworkServer.hpp"

int main()
{
    hsd::tcp::server server{hsd::net::protocol_type::ipv4, 48000, "0.0.0.0"};

    while(true)
    {
        auto [buf, code] = server.receive();
        
        if(code == hsd::net::received_state::ok)
        {
            hsd::io::print<"CLIENT> {}">(buf.data());
            server.respond<"Good\n">();
        }
        
        if(buf.to_string() == "exit")
            break;

        if(code != hsd::net::received_state::ok)
            break;
    }
}