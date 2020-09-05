#include "../../../cpp/NetworkClient.hpp"

int main()
{
    hsd::udp::client client{hsd::net::protocol_type::ipv4, 54000, "192.168.0.108"};

    while(true)
    {
        hsd::io::print("> ");
        auto state = client.respond("{}", hsd::io::read_line().to_string());

        if(state == hsd::net::received_state::err)
            continue;

        auto [buf, code] = client.receive();
            
        if(code == hsd::net::received_state::ok)
            hsd::io::print("SERVER> {}", buf.data());
        
        if(code != hsd::net::received_state::ok)
            break;
    }
}