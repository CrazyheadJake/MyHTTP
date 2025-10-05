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

    std::mutex m_lastActiveMutex;
    std::unordered_map<int, std::chrono::steady_clock::time_point> m_lastActiveTimes;

    std::mutex m_bufferMutex;
    std::unordered_map<int, std::string> m_connectionBuffers;

    void setupSocket();
    void acceptConnection();
    /// @brief This is always run in a worker thread, handles parsing and responding to a client request.
    void handleClient(int clientSocket, uint32_t events);
    /// @brief This is always run in the main epoll thread
    std::optional<std::string> receiveData(int clientSocket);
    /// @brief Close and clean up a client connection. This is only safe to call from the main epoll thread.
    /// @param clientSocket The socket file descriptor of the client to close.
    void closeConnection(int clientSocket);
    /// @brief Set a socket to non-blocking mode.
    void setNonBlocking(int fd);
    /// @brief Update the last active time for a client connection. This is only safe to call from the main epoll thread.
    void updateLastActive(int clientSocket);
    /// @brief Check for and time out idle connections. This is only safe to call from the main epoll thread.
    void timeOutConnections();
    /// @brief Add data to the buffer for a client connection. This is only safe to call from the main epoll thread.
    /// @param buffer The data to add.
    /// @param clientSocket The socket file descriptor of the client.
    void addToBuffer(std::string buffer, int clientSocket);
    /// @brief Get and clear the buffer for a client connection. This is only safe to call from the main epoll thread.
    /// @param clientSocket The socket file descriptor of the client.
    /// @return The buffered data. Can be empty if no data is buffered.
    std::string getFromBuffer(int clientSocket);
};
    