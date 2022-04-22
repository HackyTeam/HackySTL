#include <Network.hpp>
#include <Io.hpp>

int main()
{
    using namespace hsd::format_literals;

    hsd::tcp_client_v4 client = "127.0.0.1:54000";
    char send_buffer[1024]{};
    char recv_buffer[1024]{};

    while (true)
    {
        hsd::print("> "_fmt);
        hsd::io::cin().get_stream().reserve(1024);

        auto& stream_ref = hsd::io::cin().read_chunk().unwrap().get_stream();
        hsd::cstring::copy_until(send_buffer, stream_ref.c_str(), '\n');
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