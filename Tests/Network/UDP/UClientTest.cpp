#include <Network.hpp>

int main()
{
    hsd::udp_client_v4 client{"127.0.0.1:54000"};
    char raw_buf[1024];

    while (true)
    {
        hsd::io::print<"> ">();
        
        auto& stream_ref = hsd::io::read_line().unwrap();
        hsd::cstring::copy_until(raw_buf, stream_ref.c_str(), '\n');
        auto state = client.send<"{}">(raw_buf);

        if (state == static_cast<hsd::isize>(hsd::net::received_state::err))
        {
            hsd_println_err("Error: {}", client.error_message());
            continue;
        }

        auto [code, buf] = client.receive();
        
        if (code == static_cast<hsd::isize>(hsd::net::received_state::ok))
        {
            hsd::io::print<"SERVER> {}\n">(buf.data());
        }
        else
        {
            hsd_println_err("Error: {}", client.error_message());
            break;
        }
    }
}