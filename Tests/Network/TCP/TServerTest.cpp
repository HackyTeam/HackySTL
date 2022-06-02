#include <Network.hpp>
#include <Io.hpp>

using namespace hsd::format_literals;
static inline auto manage_socket(auto& socket)
    -> hsd::pair<bool, hsd::isize>;

int main()
{
    hsd::tcp_server_v4 server = "0.0.0.0:54000";

    while (true)
    {
        server.poll();

        for (auto socket = server.begin(); socket != server.end();)
        {
            auto [to_erase, state] = manage_socket(*socket);

            if (state < 0)
            {
                hsd::println("Error: {}"_fmt, server.error_message());
            }
            else if (to_erase)
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

static inline auto send_html(const hsd::string_view& request, auto& socket)
    -> hsd::pair<bool, hsd::isize>
{
    static constexpr auto fmt = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: {}\r\n"
        "\r\n{}"_fmt();

    static hsd::sstream ss;

    ss.reserve(65536);
    ss.write_data<fmt>(request.size(), request);

    auto send_state = socket.send(ss.data(), ss.size());
    
    if (send_state > 0)
    {
        return {true, send_state};
    }

    return {false, -1};
}

static inline auto manage_socket(auto& socket)
    -> hsd::pair<bool, hsd::isize>
{
    using namespace hsd::string_view_literals;
    static char recv_buffer[4096]{};

    memset(recv_buffer, 0, sizeof(recv_buffer));
    auto recv_state = socket.receive(recv_buffer, sizeof(recv_buffer));
            
    if (recv_state > 0)
    {
        hsd::println("CLIENT> {}"_fmt, recv_buffer);
        
        if (auto* req = hsd::cstring::find(recv_buffer, "GET"); req != nullptr)
        {
            if (req[4] == '/')
            {
                return send_html(
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "   <head>\r\n"
                    "       <meta charset=\"UTF-8\">\r\n"
                    "           <title>Proiect Informatică Aplicată</title>\r\n"
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
                    "               xhr.open('POST', 'http://127.0.0.1:54000/status');\r\n"
                    "               xhr.setRequestHeader('Accept', 'text');\r\n"
                    "               xhr.setRequestHeader('Content-Type', 'text');\r\n"
                    "               xhr.send('off');\r\n"
                    "               on = false;\r\n"
                    "           } else {\r\n"
                    "               xhr.open('POST', 'http://127.0.0.1:54000/status');\r\n"
                    "               xhr.setRequestHeader('Accept', 'text');\r\n"
                    "               xhr.setRequestHeader('Content-Type', 'text');\r\n"
                    "               xhr.send('on');\r\n"
                    "               on = true;\r\n"
                    "           }\r\n"
                    "       }\r\n"
                    "   </script>\r\n"
                    "   <body style=\"background-color: rgb(52, 52, 52);\">\r\n"
                    "      <h1 style=\"color: white; text-align: center;\">Testează butonul</h1>\r\n"
                    "   \r\n"
                    "      <div style=\"text-align: center; height: 800px\">\r\n"
                    "          <button style=\"border-radius: 15px; width: fit-content;"
                    "          padding: 0 10px; color: white; margin-top: 10px;"
                    "          border-color: #9904f5; background-color: rgb(52, 52, 52);\""
                    "          id=\"status\" onclick=\"toggle(); return false;\">\r\n"
                    "              Status: oprit\r\n"
                    "          </button>\r\n"
                    "      </div>\r\n"
                    "   </body>\r\n"
                    "</html>\r\n"_sv,
                    socket
                );
            }
        }
        else if (auto* req = hsd::cstring::find(recv_buffer, "POST"); req != nullptr)
        {
            auto* data = hsd::cstring::find(recv_buffer, "\r\n\r\n");

            if (data != nullptr)
            {
                data += 4;

                if (auto* req = hsd::cstring::find(data, "on"); req != nullptr)
                {
                    static constexpr auto on = "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 7\r\n"
                        "Content-Type: text\r\n"
                        "\r\n"
                        "pornit\r\n"_sv;

                    return {true, socket.send(on.data(), on.size())};
                }
                else if (auto* req = hsd::cstring::find(data, "off"); req != nullptr)
                {
                    static constexpr auto off = "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 6\r\n"
                        "Content-Type: text\r\n"
                        "\r\n"
                        "oprit\r\n"_sv;

                    return {true, socket.send(off.data(), off.size())};
                }
            }
        }
    }
    
    return {false, recv_state};
}