// Title: Building an HTTP response
// Level: Intermediate
// Shows: the response builder, writing the wire format by hand, and parsing it back.

#include <Extra/HTTP/HttpResponse.hpp>
#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

// The library stores and parses the pieces of a response but does not write
// the wire format, so this helper assembles the status line, headers and body.
static hsd::string to_wire(hsd::http::response& response, hsd::string_view body)
{
    hsd::string wire = "HTTP/1.1 ";
    wire += hsd::to_string(static_cast<hsd::u16>(response.get_status()));
    wire += " ";
    wire += response.get_message();
    wire += "\r\n";

    // Header names were lower-cased when they were stored.
    for (auto& field : response.get_header())
    {
        wire += field.first;
        wire += ": ";
        wire += field.second;
        wire += "\r\n";
    }

    wire += "\r\n";
    wire += body;
    return wire;
}

static hsd::string make_response(
    hsd::http::response_status status, hsd::string_view message, hsd::string_view body)
{
    hsd::http::response response;
    hsd::string length = hsd::to_string(body.length());

    // Content-Length is not filled in automatically, so it is set from the body.
    response
        .set_status(status)
        .set_message(message)
        .set_header_field("Content-Type"_sv, "text/plain"_sv)
        .set_header_field("Content-Length"_sv, static_cast<hsd::string_view>(length));

    return to_wire(response, body);
}

static void show(hsd::string_view label, const hsd::string& wire)
{
    hsd::http::response parsed;

    if (!parsed.parse(static_cast<hsd::string_view>(wire)).is_ok())
    {
        hsd::println("{}: could not parse"_fmt, label);
        return;
    }

    hsd::println("{}: status={} message={} length={}"_fmt,
        label, static_cast<hsd::u16>(parsed.get_status()), parsed.get_message(),
        parsed.get_header().get_field("content-length"_sv).unwrap().get());
}

int main()
{
    auto ok = make_response(hsd::http::response_status::ok, "OK"_sv, "Hello"_sv);
    auto missing = make_response(
        hsd::http::response_status::not_found, "Not Found"_sv, "No such page"_sv);
    auto teapot = make_response(
        hsd::http::response_status::im_a_teapot, "I'm a teapot"_sv, ""_sv);

    hsd::println("{}"_fmt, ok.c_str());
    hsd::println("--"_fmt);

    // Parsing the bytes we just wrote gives the same pieces back.
    show("ok"_sv, ok);
    show("missing"_sv, missing);
    show("teapot"_sv, teapot);

    return 0;
}
