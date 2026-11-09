#include <cassert>
#include <cstdio>
#include <string_view>

#include <Network/Network.hpp>
#include <Support/AbortSupport.hpp>

#if defined(__unix__) || defined(__APPLE__)
#include <netdb.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <unistd.h>
#endif

// The address and error helpers of the network layer: the packed peer key for IPv4 and IPv6
// (which the server uses to tell peers apart), name resolution with and without a port, the
// socket creation that takes an address with or without one, and the error reporting. The
// helpers that abort are checked in child processes.
namespace
{
    using protocol = hsd::network_detail::ip_protocol;

    sockaddr_storage make_ipv4(const char* text, unsigned short port)
    {
        sockaddr_storage storage{};
        auto* address = reinterpret_cast<sockaddr_in*>(&storage);
        address->sin_family = AF_INET;
        address->sin_port = htons(port);
        assert(inet_pton(AF_INET, text, &address->sin_addr) == 1);
        return storage;
    }

    sockaddr_storage make_ipv6(const char* text, unsigned short port)
    {
        sockaddr_storage storage{};
        auto* address = reinterpret_cast<sockaddr_in6*>(&storage);
        address->sin6_family = AF_INET6;
        address->sin6_port = htons(port);
        assert(inet_pton(AF_INET6, text, &address->sin6_addr) == 1);
        return storage;
    }
}

namespace
{
    // A socket type with the members create_socket uses. Its handshake always fails, so the
    // loop closes each candidate and reports failure, which name resolution alone cannot
    // produce on the loopback address.
    struct refusing_socket
    {
        hsd::network_detail::native_socket_type fd_value = -1;
        int handshakes = 0;

        static constexpr int ip_protocol() { return AF_INET; }
        static constexpr int sock_type() { return SOCK_STREAM; }
        static constexpr int net_protocol() { return 0; }

        refusing_socket& operator=(pollfd&& data)
        {
            fd_value = data.fd;
            return *this;
        }

        bool is_valid() const { return fd_value >= 0; }

        bool handle(const addrinfo*, bool)
        {
            ++handshakes;
            return false;
        }

        void close()
        {
            if (fd_value >= 0)
            {
            #if defined(HSD_PLATFORM_WINDOWS)
                ::closesocket(fd_value);
            #else
                ::close(fd_value);
            #endif
            }
            fd_value = -1;
        }
    };
}

namespace
{
    // A socket type for the failed socket() call. The descriptor limit is lowered below, so
    // socket() fails for every candidate; name resolution still succeeds with no protocol set.
    struct unsupported_socket
    {
        hsd::network_detail::native_socket_type fd_value = -1;

        static constexpr int ip_protocol() { return AF_INET; }
        static constexpr int sock_type() { return SOCK_STREAM; }
        static constexpr int net_protocol() { return 0; }

        unsupported_socket& operator=(pollfd&& data)
        {
            fd_value = data.fd;
            return *this;
        }

        bool is_valid() const { return fd_value >= 0; }

        bool handle(const addrinfo*, bool) { return false; }

        void close() { fd_value = -1; }
    };
}

