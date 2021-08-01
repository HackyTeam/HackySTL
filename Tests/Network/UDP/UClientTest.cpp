#include <Network.hpp>
#include <Io.hpp>

int main()
{
    hsd::udp_client_v4 client{"127.0.0.1:54000"};
    char send_buffer[1024]{};
    char recv_buffer[1024]{};

    while (true)
    {
        hsd::io::print<"> ">();
        
        auto& stream_ref = hsd::io::read_line().unwrap();
        hsd::cstring::copy_until(send_buffer, stream_ref.c_str(), '\n');
        auto send_state = client.send(send_buffer, 1024);

        if (send_state < 0)
        {
            hsd_println_err("Error: {}", client.error_message());
            continue;
        }

        auto recv_state = client.receive(recv_buffer, 1024);
        
        if (recv_state > 0)
        {
            hsd_println("SERVER> {}", recv_buffer);
        }
        else if (recv_state < 0)
        {
            hsd_println_err("Error: {}", client.error_message());
            break;
        }
    }
}