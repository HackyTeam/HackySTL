#include "../../../cpp/NetworkClient.hpp"

int main()
{
    hsd::tcp::client client{hsd::net::protocol_type::ipv4, 48000, "127.0.0.1"};

    for(hsd::u16 i = 48000; i < 60000; i++)
    {
        if(client.switch_to(hsd::net::protocol_type::ipv4, i, "127.0.0.1"))
            break;
    }

    while(true)
    {
        //hsd::io::print<"> ">();
        auto state = client.respond<"{}">(122);

        if(state == hsd::net::received_state::err)
            continue;

        auto [buf, code] = client.receive();
        
        if(code == hsd::net::received_state::ok)
            hsd::io::print<"SERVER> {}">(buf.data());
        
        if(code != hsd::net::received_state::ok)
            break;
    }
}