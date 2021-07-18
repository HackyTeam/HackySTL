#include <Network.hpp>

int main()
{
    hsd::tcp_client_v4 client{"127.0.0.1:54000"};
    char raw_buf[1024];

    while (true)
    {
        hsd::io::print<"> ">();
        
        auto& stream_ref = hsd::io::read_line().unwrap();
        hsd::cstring::copy_until(raw_buf, stream_ref.c_str(), '\n');
        auto send_state = client.send<"{}">(raw_buf);

        if (send_state == false)
        {
            hsd_println_err("Error: {}", client.error_message());
            continue;
        }

        auto [recv_state, buf] = client.receive();
        
        if (recv_state == true)
        {
            hsd_println("SERVER> {}", buf.data());
        }
        else
        {
            hsd_println_err("Error: {}", client.error_message());
            break;
        }
    }
}