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

class Server {
public:
    Server(int port, const std::filesystem::path& rootDir, int numThreads = 16);
    void start();
    void stop();
    HTTPResponse handleRequest(const std::optional<HTTPRequest>& request);
private:
    int m_port;
    std::filesystem::path m_rootDir;
    bool m_running = false;
    int m_serverSocket = -1;
    int m_epollFd = -1;
    WorkerPool m_pool;
    sockaddr_in m_serverAddress;

    void setupSocket();
    void acceptConnection();
    void handleClient(int clientSocket, uint32_t events);
    void setNonBlocking(int fd);
};
    