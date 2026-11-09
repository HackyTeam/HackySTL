#include <cassert>
#include <cerrno>
#include <cstdio>
#include <string_view>

#include <Network/Network.hpp>
#include <Support/AbortSupport.hpp>

// UDP over the loopback address, with the server and the client in one process. The server
// keeps one child per sender address and buffers the datagrams for it. Nothing blocks: a
// poll with no datagram waiting returns at once.
namespace
{
    using protocol = hsd::network_detail::ip_protocol;

    unsigned free_port()
    {
        hsd::network_detail::udp_socket<protocol::ipv4> probe{"127.0.0.1:0", true};
        sockaddr_in local_address{};
        socklen_t address_length = sizeof(local_address);
        assert(::getsockname(
            probe.fd(), reinterpret_cast<sockaddr*>(&local_address), &address_length
        ) == 0);
        return static_cast<unsigned>(ntohs(local_address.sin_port));
    }

    void loopback_address(char (&buffer)[64], unsigned port)
    {
        const int length = std::snprintf(buffer, sizeof(buffer), "127.0.0.1:%u", port);
        assert(length > 0 && static_cast<hsd::usize>(length) < sizeof(buffer));
    }
}

int main()
{
    char address[64]{};
    loopback_address(address, free_port());

    hsd::udp_server_v4 server{address};
    hsd::udp_client_v4 client{address};

    // Nothing is waiting, so the server's poll records nothing.
    server.poll();
    assert(server.begin() == server.end());

    // A datagram from the client creates a child for its address.
    const std::string_view request = "ORDER 42";
    assert(client.send(request.data(), request.size()) == static_cast<hsd::isize>(request.size()));
    server.poll();

    auto child = server.begin();
    assert(child != server.end());

    // A child reads into a buffer of at least 1024 bytes; smaller buffers are refused.
    char buffer[1024]{};
    assert(child->second.receive(buffer, 1023) == -1);
    const hsd::isize received = child->second.receive(buffer, sizeof(buffer));
    assert(received == static_cast<hsd::isize>(request.size()));
    assert(std::string_view{buffer, static_cast<hsd::usize>(received)} == request);

    // The child answers the sender, and the client reads the answer.
    const std::string_view reply = "SHIPPED 42";
    assert(child->second.send(reply.data(), reply.size()) == static_cast<hsd::isize>(reply.size()));

    char answer[128]{};
    const hsd::isize answered = client.receive(answer, sizeof(answer));
    assert(answered == static_cast<hsd::isize>(reply.size()));
    assert(std::string_view{answer, static_cast<hsd::usize>(answered)} == reply);

    // A second datagram from the same address is buffered for the same child.
    assert(client.send("A", 1) == 1);
    assert(client.send("B", 1) == 1);
    server.poll();
    server.poll();
    // Each datagram is read with its own length, and nothing follows it.
    assert(child->second.receive(buffer, sizeof(buffer)) == 1 && buffer[0] == 'A');
    assert(child->second.receive(buffer, sizeof(buffer)) == 1 && buffer[0] == 'B');
    assert(child->second.receive(buffer, sizeof(buffer)) == 0);

    // Children compare by their sender address.
    assert(child->second == child->second);
    assert(!(child->second != child->second));

    // The error helpers report text and a code for both protocol versions.
    assert(hsd::udp_server_v4::error_message() != nullptr);
    (void)hsd::udp_server_v4::error_code();
    assert(hsd::udp_client_v4::error_message() != nullptr);
    (void)hsd::udp_client_v4::error_code();
    assert(hsd::udp_server_v6::error_message() != nullptr);
    (void)hsd::udp_server_v6::error_code();
    assert(hsd::udp_client_v6::error_message() != nullptr);
    (void)hsd::udp_client_v6::error_code();

    // A server whose address cannot be resolved has no socket, so its receive fails with an
    // error that is not "would block", and the server aborts.
    test_support::expect_abort([]
    {
        hsd::udp_server_v4 broken{"not-a-host.invalid:1"};
        errno = EBADF;
        broken.poll();
    });
    return 0;
}
