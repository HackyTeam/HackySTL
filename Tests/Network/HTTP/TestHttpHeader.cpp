#include <Extra/HTTP/HttpHeader.hpp>
#include <cassert>

using namespace hsd::string_view_literals;

constexpr auto req = 
    "GET / HTTP/1.1\r\n"
    "Host: 127.0.0.1:54000\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:103.0) "
    "Gecko/20100101 Firefox/103.0\r\n"
    "Accept: text/html,application/xhtml+xml,application/"
    "xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\n"
    "Accept-Language: en-US,en;q=0.5\r\n"
    "Accept-Encoding: gzip, deflate, br\r\n"
    "DNT: 1\r\n"
    "Connection: keep-alive\r\n"
    "Upgrade-Insecure-Requests: 1\r\n"
    "Sec-Fetch-Dest: document\r\n"
    "Sec-Fetch-Mode: navigate\r\n"
    "Sec-Fetch-Site: none\r\n"
    "Sec-Fetch-User: ?1\r\n"
    "Sec-GPC: 1\r\n"
    "\r\n"_sv;

int main()
{
    hsd::http::header body;

    constexpr auto header_pos = req.find("\r\n"_sv) + 2;
    constexpr hsd::string_view header = {
        req.data() + header_pos, req.length() - header_pos
    };

    assert(body.parse(header).unwrap() == header.length() - 2);
    assert(body.get_field("hOsT"_sv).unwrap().get() == "127.0.0.1:54000"_sv);
    assert(body.get_field("user-agent"_sv).is_ok());
    assert(!body.get_field("missing"_sv).is_ok());

    body.set_field("X-Trace"_sv, "first"_sv);
    body.set_field("x-trace"_sv, "second"_sv);
    assert(body.get_field("X-TRACE"_sv).unwrap().get() == "second"_sv);
    body.clear();
    assert(!body.get_field("host"_sv).is_ok());

    hsd::http::header malformed;
    assert(!malformed.parse("missing-separator\r\n\r\n"_sv).is_ok());

    return 0;
}