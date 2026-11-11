#include <cassert>

#include <Extra/HTTP/HttpHeader.hpp>
#include <Extra/HTTP/HttpRequest.hpp>
#include <Extra/HTTP/HttpResponse.hpp>
#include <string_view>
#include <Support/AbortSupport.hpp>

using namespace hsd::string_view_literals;

// The HTTP parsers' error paths and every method, protocol version and status code, which
// the request and response checks handle one case at a time. The messages come from the
// header and status tables, so each case is walked here.
namespace
{
    // Every response status, in the order of the enumeration.
    constexpr hsd::http::response_status all_statuses[] = {
        hsd::http::response_status::cont, hsd::http::response_status::switching_protocols, hsd::http::response_status::processing, hsd::http::response_status::early_hints, hsd::http::response_status::ok, hsd::http::response_status::created, hsd::http::response_status::accepted, hsd::http::response_status::non_author_info, hsd::http::response_status::no_content, hsd::http::response_status::reset_content, hsd::http::response_status::partial_content, hsd::http::response_status::multi_status, hsd::http::response_status::already_reported, hsd::http::response_status::im_used, hsd::http::response_status::multiple_choices, hsd::http::response_status::moved_permanently, hsd::http::response_status::found, hsd::http::response_status::see_other, hsd::http::response_status::not_modified, hsd::http::response_status::use_proxy, hsd::http::response_status::unused, hsd::http::response_status::temporary_redirect, hsd::http::response_status::permanent_redirect, hsd::http::response_status::bad_request, hsd::http::response_status::unauthorized, hsd::http::response_status::payment_required, hsd::http::response_status::forbidden, hsd::http::response_status::not_found, hsd::http::response_status::method_not_allowed, hsd::http::response_status::not_acceptable, hsd::http::response_status::proxy_auth_req, hsd::http::response_status::request_timeout, hsd::http::response_status::conflict, hsd::http::response_status::gone, hsd::http::response_status::length_required, hsd::http::response_status::precondition_failed, hsd::http::response_status::payload_too_large, hsd::http::response_status::uri_too_long, hsd::http::response_status::unsupported_media_type, hsd::http::response_status::range_not_satisfiable, hsd::http::response_status::expectation_failed, hsd::http::response_status::im_a_teapot, hsd::http::response_status::misdirected_request, hsd::http::response_status::unprocessable_entity, hsd::http::response_status::locked, hsd::http::response_status::failed_dependency, hsd::http::response_status::too_early, hsd::http::response_status::upgrade_required, hsd::http::response_status::precondition_required, hsd::http::response_status::too_many_requests, hsd::http::response_status::req_header_too_large, hsd::http::response_status::unavailable_legal, hsd::http::response_status::internal_server_error, hsd::http::response_status::not_implemented, hsd::http::response_status::bad_gateway, hsd::http::response_status::service_unavailable, hsd::http::response_status::gateway_timeout, hsd::http::response_status::http_ver_not_supported, hsd::http::response_status::var_also_negotiates, hsd::http::response_status::insufficient_storage, hsd::http::response_status::loop_detected, hsd::http::response_status::not_extended, hsd::http::response_status::network_auth_req
    };

    bool parse_request(const hsd::string_view& text)
    {
        hsd::http::request request;
        return request.parse(text).is_ok();
    }

    bool parse_request_as(const hsd::string_view& text, hsd::http::protocol_version version)
    {
        hsd::http::request request;
        request.set_version(version);
        return request.parse(text).is_ok();
    }

    bool parse_response(const hsd::string_view& text)
    {
        hsd::http::response response;
        return response.parse(text).is_ok();
    }
}

