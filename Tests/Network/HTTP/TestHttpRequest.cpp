#include <cassert>

#include <Extra/HTTP/HttpRequest.hpp>

using namespace hsd::string_view_literals;

static constexpr auto request_text =
    "GET /status HTTP/1.1\r\n"
    "Host: example.test\r\n"
    "Accept: text/plain\r\n"
    "\r\n"_sv;

int main()
{
    hsd::http::request request;
    const auto consumed = request.parse(request_text).unwrap();

    assert(consumed > 0);
    assert(request.get_method() == hsd::http::request_method::get);
    assert(request.get_version() == hsd::http::protocol_version::ver_1_1);
    assert(request.get_path() == "/status"_sv);
    assert(request.get_header().get_field("host"_sv).unwrap().get() == "example.test"_sv);
    assert(request.get_header().get_field("ACCEPT"_sv).unwrap().get() == "text/plain"_sv);

    hsd::http::request invalid;
    assert(!invalid.parse("GET /status HTTP/1.1\n"_sv).is_ok());

    hsd::http::request built;
    built.set_method(hsd::http::request_method::post)
        .set_path("/items"_sv)
        .set_version(hsd::http::protocol_version::ver_1_0)
        .set_header_field("X-Request-ID"_sv, "42"_sv);
    assert(built.get_method() == hsd::http::request_method::post);
    assert(built.get_path() == "/items"_sv);
    assert(built.get_version() == hsd::http::protocol_version::ver_1_0);
    assert(built.get_header().get_field("x-request-id"_sv).unwrap().get() == "42"_sv);

    built.clear();
    assert(built.get_path().length() == 0);
    assert(!built.get_header().get_field("x-request-id"_sv).is_ok());
    assert(built.get_method() == hsd::http::request_method::post);
}
