#include "../../../cpp/NetworkServer.hpp"

int main()
{
    hsd::tcp::server server{hsd::tcp::protocol_type::ipv4, 53000, "0.0.0.0"};
    hsd::tcp::received_state code;
    int i = -3;

    while(true)
    {
        auto [buf, code] = server.receive();
        hsd::io::print("CLIENT> {}", buf);
        server.respond("Good\n");
        
        if(buf == "exit")
            break;

        if(code != hsd::tcp::received_state::ok)
            break;
    }
}