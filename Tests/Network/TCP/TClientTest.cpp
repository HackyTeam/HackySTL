#include "../../../cpp/NetworkClient.hpp"

int main()
{
    hsd::tcp::client client{hsd::tcp::protocol_type::ipv4, 54000, "127.0.0.1"};
    hsd::tcp::received_state code;

    hsd::u8string to_be_sent;

    while(true)
    {
        hsd::io::print("> ");
        to_be_sent = hsd::io::read_line().to_string();
        auto state = client.respond("test\n");

        if(state == hsd::tcp::received_state::err)
            continue;

        auto [buf, code] = client.receive();
        hsd::io::print("SERVER> {}", buf.data());
        
        if(code != hsd::tcp::received_state::ok)
            break;
    }
}
