// Title: Parsing an HTTP request
// Level: Basic
// Shows: http::request::parse, reading the method, path, version and header fields, and a rejected request.

#include <Extra/HTTP/HttpRequest.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

static const char* method_name(hsd::http::request_method method)
{
    switch (method)
    {
        case hsd::http::request_method::get: return "GET";
        case hsd::http::request_method::post: return "POST";
        case hsd::http::request_method::put: return "PUT";
        case hsd::http::request_method::del: return "DELETE";
        default: return "other";
    }
}

int main()
{
    // HTTP lines end with CRLF, and an empty line ends the header block.
    auto wire =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.test\r\n"
        "Accept: text/plain\r\n"
        "\r\n"_sv;

    hsd::http::request request;
    auto parsed = request.parse(wire);

    if (!parsed.is_ok())
    {
        hsd::println("parse failed"_fmt);
        return 0;
    }

    hsd::println("method: {}"_fmt, method_name(request.get_method()));
    hsd::println("path: {}"_fmt, request.get_path());
    hsd::println("http 1.1: {}"_fmt,
        request.get_version() == hsd::http::protocol_version::ver_1_1 ? "true" : "false");

    // Header names are case-insensitive, so "HOST" finds the "Host" field.
    auto host = request.get_header().get_field("HOST"_sv);
    hsd::println("host: {}"_fmt, host.unwrap().get());

    auto agent = request.get_header().get_field("user-agent"_sv);
    hsd::println("user-agent present: {}"_fmt, agent.is_ok() ? "true" : "false");

    // A first line that ends with a bare LF is not valid HTTP.
    hsd::http::request invalid;
    auto rejected = invalid.parse("GET / HTTP/1.1\n"_sv);
    hsd::println("bare LF accepted: {}"_fmt, rejected.is_ok() ? "true" : "false");

    return 0;
}
