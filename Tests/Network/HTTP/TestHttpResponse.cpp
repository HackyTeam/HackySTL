#include <cassert>

#include <Extra/HTTP/HttpResponse.hpp>

using namespace hsd::string_view_literals;

static constexpr auto response_text =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 5\r\n"
    "\r\nHello"_sv;

int main()
{
    hsd::http::response response;
    const auto consumed = response.parse(response_text).unwrap();

    assert(consumed > 0);
    assert(response.get_version() == hsd::http::protocol_version::ver_1_1);
    assert(response.get_status() == hsd::http::response_status::ok);
    assert(response.get_message().length() == 2);
    assert(response.get_message()[0] == 'O');
    assert(response.get_message()[1] == 'K');
    assert(response.get_header().get_field("content-type"_sv).unwrap().get() == "text/plain"_sv);
    assert(response.get_header().get_field("CONTENT-LENGTH"_sv).unwrap().get() == "5"_sv);

    hsd::http::response invalid;
    assert(!invalid.parse("HTTP/1.1 200 OK\n"_sv).is_ok());

    hsd::http::response built;
    built.set_status(hsd::http::response_status::not_found)
        .set_version(hsd::http::protocol_version::ver_1_0)
        .set_message("Resource missing"_sv)
        .set_header_field("Cache-Control"_sv, "no-store"_sv);
    assert(built.get_status() == hsd::http::response_status::not_found);
    assert(built.get_version() == hsd::http::protocol_version::ver_1_0);
    assert(built.get_message() == "Resource missing"_sv);
    assert(built.get_header().get_field("cache-control"_sv).unwrap().get() == "no-store"_sv);
}
