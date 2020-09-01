#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "../../../cpp/Io.hpp"

int main()
{
    //	Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        return 1;
    }

    //	Create a hint structure for the server we're connecting with
    int port = 54000;
    const char* ipAddress = "127.0.0.1";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &hint.sin_addr);

    //	Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        return 1;
    }

    //	While loop:
    char buf[4096];
    hsd::u8string userInput;

    while(true)
    {
        //		Enter lines of text
        hsd::io::print("> ");
        userInput = hsd::move(hsd::io::read_line().to_string());

        //		Send to server
        int sendRes = send(sock, userInput.data(), userInput.size() + 1, 0);
        if (sendRes == -1)
        {
            hsd::io::print("Could not send to server! Whoops!\n");
            continue;
        }

        //		Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == -1)
        {
            hsd::io::print("There was an error getting response from server!\n");
        }
        else
        {
            //		Display response
            hsd::io::print("SERVER> {}", hsd::u8string(buf, bytesReceived));
        }
    }

    //	Close the socket
    close(sock);

    return 0;
}