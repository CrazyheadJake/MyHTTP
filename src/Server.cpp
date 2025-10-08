#include "Server.h"
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "Parser.h"
#include "ResponseGenerator.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include "WorkerPool.h"
#include <fcntl.h>
#include <queue>

int Server::m_shutdownEventFd = -1;

Server::Server(int port, const std::filesystem::path &rootDir, int numThreads, int timeoutSeconds) 
    : m_timeoutSeconds(timeoutSeconds), m_port(port), m_router(rootDir), m_pool(WorkerPool(numThreads))
{
    m_epollFd = epoll_create1(0);
    if (m_epollFd == -1) {
        throw std::runtime_error("Failed to create epoll file descriptor: " + std::string(strerror(errno)));
    }
    setupSocket();
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // interested in read events, edge-triggered
    ev.data.fd = m_serverSocket;
    setNonBlocking(m_serverSocket);
    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_serverSocket, &ev);

    ev.events = EPOLLIN | EPOLLET;
    if (m_shutdownEventFd == -1)
        m_shutdownEventFd = eventfd(0, EFD_NONBLOCK);
    ev.data.fd = m_shutdownEventFd;
    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_shutdownEventFd, &ev);

    std::signal(SIGINT, signalHandler);
}

void Server::start()
{
    listen(m_serverSocket, 10);
    // m_running = 1;
    epoll_event events[64];
    bool running = true;
    while (running) {
        // Calculate wait time for epoll based on next connection timeout
        int waitTime = -1;
        if (!m_activeConnections.empty()) {
            auto now = std::chrono::steady_clock::now();
            auto& conn = m_activeConnections.top();
            double duration = 5 - std::chrono::duration<double>(now - conn.lastActive).count();
            waitTime = std::max(0.0, duration) * 1000; // convert to milliseconds
        }
        // Wait for events, this will block until an event occurs or timeout
        int n = epoll_wait(m_epollFd, events, 64, waitTime);
        timeOutConnections();
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == m_serverSocket) {
                acceptConnection();
            }
            // If we have received shutdown signal
            else if (events[i].data.fd == m_shutdownEventFd)
                running = false;
            else {
                int clientFd = events[i].data.fd;
                uint32_t ev = events[i].events;
                updateLastActive(clientFd);

                // push client work onto pool
                m_pool.enqueue([this, clientFd, ev]() {
                    handleClient(clientFd, ev);
                });
            }
        }
    }
}

Server::~Server()
{
    // Close server socket
    close(m_serverSocket);
    // Join all worker threads
    m_pool.stop();
    for (auto& [socket, time]: m_lastActiveTimes) {
        close(socket);
    }
    m_connectionBuffers.clear();
    m_activeConnections = std::priority_queue<ClientConnection, std::vector<ClientConnection>, std::greater<ClientConnection>>();

    // Close epoll instance
    close(m_epollFd);

    int m_serverSocket = -1;
    int m_epollFd = -1;
}

void Server::signalHandler(int signal) {
    uint64_t one = 1;
    std::string message = "\nShutting down Server...\n";
    write(STDOUT_FILENO, message.data(), message.size());
    write(m_shutdownEventFd, &one, sizeof(one));
}

void Server::addRoute(Route route, int method, Handler handler) {
    m_router.addRoute(route, method, handler);
}

HTTPResponse Server::handleRequest(const std::optional<HTTPRequest> &request) const
{
    // Simple routing logic
    if (!request) {
        return ResponseGenerator::generateBadRequestResponse();
    }
    std::optional<Handler> handler = m_router.getHandler(request->getRoute(), request->getMethod());
    if (handler) {
        return (*handler)(*request);
    }
    std::optional<HTTPResponse> staticFile = m_router.getStaticFile(*request);
    if (staticFile) {
        return *staticFile;
    }
    return ResponseGenerator::generateNotFoundResponse();
}

void Server::setupSocket()
{
    // Creating socket
    m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_serverSocket < 0) {
        throw std::runtime_error("Failed to create socket" + std::string(strerror(errno)));
    }

    // Specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(m_port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Binding socket.
    int opt = 1;
    setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int result = bind(m_serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (result < 0) {
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }
}

void Server::acceptConnection()
{
    // Accepting connection request
    while (true) {
        int clientSocket = accept(m_serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; // no more connections to accept
            std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
            continue;
        }

        setNonBlocking(clientSocket);
        epoll_event ev;
        // interested in read events, edge-triggered, and only let one thread access a socket at a time (epolloneshot)
        ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        ev.data.fd = clientSocket;

        if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientSocket, &ev) == -1) {
            std::cerr << "Failed to add client to epoll" << strerror(errno) << std::endl;
            close(clientSocket);
            continue;
        }
        updateLastActive(clientSocket);
        // std::cout << "Accepted new client, fd=" << clientSocket << "\n";
    }
}

