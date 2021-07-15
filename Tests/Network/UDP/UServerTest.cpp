#include <Network.hpp>

int main()
{
    hsd::udp_server_v4 server{"0.0.0.0:54000"};
    char raw_buf[1024];

    while (true)
    {
        auto [code, buf] = server.receive();
        
        if (code == static_cast<hsd::isize>(hsd::net::received_state::ok))
        {
            hsd::io::print<"CLIENT> {}\n">(buf.data());
            
            // Copy the data to a buffer
            hsd::cstring::copy(raw_buf, buf.c_str());
            server.send<"{}">(raw_buf);

            if (hsd::cstring::compare(raw_buf, "exit") == 0)
                break;
        }
        else
        {
            hsd_println_err("Error: {}", server.error_message());
            break;
        }
    }
}