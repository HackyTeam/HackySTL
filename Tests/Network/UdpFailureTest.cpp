#include <cassert>
#include <cstdio>

#include <Network/Network.hpp>
#include <Support/AbortSupport.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <netdb.h>
#include <unistd.h>
#endif

// The failure paths of the UDP sockets: an address that does not resolve, a socket that is
// closed or was never opened, address information of the wrong socket type, a bind that
// another socket already holds, a connect that the kernel refuses, and a child that has no
// socket. The setsockopt failure ends the process, so it runs in a child.
namespace
{
    using protocol = hsd::network_detail::ip_protocol;
    using socket4 = hsd::network_detail::udp_socket<protocol::ipv4>;

    void loopback_address(char (&buffer)[64], unsigned port)
    {
        const int length = std::snprintf(buffer, sizeof(buffer), "127.0.0.1:%u", port);
        assert(length > 0 && static_cast<hsd::usize>(length) < sizeof(buffer));
    }
}

int main()
{
    // An address that cannot be resolved leaves the socket invalid, and sends fail.
    socket4 unresolved{"not-a-host.invalid:1"};
    assert(!unresolved.is_valid());
    char scratch[8]{};
    assert(unresolved.send("x", 1) == -1);
    assert(unresolved.receive(scratch, sizeof(scratch)) == -1);

#if defined(__unix__) || defined(__APPLE__)
    addrinfo datagram_info{};
    datagram_info.ai_socktype = SOCK_DGRAM;
    addrinfo stream_info{};
    stream_info.ai_socktype = SOCK_STREAM;

    // A socket built from a closed descriptor refuses to handle addresses.
    socket4 closed{-1, sockaddr_storage{}, 0};
    assert(!closed.handle(&datagram_info, true));
    assert(!closed.handle(&datagram_info, false));

    // Address information for a stream is not for a datagram socket, on either side.
    socket4 open{::socket(AF_INET, SOCK_DGRAM, 0), sockaddr_storage{}, 0};
    assert(open.is_valid());
    assert(!open.handle(&stream_info, true));
    assert(!open.handle(&stream_info, false));
    open.close();
    assert(!open.is_valid());

    // A port that another socket holds without address reuse cannot be bound.
    const int holder = ::socket(AF_INET, SOCK_DGRAM, 0);
    assert(holder >= 0);
    sockaddr_in held{};
    held.sin_family = AF_INET;
    held.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(::bind(holder, reinterpret_cast<sockaddr*>(&held), sizeof(held)) == 0);
    socklen_t held_length = sizeof(held);
    assert(::getsockname(holder, reinterpret_cast<sockaddr*>(&held), &held_length) == 0);
    char held_address[64]{};
    loopback_address(held_address, static_cast<unsigned>(ntohs(held.sin_port)));
    socket4 blocked{held_address, true};
    assert(!blocked.is_valid());
    ::close(holder);

    // A connect to the broadcast address is refused without the broadcast option.
    socket4 broadcast{"255.255.255.255:9"};
    assert(!broadcast.is_valid());

    // A socket option cannot be set on a descriptor that is not a socket, and the socket aborts.
    test_support::expect_abort([]
    {
        int ends[2]{};
        assert(::pipe(ends) == 0);
        addrinfo datagram_hint{};
        datagram_hint.ai_socktype = SOCK_DGRAM;
        socket4 not_a_socket{ends[0], sockaddr_storage{}, 0};
        (void)not_a_socket.handle(&datagram_hint, true);
    });
#endif

    // A child without a descriptor cannot send, and has nothing to receive into.
    hsd::network_detail::udp_child_socket<protocol::ipv4> orphan{-1, sockaddr_storage{}, 0};
    assert(orphan.send("x", 1) == -1);
    assert(orphan.receive(scratch, 1024) == -1);
    assert(!orphan.is_valid());

    // A client whose address does not resolve reports the failure, and it cannot send.
    hsd::udp_client_v4 unresolved_client{"not-a-host.invalid:1"};
    assert(unresolved_client.send("x", 1) == -1);

    // Children with the same peer address are equal only when their handles are equal too.
    sockaddr_storage peer{};
    hsd::network_detail::udp_child_socket<protocol::ipv4> first{-1, peer, 0};
    hsd::network_detail::udp_child_socket<protocol::ipv4> second{-2, peer, 0};
    assert(!(first == second));
    assert(first != second);

    // Children with different peer addresses are never equal, whatever their handles.
    sockaddr_storage elsewhere_address{};
    elsewhere_address.ss_family = AF_INET6;
    hsd::network_detail::udp_child_socket<protocol::ipv4> elsewhere{-1, elsewhere_address, 0};
    assert(!(first == elsewhere));
    assert(first != elsewhere);

    // The helper that reports the error text of a server and client with no socket is still valid.
    assert(hsd::udp_client_v6::error_message() != nullptr);
    return 0;
}
