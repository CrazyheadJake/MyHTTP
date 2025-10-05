#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"

class IntegrationTest : public ::testing::Test {
protected:
    static void runServer() {
        Server server(8081, "../../public/", 16, 5);
        server.start(); // Blocking call
    }
    static std::thread serverThread;

    // Runs once before any test in this suite
    static void SetUpTestSuite() {
        serverThread = std::thread(runServer);
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Give server time to start
    }

    // Runs once after all tests in this suite
    static void TearDownTestSuite() {
        // TODO: implement a proper server shutdown signal
        serverThread.detach();
    }

    // Utility function to open a client connection
    int connectClient() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        EXPECT_NE(sock, -1);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8081);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        int ret = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        EXPECT_EQ(ret, 0);
        return sock;
    }
};

std::thread IntegrationTest::serverThread;

TEST_F(IntegrationTest, SimpleGET) {
    // Connect as a client
    int sock = connectClient();

    // Send HTTP GET request
    const char* request = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    send(sock, request, strlen(request), 0);

    char buffer[1024] = {0};
    int n = recv(sock, buffer, sizeof(buffer), 0);
    ASSERT_GT(n, 0);
    std::string response(buffer, n);

    // Simple check: status 200 OK
    EXPECT_TRUE(response.find("200 OK") != std::string::npos);    
    close(sock);
}

TEST_F(IntegrationTest, PartialData) {
    // Connect as a client
    int sock = connectClient();
    
    // Send partial HTTP request to test server's handling of incomplete data
    const char* partialRequest = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n";
    send(sock, partialRequest, strlen(partialRequest), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const char* restRequest = "\r\n";
    send(sock, restRequest, strlen(restRequest), 0);

    char buffer[1024] = {0};
    int n = recv(sock, buffer, sizeof(buffer), 0);
    ASSERT_GT(n, 0);

    // Simple check: status 200 OK
    std::string response(buffer, n);
    EXPECT_TRUE(response.find("200 OK") != std::string::npos);
}

TEST_F(IntegrationTest, StressTest) {
    const char* request = "GET /test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    int numConnections = 1000;
    int socks[numConnections];
    char buffer[1024] = {0};

    // Connect as a client
    for (int i = 0; i < numConnections; i++) {
        socks[i] = connectClient();
    }
    for (int i = 0; i < numConnections; i++) {
        // Send HTTP GET request
        send(socks[i], request, strlen(request), 0);
    }
    for (int i = 0; i < numConnections; i++) {
        std::string response;
        int n = recv(socks[i], buffer, sizeof(buffer), 0);
        response.append(buffer, n);
        while (n > 0) {
            if (response.find("EndOfTest") != std::string::npos)
                break;
            n = recv(socks[i], buffer, sizeof(buffer), 0);
            response.append(buffer, n);
        }
        // Check to see if end of message was received
        EXPECT_TRUE(response.find("EndOfTest") != std::string::npos);
        close(socks[i]);
    }
}