// This is run in a worker thread
void Server::handleClient(int clientSocket, uint32_t events)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.fd = clientSocket;

    // Receiving message from client
    auto optBuffer = receiveData(clientSocket);
    if (!optBuffer) {
        // Rearm the fd in epoll, this will fail if receiveData closed the connection due to malformed data
        epoll_ctl(m_epollFd, EPOLL_CTL_MOD, clientSocket, &ev);
        return;
    }
    std::string buffer = std::move(*optBuffer);

    // Parse the request string into an HTTPRequest object
    auto request = Parser::parseRequest(buffer);
    // Handle the request and generate a response
    auto response = handleRequest(request);
    std::string responseStr = response.toString();

    // Debug output
    // std::cout << "Parsed HTTP request from fd=" << clientSocket << std::endl;
    // if (request)
    //     std::cout << HTTPRequest::getMethodString(request->getMethod()) << " " << request->getRoute() << std::endl;
    // std::cout << "Response: \n" << responseStr << std::endl;
    
    // Send data, this will block if the kernel send buffer is filled up
    if (sendData(clientSocket, std::move(responseStr)) == -1) {
        std::cerr << "Failed to send response to client: " << strerror(errno) << std::endl;
        closeConnection(clientSocket);
        return;
    }

    // If we failed to parse the request, close the connection after sending BadRequest response
    if (!request) {
        std::cerr << "Failed to parse HTTP request" << std::endl;
        closeConnection(clientSocket);
        return;
    }

    // Close connection if "Connection: close" header is present
    auto connectionHeader = request->getHeader("Connection");
    if (connectionHeader && *connectionHeader == "close") {
        closeConnection(clientSocket);
    }
    else {      // Rearm the fd in epoll
        epoll_ctl(m_epollFd, EPOLL_CTL_MOD, clientSocket, &ev);
    }
}

// This is always run in a worker thread
std::optional<std::string> Server::receiveData(int clientSocket)
{
    std::string contentLength = "Content-Length: ";
    std::string endHeaders = "\r\n\r\n";

    // Receiving message from client
    std::string buffer = getFromBuffer(clientSocket);
    size_t oldSize;
    int bytesToReceive = 1024;
    while (true) {
        size_t end = buffer.find(endHeaders);
        // Make sure we have received the full body before we consider the transmission finished
        size_t contentLengthIndex = buffer.find(contentLength);
        if (contentLengthIndex != std::string::npos) {
            int bodySize = std::stoi(buffer.substr(contentLengthIndex + contentLength.size()));
            if (end + endHeaders.size() + bodySize == buffer.size())
                break;
        }
        else if (end != std::string::npos)
            break;

        oldSize = buffer.size();
        buffer.resize(buffer.size() + bytesToReceive);
        // This is a non-blocking recv due to non-blocking socket, exits thread if not done receiving data
        int bytes = recv(clientSocket, buffer.data() + oldSize, bytesToReceive, 0);
        if (bytes == -1) {
            buffer.resize(oldSize);
            // No more data, store data in buffer for next time
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                addToBuffer(std::move(buffer), clientSocket);
                return std::nullopt;
            }
            if (errno == ECONNRESET) {
                closeConnection(clientSocket);
                return std::nullopt;
            }

            // Other errors
            std::cerr << "Failed to read from client: " << strerror(errno) << std::endl;
            closeConnection(clientSocket);
            return std::nullopt;
        }
        // Client closed connection before completing transmission
        if (bytes == 0) {
            // std::cout << "Client disconnected, fd=" << clientSocket << "\n";
            closeConnection(clientSocket);
            return std::nullopt;
        }
        // Resize buffer to get rid of unused data (very cheap, just updates internal size)
        buffer.resize(oldSize + bytes);
    }

    return buffer;
}

ssize_t Server::sendData(int clientSocket, std::string data)
{
    // Simple send; if we need to block just keep trying until it gets sent
    // Not a good solution, but we so rarely block that its almost a non issue
    size_t sent = 0;
    while (sent < data.size()) {
        if (sent != 0)
            std::cout << "Send blocked, fd=" << clientSocket << std::endl;
        int bytesSent = send(clientSocket, data.c_str() + sent, data.size() - sent, 0);
        if (bytesSent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return -1;
        }
        sent += bytesSent;
    }
    return sent;
}

void Server::closeConnection(int clientSocket)
{
    epoll_ctl(m_epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
    close(clientSocket);
    m_lastActiveMutex.lock();
    m_lastActiveTimes.erase(clientSocket);
    m_lastActiveMutex.unlock();
    m_bufferMutex.lock();
    m_connectionBuffers.erase(clientSocket);
    m_bufferMutex.unlock();
    // std::cout << "Closed connection, fd=" << clientSocket << "\n";
}

// Helper: set socket to be non-blocking
void Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::updateLastActive(int clientSocket)
{
    std::lock_guard<std::mutex> lock(m_lastActiveMutex);
    auto now = std::chrono::steady_clock::now();
    m_lastActiveTimes[clientSocket] = now;
    m_activeConnections.push(ClientConnection{clientSocket, now});
}

void Server::timeOutConnections()
{
    auto now = std::chrono::steady_clock::now();
    while (!m_activeConnections.empty()) {
        auto& conn = m_activeConnections.top();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - conn.lastActive).count();
        if (duration > m_timeoutSeconds) {
            // Look up to see if this is still the last active time for this socket
            // or if the socket is already closed
            m_lastActiveMutex.lock();
            auto it = m_lastActiveTimes.find(conn.socket);
            if (it == m_lastActiveTimes.end() || it->second != conn.lastActive) {
                m_activeConnections.pop(); // stale entry, skip it
                m_lastActiveMutex.unlock();
                continue;
            }
            m_lastActiveMutex.unlock();
            std::cout << "Timing out connection, fd=" << conn.socket << "\n";
            // Remove from epoll and close socket
            closeConnection(conn.socket);
            m_activeConnections.pop();
        } else {
            break; // since the queue is ordered, we can stop checking further
        }
    }
}

void Server::addToBuffer(std::string buffer, int clientSocket)
{
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    m_connectionBuffers[clientSocket] += buffer;
}

std::string Server::getFromBuffer(int clientSocket)
{
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    auto it = m_connectionBuffers.find(clientSocket);
    if (it == m_connectionBuffers.end()) {
        return "";
    }
    std::string buffer = std::move(it->second);
    m_connectionBuffers.erase(it);
    return buffer;
}
