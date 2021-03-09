#include <NetworkServer.hpp>

int main()
{
    hsd::tcp::server server{hsd::net::protocol_type::ipv4, 48000, "0.0.0.0"};

    while(true)
    {
        auto [buf, code] = server.receive();
        
        if(code == hsd::net::received_state::ok)
        {
            hsd::io::print<"CLIENT> {}">(buf.data());
            server.respond<"GET / HTTP/1.0\r\n\r\n">();
        }
        
        if(hsd::cstring::compare(buf.c_str(), "exit") == 0)
            break;

        if(code != hsd::net::received_state::ok)
            break;
    }
}