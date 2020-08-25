#include "../../../cpp/NetworkClient.hpp"

int main()
{
    hsd::tcp::client client{hsd::tcp::protocol_type::ipv4, 53000, "127.0.0.1"};
    hsd::tcp::received_state code;

    do
    {
        hsd::io::print("> ");
        hsd::u8string to_be_sent = hsd::io::read_line().to_string();
        auto state = client.respond("{}\n", to_be_sent);

        if(state == hsd::tcp::received_state::err)
        {
            hsd::io::err_print("couldn\'t send to server\n");
            continue;
        }

        auto [buf, code] = client.receive();
        hsd::io::print("SERVER> {}", buf);
        
        if(code != hsd::tcp::received_state::ok)
            break;
    } while(true);
}
