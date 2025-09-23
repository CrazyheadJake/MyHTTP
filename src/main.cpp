#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Parser.h"
#include "HTTPRequest.h"
#include "Server.h"

const int PORT = 8080;

int main() {
    // Creating socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding socket.
    bind(serverSocket, (struct sockaddr*)&serverAddress,
         sizeof(serverAddress));

    // Listening to the assigned socket
    listen(serverSocket, 5);

    // Accepting connection request
    int clientSocket
        = accept(serverSocket, nullptr, nullptr);

    // Receiving message from client
    std::string buffer;
    while (buffer.find("\r\n\r\n") == std::string::npos) {
        char temp[1024];
        int bytes = recv(clientSocket, temp, sizeof(temp), 0);
        if (bytes <= 0) break;
        buffer.append(temp, bytes);
    }

    auto request = Parser::parseRequest(buffer);
    if (!request)
        std::cerr << "Failed to parse HTTP request" << std::endl;
    else {
        std::cout << "Parsed HTTP request:" << std::endl;
        std::cout << request->toString() << std::endl;
    }

    Server server(PORT, ".");
    auto response = server.handleRequest(*request);
    std::string responseStr = response.toString();
    send(clientSocket, responseStr.c_str(), responseStr.size(), 0);

    // Closing the socket.
    close(serverSocket);
}