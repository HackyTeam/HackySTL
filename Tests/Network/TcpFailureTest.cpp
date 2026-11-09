#include <cassert>
#include <cstdio>
#include <string_view>

#include <Network/Network.hpp>
#include <Support/AbortSupport.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <csignal>
#include <netdb.h>
#include <unistd.h>
#endif

// The failure paths of the TCP sockets over the loopback address: a connection that is
// refused, a listening address that is already taken, sockets used after they are closed,
// address information of the wrong socket type, moves between sockets, and a poll that a
// signal interrupts (checked in a child process, because the library aborts there).
namespace
{
    using protocol = hsd::network_detail::ip_protocol;
    using socket4 = hsd::network_detail::tcp_socket<protocol::ipv4>;

    // A loopback port that no socket is listening on: bound, read and released.
    unsigned closed_port()
    {
        socket4 probe{"127.0.0.1:0", true};
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
    loopback_address(address, closed_port());

    // A client to a port with no listener cannot connect, so its socket is not valid.
    hsd::tcp_client_v4 refused{address};
    char scratch[8]{};
    assert(refused.send("x", 1) == -1);
    assert(refused.receive(scratch, sizeof(scratch)) == -1);

    // A second server on an address that a server already holds cannot bind to it.
    hsd::tcp_server_v4 first{address};
    hsd::tcp_server_v4 second{address};
    second.poll(0);
    assert(second.begin() == second.end());

    // A socket that has been closed refuses to handle addresses.
    addrinfo stream_info{};
    stream_info.ai_socktype = SOCK_STREAM;
    socket4 closed_socket{pollfd{-1, 0, 0}};
    assert(!closed_socket.handle(&stream_info, true));
    assert(closed_socket.send("x", 1) == -1);
    assert(closed_socket.receive(scratch, sizeof(scratch)) == -1);

    // Address information for a datagram socket is not a stream, so it is refused.
    addrinfo datagram_info{};
    datagram_info.ai_socktype = SOCK_DGRAM;
    socket4 open_socket{pollfd{::socket(AF_INET, SOCK_STREAM, 0), 0, 0}};
    assert(open_socket.is_valid());
    assert(!open_socket.handle(&datagram_info, true));
    assert(!open_socket.handle(&datagram_info, false));
    open_socket.close();
    assert(!open_socket.is_valid());

    // A datagram socket binds to a stream address but cannot listen, so the handshake fails.
    addrinfo* resolved = nullptr;
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    assert(getaddrinfo("127.0.0.1", "0", &hints, &resolved) == 0);
    socket4 datagram{pollfd{::socket(AF_INET, SOCK_DGRAM, 0), 0, 0}};
    assert(!datagram.handle(resolved, true));
    freeaddrinfo(resolved);

    // Moving one socket into another closes the target's old descriptor with the moved-from
    // object, and the moved-from socket keeps the descriptor it had.
    socket4 source{pollfd{::socket(AF_INET, SOCK_STREAM, 0), 0, 0}};
    socket4 target{pollfd{::socket(AF_INET, SOCK_STREAM, 0), 0, 0}};
    const auto source_fd = source.fd();
    target = hsd::move(source);
    assert(target.fd() == source_fd);

    // Server children move between slots through their own move assignment.
    const unsigned port = closed_port();
    char live_address[64]{};
    loopback_address(live_address, port);
    hsd::tcp_server_v4 server{live_address};
    hsd::tcp_client_v4 client{live_address};
    assert(client.send("ping", 4) == 4);
    server.poll(1000);

    // Until the next poll, the accepted child has not been seen as readable, so reading gives 0.
    auto early_child = server.begin();
    assert(early_child != server.end());
    assert(early_child->receive(scratch, sizeof(scratch)) == 0);
    server.poll(1000);

    hsd::network_detail::tcp_child_socket<protocol::ipv4> left{pollfd{-1, 0, 0}};
    hsd::network_detail::tcp_child_socket<protocol::ipv4> right{pollfd{-1, 0, 0}};
    left = hsd::move(right);
    assert(!left.is_valid());

    // The error helpers of the IPv6 variants report text and a code, with no socket needed.
    assert(hsd::tcp_server_v6::error_message() != nullptr);
    (void)hsd::tcp_server_v6::error_code();
    assert(hsd::tcp_client_v6::error_message() != nullptr);
    (void)hsd::tcp_client_v6::error_code();

#if defined(__unix__) || defined(__APPLE__)
    // A socket option cannot be set on a descriptor that is not a socket, and the server aborts.
    test_support::expect_abort([]
    {
        int ends[2]{};
        assert(::pipe(ends) == 0);
        socket4 not_a_socket{pollfd{ends[0], 0, 0}};
        addrinfo stream_hint{};
        stream_hint.ai_socktype = SOCK_STREAM;
        (void)not_a_socket.handle(&stream_hint, true);
    });

    // A signal that interrupts poll makes it fail, and the server reports that and aborts.
    test_support::expect_abort([]
    {
        char interrupted_address[64]{};
        loopback_address(interrupted_address, closed_port());
        hsd::tcp_server_v4 waiting{interrupted_address};
        std::signal(SIGALRM, [](int) {});
        alarm(1);
        waiting.poll(5000);
    });
#endif
    return 0;
}