int main()
{
    // Every candidate fails its handshake, so each socket is closed and no socket is kept.
    refusing_socket refusing;
    assert(!hsd::network_detail::create_socket(refusing, "127.0.0.1:0", true));
    assert(refusing.handshakes == 1 && !refusing.is_valid());

#if defined(__unix__) || defined(__APPLE__)
    // When socket() itself fails, no candidate is handled and none is kept. With a descriptor
    // limit of zero, socket() fails for every candidate, and the limit is restored at once.
    rlimit saved_limit{};
    assert(getrlimit(RLIMIT_NOFILE, &saved_limit) == 0);
    rlimit no_descriptors = saved_limit;
    no_descriptors.rlim_cur = 0;
    const int lowered = setrlimit(RLIMIT_NOFILE, &no_descriptors);
    unsupported_socket unsupported;
    const bool made = hsd::network_detail::create_socket(unsupported, "127.0.0.1:0", true);
    const int restored = setrlimit(RLIMIT_NOFILE, &saved_limit);
    assert(lowered == 0 && restored == 0);
    assert(!made && !unsupported.is_valid());
#endif

    // The IPv4 key is the four address bytes followed by the port, in network byte order.
    // The key's size counts one more than its characters, so the size is used here.
    const sockaddr_storage v4 = make_ipv4("127.0.0.1", 8080);
    const auto v4_key = hsd::network_detail::to_str(&v4);
    assert(v4_key.size() == 6);
    assert(v4_key[0] == 127 && v4_key[1] == 0 && v4_key[2] == 0 && v4_key[3] == 1);
    assert(static_cast<unsigned char>(v4_key[4]) == (8080 >> 8));
    assert(static_cast<unsigned char>(v4_key[5]) == (8080 & 0xff));

    // The IPv6 key is the sixteen address bytes followed by the port, in network byte order.
    const sockaddr_storage v6 = make_ipv6("::1", 9);
    const auto v6_key = hsd::network_detail::to_str(&v6);
    assert(v6_key.size() == 18);
    assert(v6_key[15] == 1 && v6_key[0] == 0);
    assert(v6_key[16] == 0 && v6_key[17] == 9);

    // Peers that differ only in the high byte of the port have different keys.
    const sockaddr_storage other_port = make_ipv4("127.0.0.1", 8336);
    assert(hsd::network_detail::to_str(&v4) != hsd::network_detail::to_str(&other_port));

    // Name resolution: a numeric address resolves, an unknown host does not.
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* resolved = nullptr;
    assert(hsd::network_detail::get_addr_info("127.0.0.1", "80", &hints, &resolved));
    assert(resolved != nullptr);
    freeaddrinfo(resolved);
    addrinfo* missing = nullptr;
    assert(!hsd::network_detail::get_addr_info("not-a-host.invalid", "80", &hints, &missing));
    assert(missing == nullptr);

    // Socket creation takes "address:port" and also an address alone, which binds to any port.
    hsd::network_detail::tcp_socket<protocol::ipv4> with_port{"127.0.0.1:0", true};
    assert(with_port.is_valid());
    hsd::network_detail::tcp_socket<protocol::ipv4> without_port{"127.0.0.1", true};
    assert(without_port.is_valid());
    hsd::network_detail::tcp_socket<protocol::ipv4> unresolved{"not-a-host.invalid", true};
    assert(!unresolved.is_valid());
    hsd::network_detail::tcp_socket<protocol::ipv4> unresolved_port{"not-a-host.invalid:80", true};
    assert(!unresolved_port.is_valid());

    // IPv6 sockets work the same way, with the port after the last colon. A round trip runs
    // only where the loopback address can be bound, which is the case on most hosts.
    unsigned short free_port6 = 0;
    {
        hsd::network_detail::tcp_socket<protocol::ipv6> probe{"::1:0", true};
        if (probe.is_valid())
        {
            sockaddr_in6 local{};
            socklen_t length = sizeof(local);
            assert(::getsockname(probe.fd(), reinterpret_cast<sockaddr*>(&local), &length) == 0);
            free_port6 = ntohs(local.sin6_port);
        }
    }
    if (free_port6 != 0)
    {
        char address6[64]{};
        std::snprintf(address6, sizeof(address6), "::1:%u", static_cast<unsigned>(free_port6));
        hsd::tcp_server_v6 server6{address6};
        hsd::tcp_client_v6 client6{address6};
        assert(client6.send("hi", 2) == 2);
        server6.poll(1000);
        server6.poll(1000);
        auto child6 = server6.begin();
        assert(child6 != server6.end());
        char reply6[8]{};
        assert(child6->receive(reply6, sizeof(reply6)) == 2);
        assert((std::string_view{reply6, 2} == "hi"));
    }

    // The error helpers report a message and a code with no socket involved.
    assert(hsd::network_detail::sock_error_msg() != nullptr);
    (void)hsd::network_detail::sock_error_code();

    // A missing address aborts, and so does an address family that is not IP.
    test_support::expect_abort([]
    {
        (void)hsd::network_detail::to_str(nullptr);
    });
#if defined(__unix__) || defined(__APPLE__)
    test_support::expect_abort([]
    {
        sockaddr_storage unix_address{};
        unix_address.ss_family = AF_UNIX;
        (void)hsd::network_detail::to_str(&unix_address);
    });
#endif
    return 0;
}
