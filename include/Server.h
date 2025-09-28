#pragma once

#include <string>
#include <filesystem>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "ResponseGenerator.h"
#include <optional>
#include <netinet/in.h>
#include <sys/socket.h>
#include "WorkerPool.h"
#include "Router.h"
#include <chrono>

struct ClientConnection {
    int socket;
    std::chrono::steady_clock::time_point lastActive;
    bool operator>(const ClientConnection& other) const {
        return lastActive > other.lastActive;
    }
};

class Server {
public:
    Server(int port, const std::filesystem::path& rootDir, int numThreads = 16, int timeoutSeconds = 10);
    void start();
    void stop();
    void addRoute(Route route, int method, Handler handler);
    HTTPResponse handleRequest(const std::optional<HTTPRequest>& request) const;

private:
    int m_timeoutSeconds;
    int m_port;
    bool m_running = false;
    int m_serverSocket = -1;
    int m_epollFd = -1;
    WorkerPool m_pool;
    Router m_router;
    sockaddr_in m_serverAddress;
    std::priority_queue<ClientConnection, std::vector<ClientConnection>, std::greater<ClientConnection>> m_activeConnections;

    void setupSocket();
    void acceptConnection();
    void handleClient(int clientSocket, uint32_t events);
    void setNonBlocking(int fd);
    void timeOutConnections();
};
    