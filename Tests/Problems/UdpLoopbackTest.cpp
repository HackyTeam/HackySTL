#include <cassert>
#include <cstdio>

#include <Network/Network.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::string_view_literals;

int main()
{
    using protocol = hsd::network_detail::ip_protocol;

#if defined(HSD_PLATFORM_WINDOWS)
    hsd::network_detail::init_winsock();
#endif

    hsd::network_detail::udp_socket<protocol::ipv4> server{
        "127.0.0.1:0", true
    };
    sockaddr_in local_address{};
#if defined(HSD_PLATFORM_WINDOWS)
    int address_length = sizeof(local_address);
#else
    socklen_t address_length = sizeof(local_address);
#endif
    assert(::getsockname(
        server.fd(),
        reinterpret_cast<sockaddr*>(&local_address),
        &address_length
    ) == 0);

    char client_address[64]{};
    const auto port = static_cast<unsigned>(ntohs(local_address.sin_port));
    const int address_size = std::snprintf(
        client_address, sizeof(client_address), "127.0.0.1:%u", port
    );
    assert(address_size > 0);
    assert(static_cast<hsd::usize>(address_size) < sizeof(client_address));

    hsd::udp_client_v4 client{client_address};
    constexpr auto request = "PING order-42"_sv;
    assert(client.send(request.data(), request.length()) ==
        static_cast<hsd::isize>(request.length()));

    char request_buffer[128]{};
    hsd::isize received = -1;
    for (int attempt = 0; attempt < 2000 && received < 0; ++attempt)
    {
        received = server.receive(request_buffer, sizeof(request_buffer));
        if (received < 0)
        {
            hsd::this_thread::sleep_for(0.001f).unwrap();
        }
    }
    assert(received == static_cast<hsd::isize>(request.length()));
    const hsd::string_view received_request{
        request_buffer, static_cast<hsd::usize>(received)
    };
    assert(received_request == request);

    constexpr auto response = "ACK order-42"_sv;
    assert(server.send(response.data(), response.length()) ==
        static_cast<hsd::isize>(response.length()));

    char response_buffer[128]{};
    const auto response_size = client.receive(response_buffer, sizeof(response_buffer));
    assert(response_size == static_cast<hsd::isize>(response.length()));
    const hsd::string_view received_response{
        response_buffer, static_cast<hsd::usize>(response_size)
    };
    assert(received_response == response);
}