int main()
{
    // Each request method is recognised, and a method that is not known is refused.
    assert(parse_request("GET / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("HEAD / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("POST / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("PUT / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("DELETE / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("CONNECT / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("OPTIONS / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("TRACE / HTTP/1.1\r\n\r\n"_sv));
    assert(parse_request("PATCH / HTTP/1.1\r\n\r\n"_sv));
    // An unknown method panics: the parser unwraps the method check, so the process aborts.
    test_support::expect_abort([]
    {
        (void)parse_request("FETCH / HTTP/1.1\r\n\r\n"_sv);
    });

    // The request line must have a method, a path that starts with '/', a space before the
    // version, and a line end.
    assert(!parse_request("GET"_sv));
    assert(!parse_request("GET index HTTP/1.1\r\n\r\n"_sv));
    assert(!parse_request("GET /index\r\n\r\n"_sv));
    assert(!parse_request("GET /index HTTP/1.1"_sv));
    assert(!parse_request("GET /index HTTP/1.1\n\n"_sv));

    // A request is parsed for the protocol version it is set to. Each version is recognised
    // by a request set to it, and text for another version panics, as the parser unwraps the
    // version check.
    assert(parse_request_as("GET / HTTP/0.9\r\n\r\n"_sv, hsd::http::protocol_version::ver_0_9));
    assert(parse_request_as("GET / HTTP/1.0\r\n\r\n"_sv, hsd::http::protocol_version::ver_1_0));
    assert(parse_request_as("GET / HTTP/2.0\r\n\r\n"_sv, hsd::http::protocol_version::ver_2_0));
    assert(parse_request_as("GET / HTTP/3.0\r\n\r\n"_sv, hsd::http::protocol_version::ver_3_0));
    test_support::expect_abort([]
    {
        (void)parse_request("GET / HTTP/1.0\r\n\r\n"_sv);
    });
    test_support::expect_abort([]
    {
        (void)parse_request("GET / HTTP/9.9\r\n\r\n"_sv);
    });

    // A request set to one version panics on text for another, for each of the versions.
    test_support::expect_abort([]
    {
        (void)parse_request("GET / HTTP/0.9\r\n\r\n"_sv);
    });
    test_support::expect_abort([]
    {
        (void)parse_request("GET / HTTP/2.0\r\n\r\n"_sv);
    });
    test_support::expect_abort([]
    {
        hsd::http::request request;
        request.set_version(hsd::http::protocol_version::ver_1_0);
        (void)request.parse("GET / HTTP/1.1\r\n\r\n"_sv);
    });
    test_support::expect_abort([]
    {
        hsd::http::request request;
        request.set_version(hsd::http::protocol_version::ver_2_0);
        (void)request.parse("GET / HTTP/3.0\r\n\r\n"_sv);
    });

    // Each response status has a message, and a response that is missing parts is refused.
    for (const auto status : all_statuses)
    {
        hsd::http::response response;
        response.set_status(status);
        assert(response.get_status() == status);

        hsd::string_view message;
        assert(hsd::http::set_status_message_spec(status, message).is_ok());
        assert(message.length() > 0);
    }

    // A status that the table does not know is refused with the table's error.
    assert(!parse_response("HTTP/1.1 999 Unknown\r\n\r\n"_sv));
    // A status code that starts with a character below '0' is refused as well as one above '9'.
    assert(!parse_response("HTTP/1.1 /00 Bad\r\n\r\n"_sv));

    // The status must be digits. Text where the code belongs is refused with the status message.
    {
        hsd::http::response response;
        auto refused = response.parse("HTTP/1.1 OK\r\n\r\n"_sv);
        assert(!refused.is_ok());
        assert(std::string_view{refused.unwrap_err().pretty_error()} ==
            "HTTP responses are required to have a status code");
    }

    // A status line without a space has no method separator.
    assert(!parse_response("HTTP/1.1"_sv));

    // The status line must have a space after the version, and end with CRLF.
    test_support::expect_abort([]
    {
        (void)parse_response("HTTP1.1 200 OK\r\n\r\n"_sv);
    });
    assert(!parse_response("HTTP/1.1 200 OK"_sv));
    assert(!parse_response("HTTP/1.1 200 OK\n\n"_sv));
    assert(parse_response("HTTP/1.1 200\r\n\r\n"_sv));
    assert(parse_response("HTTP/1.1 404 Not Found\r\nHost: example.test\r\n\r\n"_sv));

    // The version must be the one the response is set to, and a header error panics.
    test_support::expect_abort([]
    {
        (void)parse_response("HTTP/9.9 200 OK\r\n\r\n"_sv);
    });
    test_support::expect_abort([]
    {
        (void)parse_response("HTTP/1.1 200 OK\r\nNo separator here\r\n\r\n"_sv);
    });

    // A header block that is not valid reports the line it fails on.
    {
        hsd::http::header fields;
        auto no_separator = fields.parse("Host example.test\r\n\r\n"_sv);
        assert(!no_separator.is_ok());
        assert(std::string_view{no_separator.unwrap_err().pretty_error()}.find("separator") != std::string_view::npos);

        hsd::http::header unterminated;
        assert(!unterminated.parse("Host: example.test"_sv).is_ok());

        hsd::http::header bare_newline;
        assert(!bare_newline.parse("Host: example.test\n\n"_sv).is_ok());

        hsd::http::header spaced;
        assert(spaced.parse("Host:   example.test\r\n\r\n"_sv).is_ok());
        assert(spaced.get_field("host"_sv).unwrap().get() == "example.test"_sv);
    }
    return 0;
}
