#include <NetworkClient.hpp>

int main()
{
    hsd::tcp::client client{hsd::net::protocol_type::ipv4, 48000, "127.0.0.1"};

    while(true)
    {
        hsd::io::print<"> ">();
        auto state = client.respond<"{}">(hsd::io::read_line().unwrap().to_string());

        if(state == hsd::net::received_state::err)
            continue;

        auto [buf, code] = client.receive();
        
        if(code == hsd::net::received_state::ok)
            hsd::io::print<"SERVER> {}">(buf.data());
        
        if(code != hsd::net::received_state::ok)
            break;
    }
}