#include <NetworkClient.hpp>

int main()
{
    hsd::udp::client client{hsd::net::protocol_type::ipv4, 443, "https://www.discord.com"};

    while(true)
    {
        hsd::io::print<"> ">();
        auto state = client.respond<"{}">(hsd::io::read_line().expect().c_str());

        if(state == hsd::net::received_state::err)
            continue;

        auto [buf, code] = client.receive();
            
        if(code == hsd::net::received_state::ok)
            hsd::io::print<"SERVER> {}">(buf.data());
        
        if(code != hsd::net::received_state::ok)
            break;
    }
}