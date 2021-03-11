#include <NetworkServer.hpp>

int main()
{
    hsd::tcp::server server{hsd::net::protocol_type::ipv4, 48000, "0.0.0.0"};
    char raw_buf[1024];

    while(true)
    {
        auto [buf, code] = server.receive();
        
        if(code == hsd::net::received_state::ok)
        {
            hsd::io::print<"CLIENT> {}">(buf.data());
            
            // Copy the data to a buffer
            hsd::usize length = hsd::cstring::length(buf.c_str());
            hsd::cstring::copy(raw_buf, buf.c_str(), length - 1);
            raw_buf[length - 1] = '\0';

            server.respond<"{}\n">(static_cast<const char*>(raw_buf));
        }
        
        // Because newline will be sent as well
        if(hsd::cstring::compare(buf.c_str(), "exit\n") == 0)
            break;

        if(code != hsd::net::received_state::ok)
            break;
    }
}