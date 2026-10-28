#include <Serialize/Io.hpp>
#include <Extra/HTTP/HttpSocket.hpp>
#include <Network/Network.hpp>
#include <format>

using namespace hsd::format_literals;
static inline bool manage_socket(auto& socket);

int main()
{
    hsd::tcp_server_v4 server = "0.0.0.0:55000";
    using socket_type = decltype(server)::socket_type;
    hsd::http::socket<socket_type> http_socket;

    http_socket
    .reserve_buffer(65536)
    .set_version(hsd::http::protocol_version::ver_1_1);

    while (true)
    {
        server.poll();

        for (auto socket = server.begin(); socket != server.end();)
        {
            http_socket.set_current_sock(*socket);

            bool to_erase = manage_socket(http_socket);

            if (to_erase)
            {
                socket = server.erase(socket).unwrap();
            }
            else
            {
                ++socket;
            }
        }
    }
}

using namespace hsd::string_view_literals;

static constexpr auto page = 
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "   <head>\r\n"
    "       <meta charset=\"UTF-8\">\r\n"
    "           <title>Test HTTP site</title>\r\n"
    "   </head>\r\n"
    "   \r\n"
    "   <script>\r\n"
    "       var xhr = new XMLHttpRequest();\r\n"
    "       xhr.onreadystatechange = function () {\r\n"
    "           if (xhr.readyState === 4) {\r\n"
    "               document.getElementById('status').innerHTML = 'Status: ' + xhr.responseText;\r\n"
    "           }\r\n"
    "       };\r\n"
    "       \r\n"
    "       var on = false;\r\n"
    "       \r\n"
    "       function toggle() {\r\n"
    "           if (on) {\r\n"
    "               xhr.open('POST', 'http://127.0.0.1:55000/status');\r\n"
    "               xhr.setRequestHeader('Accept', 'text');\r\n"
    "               xhr.setRequestHeader('Content-Type', 'text');\r\n"
    "               xhr.send('off');\r\n"
    "               on = false;\r\n"
    "           } else {\r\n"
    "               xhr.open('POST', 'http://127.0.0.1:55000/status');\r\n"
    "               xhr.setRequestHeader('Accept', 'text');\r\n"
    "               xhr.setRequestHeader('Content-Type', 'text');\r\n"
    "               xhr.send('on');\r\n"
    "               on = true;\r\n"
    "           }\r\n"
    "       }\r\n"
    "   </script>\r\n"
    "   <body style=\"background-color: rgb(52, 52, 52);\">\r\n"
    "      <h1 style=\"color: white; text-align: center;\">Test the button</h1>\r\n"
    "   \r\n"
    "      <div style=\"text-align: center; height: 800px\">\r\n"
    "          <button style=\"border-radius: 15px; width: fit-content;"
    "          padding: 0 10px; color: white; margin-top: 10px;"
    "          border-color: #9904f5; background-color: rgb(52, 52, 52);\""
    "          id=\"status\" onclick=\"toggle(); return false;\">\r\n"
    "              Status: off\r\n"
    "          </button>\r\n"
    "      </div>\r\n"
    "   </body>\r\n"
    "</html>\r\n"_sv;

static inline bool manage_socket(auto& socket)
{
    using namespace hsd::string_view_literals;
    auto& req = socket.get_request().clear();
    auto& resp = socket.get_response();
    auto data = socket.receive_request().unwrap();

    const auto length_str = std::format("{}", page.length());
    
    if (req.get_method() == hsd::http::request_method::get && req.get_path() == "/"_sv)
    {
        resp
        .set_status(hsd::http::response_status::ok)
        .set_header_field("Content-Type"_sv, "text/html"_sv)
        .set_header_field(
            "Content-Length"_sv,
            hsd::string_view{length_str.data(), length_str.size()}
        );

        socket.send_response(page).unwrap();
        return true;
    }

    if (req.get_method() == hsd::http::request_method::post && req.get_path() == "/status"_sv)
    {
        if (data.find("on") != data.npos)
        {
            resp
            .set_status(hsd::http::response_status::ok)
            .set_header_field("Content-Type"_sv, "text/html"_sv)
            .set_header_field("Content-Length"_sv, "5"_sv);

            socket.send_response("off\r\n"_sv).unwrap();
            return true;
        }
        else if (data.find("off") != data.npos)
        {
            resp
            .set_status(hsd::http::response_status::ok)
            .set_header_field("Content-Type"_sv, "text/html"_sv)
            .set_header_field("Content-Length"_sv, "4"_sv);

            socket.send_response("on\r\n"_sv).unwrap();
            return true;
        }
    }

    return false;
}