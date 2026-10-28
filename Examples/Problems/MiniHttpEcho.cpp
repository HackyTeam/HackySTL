// Title: Mini HTTP echo
// Level: Advanced
// Shows: HTTP requests and responses carried over a loopback UDP socket pair inside one process.

#include <Containers/String.hpp>
#include <Containers/StringView.hpp>
#include <Extra/HTTP/HttpRequest.hpp>
#include <Extra/HTTP/HttpResponse.hpp>
#include <Network/Network.hpp>
#include <Serialize/Io.hpp>
#include <Threading/Thread.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

using udp_server_socket = hsd::network_detail::udp_socket<
    hsd::network_detail::ip_protocol::ipv4>;

// The http classes parse wire text but do not build it, so the examples
// assemble the text by hand. Content-Length is set here too.
static hsd::string make_request(const char* method, const char* path, const char* body)
{
    const hsd::string text{body};
    return hsd::string{method} + " " + path + " HTTP/1.1\r\n"
        "Host: loopback.test\r\n"
        "Content-Length: " + hsd::to_string(text.length()) + "\r\n\r\n" + text;
}

static hsd::string make_response(const char* status_line, hsd::string_view body)
{
    return hsd::string{"HTTP/1.1 "} + status_line + "\r\n"
        "Content-Length: " + hsd::to_string(body.length()) + "\r\n\r\n" + body;
}

// The body is everything after the blank line that ends the headers.
static hsd::string_view body_of(hsd::string_view wire)
{
    const auto separator = wire.find("\r\n\r\n"_sv);
    if (separator == hsd::string_view::npos)
    {
        return {};
    }
    return {wire.data() + separator + 4, wire.length() - separator - 4};
}

// Reads one datagram, polling the non-blocking socket for a short while.
template <typename Socket>
static hsd::isize receive_text(Socket& socket, char* buffer, hsd::usize size)
{
    hsd::isize received = -1;
    for (int attempt = 0; attempt < 2000 && received < 0; ++attempt)
    {
        received = socket.receive(buffer, size);
        if (received < 0)
        {
            hsd::this_thread::sleep_for(0.001f).unwrap();
        }
    }
    return received;
}

// Routes one parsed request. The echo route returns whatever body it received.
// (Bodies stay string_views: hsd::string{ptr, size} counts the terminator slot in size.)
static hsd::string handle(hsd::http::request& request, hsd::string_view wire)
{
    if (request.get_path() == "/hello"_sv)
    {
        return make_response("200 OK", "hello from the server"_sv);
    }
    if (request.get_path() == "/echo"_sv &&
        request.get_method() == hsd::http::request_method::post)
    {
        return make_response("201 Created", body_of(wire));
    }
    return make_response("404 Not Found", "no such route"_sv);
}

int main()
{
#if defined(HSD_PLATFORM_WINDOWS)
    hsd::network_detail::init_winsock();
#endif

    // Port 0 asks the OS for a free port, so nothing is hard-coded.
    udp_server_socket server{"127.0.0.1:0", true};
    sockaddr_in local_address{};
#if defined(HSD_PLATFORM_WINDOWS)
    int address_length = sizeof(local_address);
#else
    socklen_t address_length = sizeof(local_address);
#endif
    if (::getsockname(server.fd(), reinterpret_cast<sockaddr*>(&local_address),
        &address_length) != 0)
    {
        hsd::println("could not read the server address"_fmt);
        return 0;
    }

    const hsd::string address = hsd::string{"127.0.0.1:"} +
        hsd::to_string(static_cast<unsigned>(ntohs(local_address.sin_port)));
    hsd::udp_client_v4 client{address.c_str()};

    const char* requests[][3] = {
        {"GET", "/hello", ""},
        {"POST", "/echo", "ping pong"},
        {"GET", "/missing", ""}
    };

    for (const auto& item : requests)
    {
        const hsd::string wire = make_request(item[0], item[1], item[2]);
        client.send(wire.c_str(), wire.length());

        // Server side: receive, parse, route and answer.
        char request_buffer[512]{};
        const auto request_size = receive_text(server, request_buffer, sizeof(request_buffer));
        if (request_size < 0)
        {
            hsd::println("server timed out"_fmt);
            continue;
        }

        const hsd::string_view request_wire{
            request_buffer, static_cast<hsd::usize>(request_size)};
        hsd::http::request request;
        auto parsed = request.parse(request_wire);
        hsd::println("server: {} {}"_fmt, item[0], request.get_path());
        if (!parsed.is_ok())
        {
            continue;
        }

        const hsd::string answer = handle(request, request_wire);
        server.send(answer.c_str(), answer.length());

        // Client side: read the reply and show status and body.
        char response_buffer[512]{};
        const auto response_size = receive_text(client, response_buffer, sizeof(response_buffer));
        if (response_size < 0)
        {
            hsd::println("client timed out"_fmt);
            continue;
        }

        const hsd::string_view response_wire{
            response_buffer, static_cast<hsd::usize>(response_size)};
        hsd::http::response response;
        if (!response.parse(response_wire).is_ok())
        {
            hsd::println("client: bad response"_fmt);
            continue;
        }

        hsd::println("client: {} \"{}\""_fmt,
            static_cast<hsd::u16>(response.get_status()), body_of(response_wire));
    }
}
