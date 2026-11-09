#include <cassert>
#include <cstdio>
#include <string_view>

#include <Network/Network.hpp>

// TCP over the loopback address, with the server and the client in one process. The
// connection is queued by the kernel before the server accepts it, so one thread can
// run both ends: the client connects, the server accepts on poll, and each side reads
// and writes in turn.
namespace
{
    using protocol = hsd::network_detail::ip_protocol;

    // Asks the kernel for a free port on the loopback address, then releases it for the test.
    unsigned free_port()
    {
        hsd::network_detail::tcp_socket<protocol::ipv4> probe{"127.0.0.1:0", true};
        sockaddr_in local_address{};
        socklen_t address_length = sizeof(local_address);
        assert(::getsockname(
            probe.fd(), reinterpret_cast<sockaddr*>(&local_address), &address_length
        ) == 0);
        return static_cast<unsigned>(ntohs(local_address.sin_port));
    }

    // Formats "127.0.0.1:<port>" for the address-taking constructors.
    void loopback_address(char (&buffer)[64], unsigned port)
    {
        const int length = std::snprintf(buffer, sizeof(buffer), "127.0.0.1:%u", port);
        assert(length > 0 && static_cast<hsd::usize>(length) < sizeof(buffer));
    }
}

int main()
{
    const unsigned port = free_port();
    char address[64]{};
    loopback_address(address, port);

    // The server listens, and the client connects through the backlog.
    hsd::tcp_server_v4 server{address};
    hsd::tcp_client_v4 client{address};

    // Nothing is pending until the client sends, so the first poll accepts the connection.
    const std::string_view request = "GET / HTTP/1.1";
    assert(client.send(request.data(), request.size()) == static_cast<hsd::isize>(request.size()));
    server.poll(1000);

    // A child joins the poll set after the poll that accepted it, so its readiness is
    // known only after the next poll.
    server.poll(1000);

    auto child = server.begin();
    assert(child != server.end());

    char buffer[128]{};
    hsd::isize received = child->receive(buffer, sizeof(buffer));
    assert(received == static_cast<hsd::isize>(request.size()));
    assert((std::string_view{buffer, static_cast<hsd::usize>(received)} == request));

    const std::string_view reply = "HTTP/1.1 200 OK";
    assert(child->send(reply.data(), reply.size()) == static_cast<hsd::isize>(reply.size()));

    char answer[128]{};
    const hsd::isize answered = client.receive(answer, sizeof(answer));
    assert(answered == static_cast<hsd::isize>(reply.size()));
    assert((std::string_view{answer, static_cast<hsd::usize>(answered)} == reply));

    // The error helpers report the last socket error as text and as a code.
    assert(hsd::tcp_server_v4::error_message() != nullptr);
    (void)hsd::tcp_server_v4::error_code();
    assert(hsd::tcp_client_v4::error_message() != nullptr);
    (void)hsd::tcp_client_v4::error_code();

    // Closing the child socket removes it from the server's list.
    server.erase(child).unwrap();
    assert(server.begin() == server.end());
    return 0;
}
