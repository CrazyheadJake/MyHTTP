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
    /// @brief Construct a new Server object.
    /// @param port The port number to listen on.
    /// @param rootDir The root directory for serving static files.
    /// @param numThreads The number of worker threads in the thread pool. There is always only one acceptor thread.
    /// @param timeoutSeconds The timeout in seconds for idle connections.
    Server(int port, const std::filesystem::path& rootDir, int numThreads = 16, int timeoutSeconds = 30);
    /// @brief Start the server's main loop. This will block.
    void start();
    /// @brief Stop the server. This is not yet implemented.
    void stop();
    /// @brief Add a route to the server's router.
    /// @param route The route path.
    /// @param method The HTTP method(s) for this route (bitmask of HTTPRequest::Method).
    /// @param handler The handler function to process requests for this route.
    void addRoute(Route route, int method, Handler handler);
    /// @brief Handle an incoming HTTP request and generate a response.
    /// @param request The HTTP request to handle.
    /// @return The generated HTTP response.
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
    std::unordered_map<int, std::chrono::steady_clock::time_point> m_lastActiveTimes;

    void setupSocket();
    void acceptConnection();
    void handleClient(int clientSocket, uint32_t events);
    void closeConnection(int clientSocket);
    void setNonBlocking(int fd);

    void updateLastActive(int clientSocket);
    void timeOutConnections();
};
    