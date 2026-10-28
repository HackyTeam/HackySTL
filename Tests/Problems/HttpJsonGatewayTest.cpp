#include <cassert>
#include <cstdio>

#include <Extra/HTTP/HttpRequest.hpp>
#include <Extra/HTTP/HttpResponse.hpp>
#include <Extra/Json.hpp>

using namespace hsd::string_view_literals;

static hsd::unique_ptr<hsd::JsonValue> parse_json(hsd::string_view input)
{
    hsd::JsonStream stream;
    stream.lex(input).unwrap();
    stream.push_eot().unwrap();
    hsd::JsonParser parser = stream;
    return parser.parse_next().unwrap();
}

int main()
{
    constexpr auto request_body = R"json({
        "order_id": "A-17",
        "items": [
            {"sku": "tea", "quantity": 2},
            {"sku": "cake", "quantity": 1}
        ]
    })json"_sv;

    char request_buffer[512]{};
    const int request_length = std::snprintf(
        request_buffer,
        sizeof(request_buffer),
        "POST /orders HTTP/1.1\r\n"
        "Host: shop.example.test\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n%s",
        request_body.length(), request_body.data()
    );
    assert(request_length > 0);
    assert(static_cast<hsd::usize>(request_length) < sizeof(request_buffer));

    const hsd::string_view request_wire{
        request_buffer, static_cast<hsd::usize>(request_length)
    };
    hsd::http::request request;
    assert(request.parse(request_wire).is_ok());
    assert(request.get_method() == hsd::http::request_method::post);
    assert(request.get_path() == "/orders"_sv);
    assert(request.get_header().get_field("content-type"_sv)
        .unwrap().get() == "application/json"_sv);

    const auto request_separator = request_wire.find("\r\n\r\n"_sv);
    assert(request_separator != hsd::string_view::npos);
    const auto body_offset = request_separator + 4;
    auto order = parse_json({
        request_wire.data() + body_offset,
        request_wire.length() - body_offset
    });
    assert((*order)["order_id"_sv].as_str().unwrap() == "A-17"_sv);
    assert((*order)["items"_sv].as_array().size() == 2);
    assert((*order)["items"_sv][0]["quantity"_sv]
        .as_num<hsd::i32>().unwrap() == 2);

    constexpr auto response_body = R"json({
        "order_id": "A-17",
        "accepted": true,
        "total_cents": 1200
    })json"_sv;
    char response_buffer[512]{};
    const int response_length = std::snprintf(
        response_buffer,
        sizeof(response_buffer),
        "HTTP/1.1 201 Created\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n\r\n%s",
        response_body.length(), response_body.data()
    );
    assert(response_length > 0);
    assert(static_cast<hsd::usize>(response_length) < sizeof(response_buffer));

    const hsd::string_view response_wire{
        response_buffer, static_cast<hsd::usize>(response_length)
    };
    hsd::http::response response;
    assert(response.parse(response_wire).is_ok());
    assert(response.get_status() == hsd::http::response_status::created);
    assert(response.get_header().get_field("content-type"_sv)
        .unwrap().get() == "application/json"_sv);

    const auto response_separator = response_wire.find("\r\n\r\n"_sv);
    assert(response_separator != hsd::string_view::npos);
    const auto response_body_offset = response_separator + 4;
    auto result = parse_json({
        response_wire.data() + response_body_offset,
        response_wire.length() - response_body_offset
    });
    assert((*result)["order_id"_sv].as_str().unwrap() == "A-17"_sv);
    assert((*result)["accepted"_sv].as_bool().unwrap());
    assert((*result)["total_cents"_sv].as_num<hsd::i32>().unwrap() == 1200);
}