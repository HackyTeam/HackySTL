// Title: JSON over HTTP exchange
// Level: Advanced
// Shows: a request and response as wire text, a handler that routes and validates, and JSON bodies parsed in both directions.

#include <Extra/HTTP/HttpRequest.hpp>
#include <Extra/HTTP/HttpResponse.hpp>
#include <Extra/Json.hpp>
#include <Containers/String.hpp>
#include <Serialize/Io.hpp>

using namespace hsd::format_literals;
using namespace hsd::string_view_literals;

static hsd::string_view view_of(const hsd::string& text)
{
    return static_cast<hsd::string_view>(text);
}

// Everything after the blank line is the message body.
static hsd::string_view body_of(hsd::string_view wire)
{
    auto separator = wire.find("\r\n\r\n"_sv);

    if (separator == hsd::string_view::npos)
        return ""_sv;

    return {wire.data() + separator + 4, wire.length() - separator - 4};
}

static hsd::unique_ptr<hsd::JsonValue> parse_json(hsd::string_view text, bool& ok)
{
    hsd::JsonStream lexer;
    ok = lexer.lex(text).is_ok() && lexer.push_eot().is_ok();

    if (!ok)
        return {};

    hsd::JsonParser parser = lexer;
    auto result = parser.parse_next();
    ok = result.is_ok();

    if (!ok)
        return {};

    return result.unwrap();
}

static hsd::string make_request(
    hsd::string_view method, hsd::string_view path, hsd::string_view body)
{
    hsd::string wire = method;
    wire += " ";
    wire += path;
    wire += " HTTP/1.1\r\nHost: shop.test\r\nContent-Type: application/json\r\nContent-Length: ";
    wire += hsd::to_string(body.length());
    wire += "\r\n\r\n";
    wire += body;
    return wire;
}

static hsd::string make_response(
    hsd::http::response_status status, hsd::string_view message, hsd::string_view body)
{
    hsd::string wire = "HTTP/1.1 ";
    wire += hsd::to_string(static_cast<hsd::u16>(status));
    wire += " ";
    wire += message;
    wire += "\r\nContent-Type: application/json\r\nContent-Length: ";
    wire += hsd::to_string(body.length());
    wire += "\r\n\r\n";
    wire += body;
    return wire;
}

static hsd::string error_response(
    hsd::http::response_status status, hsd::string_view message, hsd::string_view reason)
{
    hsd::string body = "{\"error\": \"";
    body += reason;
    body += "\"}";
    return make_response(status, message, view_of(body));
}

static hsd::i32 price_cents(hsd::string_view sku)
{
    if (sku == "tea"_sv)
        return 350;

    if (sku == "cake"_sv)
        return 500;

    return -1;
}

// The "server": parses the request text, checks it, and answers with response text.
static hsd::string handle(hsd::string_view request_wire)
{
    using hsd::http::response_status;

    hsd::http::request request;

    if (!request.parse(request_wire).is_ok())
        return error_response(response_status::bad_request, "Bad Request"_sv, "malformed request"_sv);

    if (request.get_path() != "/orders"_sv)
        return error_response(response_status::not_found, "Not Found"_sv, "unknown path"_sv);

    if (request.get_method() != hsd::http::request_method::post)
        return error_response(response_status::method_not_allowed, "Method Not Allowed"_sv, "use POST"_sv);

    bool json_ok = false;
    auto order = parse_json(body_of(request_wire), json_ok);

    if (!json_ok)
        return error_response(response_status::bad_request, "Bad Request"_sv, "invalid json"_sv);

    auto id = order->access("order_id"_sv);
    auto items = order->access("items"_sv);

    if (!id.is_ok() || !items.is_ok())
        return error_response(response_status::unprocessable_entity, "Unprocessable Entity"_sv, "missing field"_sv);

    hsd::i32 total = 0;

    for (auto& item : items.unwrap().get().as_array())
    {
        auto sku = item->access("sku"_sv);
        auto quantity = item->access("quantity"_sv);

        if (!sku.is_ok() || !quantity.is_ok())
            return error_response(response_status::unprocessable_entity, "Unprocessable Entity"_sv, "bad item"_sv);

        auto cents = price_cents(sku.unwrap().get().as_str().unwrap_or(""_sv));
        auto count = quantity.unwrap().get().as_num<hsd::i32>().unwrap_or(0);

        if (cents < 0)
            return error_response(response_status::unprocessable_entity, "Unprocessable Entity"_sv, "unknown sku"_sv);

        total += cents * count;
    }

    hsd::string body = "{\"order_id\": \"";
    body += id.unwrap().get().as_str().unwrap_or(""_sv);
    body += "\", \"accepted\": true, \"total_cents\": ";
    body += hsd::to_string(total);
    body += "}";
    return make_response(response_status::created, "Created"_sv, view_of(body));
}

// The "client": sends one request and reports what came back.
static void exchange(hsd::string_view label, hsd::string_view method,
    hsd::string_view path, hsd::string_view body)
{
    auto request_wire = make_request(method, path, body);
    auto response_wire = handle(view_of(request_wire));

    hsd::http::response response;

    if (!response.parse(view_of(response_wire)).is_ok())
    {
        hsd::println("{}: unreadable response"_fmt, label);
        return;
    }

    hsd::print("{}: {} "_fmt, label, static_cast<hsd::u16>(response.get_status()));

    bool json_ok = false;
    auto reply = parse_json(body_of(view_of(response_wire)), json_ok);

    if (!json_ok)
    {
        hsd::println("(no json)"_fmt);
        return;
    }

    auto error = reply->access("error"_sv);

    if (error.is_ok())
    {
        hsd::println("error={}"_fmt, error.unwrap().get().as_str().unwrap_or(""_sv));
    }
    else
    {
        hsd::println("order={} accepted={} total={}"_fmt,
            (*reply)["order_id"_sv].as_str().unwrap(),
            (*reply)["accepted"_sv].as_bool().unwrap() ? "true" : "false",
            (*reply)["total_cents"_sv].as_num<hsd::i32>().unwrap());
    }
}

int main()
{
    exchange("order A-17"_sv, "POST"_sv, "/orders"_sv,
        R"json({"order_id": "A-17", "items": [{"sku": "tea", "quantity": 2}, {"sku": "cake", "quantity": 1}]})json"_sv);
    exchange("order B-02"_sv, "POST"_sv, "/orders"_sv,
        R"json({"order_id": "B-02", "items": []})json"_sv);
    exchange("unknown sku"_sv, "POST"_sv, "/orders"_sv,
        R"json({"order_id": "C-9", "items": [{"sku": "pie", "quantity": 1}]})json"_sv);
    exchange("missing items"_sv, "POST"_sv, "/orders"_sv,
        R"json({"order_id": "D-1"})json"_sv);
    exchange("broken body"_sv, "POST"_sv, "/orders"_sv, "{\"order_id\": "_sv);
    exchange("wrong method"_sv, "GET"_sv, "/orders"_sv, ""_sv);
    exchange("wrong path"_sv, "POST"_sv, "/basket"_sv, "{}"_sv);

    return 0;
}
