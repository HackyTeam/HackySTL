#include <Network/Network.hpp>
#include <Serialize/Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::udp_client_v4 client = "127.0.0.1:54000";
    char send_buffer[1024]{};
    char recv_buffer[1024]{};

    while (true)
    {
        hsd::print("> "_fmt);

        auto read_result = hsd::io::cin().read_chunk();
        if (read_result.is_ok() == false)
        {
            break;
        }

        auto& input = read_result.unwrap().get();
        if (input.get_stream_data().length() == 0)
        {
            break;
        }

        if (input.set_data(send_buffer).is_ok() == false)
        {
            break;
        }
 
        auto send_state = client.send(send_buffer, 1024);

        if (send_state < 0)
        {
            hsd::println_err("Error: {}"_fmt, client.error_message());
            continue;
        }

        auto recv_state = client.receive(recv_buffer, 1024);
        
        if (recv_state > 0)
        {
            hsd::println("SERVER> {}"_fmt, recv_buffer);
        }
        else if (recv_state < 0)
        {
            hsd::println_err("Error: {}"_fmt, client.error_message());
            break;
        }
    }
}