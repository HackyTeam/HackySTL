#include <cassert>
#include <cstdio>
#include <string_view>

#include <Extra/HTTP/HttpSocket.hpp>
#include <Network/Network.hpp>
#include <Support/AbortSupport.hpp>

using namespace hsd::string_view_literals;

// An HTTP exchange over the loopback address, with the client and the server in one process:
// the client writes a request, the server reads it from its accepted socket and answers, and
// the client reads the answer. Each side is also driven without a socket, and with a socket
// that has nothing to read or that fails.
namespace
{
    using protocol = hsd::network_detail::ip_protocol;
    using child_socket = hsd::network_detail::tcp_child_socket<protocol::ipv4>;

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
}

int main()
{
    char address[64]{};
    std::snprintf(address, sizeof(address), "127.0.0.1:%u", free_port());

    hsd::tcp_server_v4 server{address};
    hsd::tcp_client_v4 client{address};

    // The client sends a request with a header field.
    hsd::http::socket<hsd::tcp_client_v4> client_http;
    client_http.reserve_buffer(1024);
    client_http.set_current_sock(client);
    client_http.set_version(hsd::http::protocol_version::ver_1_1);
    client_http.get_request()
        .set_method(hsd::http::request_method::get)
        .set_path("/status"_sv)
        .set_header_field("Host"_sv, "example.test"_sv);
    assert(client_http.send_request().is_ok());

    // The server accepts the connection, and reads the request from its child socket.
    server.poll(1000);
    server.poll(1000);
    auto child = server.begin();
    assert(child != server.end());

    // Reads fill the buffer's free space, so a buffer must be reserved before receiving.
    hsd::http::socket<child_socket> server_http;
    server_http.reserve_buffer(1024);
    server_http.set_current_sock(*child);
    server_http.set_version(hsd::http::protocol_version::ver_1_1);
    auto rest = server_http.receive_request();
    assert(rest.is_ok());
    // The header parser stops at the blank line that ends the headers, so that line is what is left.
    assert(rest.unwrap() == "\r\n"_sv);
    assert(server_http.get_request().get_method() == hsd::http::request_method::get);
    assert(server_http.get_request().get_path() == "/status"_sv);
    assert(server_http.get_request().get_header().get_field("host"_sv).unwrap().get() == "example.test"_sv);

    // The server answers with a status and a body, and the client reads them back.
    server_http.get_response().set_status(hsd::http::response_status::ok);
    assert(server_http.send_response("hello"_sv).is_ok());

    auto answer = client_http.receive_response();
    assert(answer.is_ok());
    assert(client_http.get_response().get_status() == hsd::http::response_status::ok);
    // As with the request, the header parser leaves the blank line in front of the body.
    assert(answer.unwrap() == "\r\nhello"_sv);

    // Without a socket, every exchange reports that there is nothing to use.
    hsd::http::socket<hsd::tcp_client_v4> detached;
    assert(!detached.send_request().is_ok());
    assert(!detached.send_response("x"_sv).is_ok());
    assert(!detached.receive_request().is_ok());
    assert(!detached.receive_response().is_ok());

    // A request that has no method separator is refused with the parser's message.
    hsd::tcp_client_v4 second_client{address};
    hsd::http::socket<hsd::tcp_client_v4> writer;
    writer.set_current_sock(second_client);
    assert(second_client.send("GET\r\n\r\n", 7) == 7);
    server.poll(1000);
    server.poll(1000);
    // The first child is the one accepted earlier, so the second is the next one listed.
    auto second_child = server.begin();
    ++second_child;
    assert(second_child != server.end());
    hsd::http::socket<child_socket> refusing;
    refusing.reserve_buffer(1024);
    refusing.set_current_sock(*second_child);
    refusing.set_version(hsd::http::protocol_version::ver_1_1);
    auto malformed = refusing.receive_request();
    assert(!malformed.is_ok());
    assert(std::string_view{malformed.unwrap_err().pretty_error()} == "Method separator not found");

    // Every method and every protocol version is written in its own text.
    const hsd::http::protocol_version versions[] = {
        hsd::http::protocol_version::ver_0_9, hsd::http::protocol_version::ver_1_0,
        hsd::http::protocol_version::ver_1_1, hsd::http::protocol_version::ver_2_0,
        hsd::http::protocol_version::ver_3_0,
    };
    const hsd::http::request_method methods[] = {
        hsd::http::request_method::get, hsd::http::request_method::head,
        hsd::http::request_method::post, hsd::http::request_method::put,
        hsd::http::request_method::del, hsd::http::request_method::connect,
        hsd::http::request_method::options, hsd::http::request_method::trace,
        hsd::http::request_method::patch,
    };
    for (const auto version : versions)
    {
        for (const auto method : methods)
        {
            client_http.set_version(version);
            client_http.get_request().set_method(method).set_path("/"_sv);
            assert(client_http.send_request().is_ok());
        }
    }

    // The second poll may have accepted a child, and the children are stored in a growing list
    // that moves them, so the first child is looked up again before the socket uses it.
    child = server.begin();
    server_http.set_current_sock(*child);

    // A response with a header field writes the field before the body.
    server_http.get_response().set_header_field("Content-Type"_sv, "text/plain"_sv);
    assert(server_http.send_response("x"_sv).is_ok());

    // A child with no data yet gives an empty request, and a child that is gone reports the failure.
    child_socket idle{pollfd{::socket(AF_INET, SOCK_STREAM, 0), 0, 0}};
    hsd::http::socket<child_socket> quiet;
    quiet.reserve_buffer(64);
    quiet.set_current_sock(idle);
    assert(quiet.receive_request().unwrap() == ""_sv);

    child_socket gone{pollfd{static_cast<hsd::network_detail::native_socket_type>(-1), 0, 0}};
    hsd::http::socket<child_socket> absent;
    absent.reserve_buffer(64);
    absent.set_current_sock(gone);
    assert(!absent.receive_request().is_ok());

    // A response that has no method separator is refused with the parser's message.
    assert(second_child->send("NOPE\r\n\r\n", 8) == 8);
    hsd::http::socket<hsd::tcp_client_v4> reader;
    reader.reserve_buffer(1024);
    reader.set_current_sock(second_client);
    auto bad_response = reader.receive_response();
    assert(!bad_response.is_ok());
    assert(std::string_view{bad_response.unwrap_err().pretty_error()} == "Method separator not found");

    // A socket that has failed cannot write or read. Its port has no listener, so the connect fails.
    char closed_address[64]{};
    std::snprintf(closed_address, sizeof(closed_address), "127.0.0.1:%u", free_port());
    hsd::tcp_client_v4 refused{closed_address};
    hsd::http::socket<hsd::tcp_client_v4> failing;
    failing.reserve_buffer(1024);
    failing.set_current_sock(refused);
    assert(!failing.receive_response().is_ok());

    // A request method outside the enumeration cannot be written, and the writer panics.
    test_support::expect_abort([]
    {
        hsd::tcp_client_v4 method_client{"127.0.0.1:1"};
        hsd::http::socket<hsd::tcp_client_v4> method_writer;
        method_writer.set_current_sock(method_client);
        method_writer.get_request().set_method(static_cast<hsd::http::request_method>(42));
        (void)method_writer.send_request();
    });

    // An unknown protocol version cannot be written, and the writer panics.
    test_support::expect_abort([]
    {
        hsd::tcp_client_v4 unknown_client{"127.0.0.1:1"};
        hsd::http::socket<hsd::tcp_client_v4> unknown;
        unknown.set_current_sock(unknown_client);
        unknown.set_version(static_cast<hsd::http::protocol_version>(9));
        (void)unknown.send_request();
    });
    return 0;
}
