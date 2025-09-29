#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"

void runServer() {
    Server server(8080, "../../public/");
    server.start(); // Blocking call
}

TEST(IntegrationTest, SimpleGET) {
    // Start server in separate thread
    std::thread serverThread(runServer);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Give server time to start

    // Connect as a client
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(sock, -1);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    ASSERT_EQ(ret, 0);

    // Send HTTP GET request
    const char* request = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    send(sock, request, strlen(request), 0);

    char buffer[1024] = {0};
    int n = recv(sock, buffer, sizeof(buffer), 0);
    ASSERT_GT(n, 0);
    std::string response(buffer, n);

    // Simple check: status 200 OK
    EXPECT_TRUE(response.find("200 OK") != std::string::npos);

    // Send partial HTTP request to test server's handling of incomplete data
    const char* partialRequest = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n";
    send(sock, partialRequest, strlen(partialRequest), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const char* restRequest = "\r\n";
    send(sock, restRequest, strlen(restRequest), 0);

    n = recv(sock, buffer, sizeof(buffer), 0);
    ASSERT_GT(n, 0);
    std::string partialResponse(buffer, n);
    EXPECT_TRUE(partialResponse.find("200 OK") != std::string::npos);

    close(sock);
    serverThread.detach(); // In production, implement proper shutdown
}