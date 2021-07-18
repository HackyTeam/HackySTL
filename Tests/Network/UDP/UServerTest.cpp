#include <Network.hpp>

int main()
{
    hsd::udp_dserver_v4 server{"0.0.0.0:54000"};
    char raw_buf[1024];

    while (true)
    {
        auto [recv_state, buf] = server.receive();
        
        if (recv_state == true)
        {
            hsd_println("CLIENT> {}", buf.data());
            
            // Copy the data to a buffer
            hsd::cstring::copy(raw_buf, buf.c_str());
            bool send_state = server.send<"{}">(raw_buf);

            if (send_state == false)
            {
                hsd_println_err(
                    "Error: {}", server.error_message()
                );
            }

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