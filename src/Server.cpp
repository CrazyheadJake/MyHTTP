#include "Server.h"
#include <cstring>
#include <unistd.h>
#include <iostream>
#include "Parser.h"
#include "ResponseGenerator.h"
#include <sys/epoll.h>
#include "WorkerPool.h"
#include <fcntl.h>
#include <queue>

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
}

void Server::start()
{
    listen(m_serverSocket, 10);
    m_running = true;
    epoll_event events[64];

    while (true) {
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
            } else {
                int clientFd = events[i].data.fd;
                uint32_t ev = events[i].events;

                // push client work onto pool
                m_pool.enqueue([this, clientFd, ev]() {
                    handleClient(clientFd, ev);
                });
            }
        }
    }
}

void Server::stop()
{
    if (m_running) {
        close(m_serverSocket);
        m_running = false;
    }
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
                break; // no more clients
            std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
            return;
        }

        setNonBlocking(clientSocket);

        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // interested in read events, edge-triggered
        ev.data.fd = clientSocket;

        if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientSocket, &ev) == -1) {
            std::cerr << "Failed to add client to epoll" << strerror(errno) << std::endl;
            close(clientSocket);
            return;
        }
        m_activeConnections.push({clientSocket, std::chrono::steady_clock::now()});
        std::cout << "Accepted new client, fd=" << clientSocket << "\n";
    }
}

// This is run in a worker thread
void Server::handleClient(int clientSocket, uint32_t events)
{
    // Receiving message from client
    std::string buffer;
    while (buffer.find("\r\n\r\n") == std::string::npos) {
        char temp[1024];
        // This is a non-blocking recv due to non-blocking socket, we aren't currently taking
        // advantage of that fully though as we just repeat until we get the full request
        // Ideally, we would return to epoll and wait for more data while saving the partial data received
        int bytes = recv(clientSocket, temp, sizeof(temp), 0);
        if (bytes == -1) {
            std::cerr << "Failed to read from client: " << strerror(errno) << std::endl;
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
            close(clientSocket);
            return;
        }
        buffer.append(temp, bytes);
    }

    // Parse the request string into an HTTPRequest object
    auto request = Parser::parseRequest(buffer);
    if (!request) {
        std::cerr << "Failed to parse HTTP request" << std::endl;
        epoll_ctl(m_epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
        close(clientSocket);
        return;
    }
    else {
        std::cout << "Parsed HTTP request:" << std::endl;
        std::cout << request->toString() << std::endl;
    }

    // Handle the request and generate a response
    auto response = handleRequest(*request);
    std::string responseStr = response.toString();
    send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
}

// Helper: set socket to be non-blocking
void Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::timeOutConnections()
{
    auto now = std::chrono::steady_clock::now();
    while (!m_activeConnections.empty()) {
        auto& conn = m_activeConnections.top();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - conn.lastActive).count();
        if (duration > m_timeoutSeconds) {
            std::cout << "Timing out connection, fd=" << conn.socket << "\n";
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, conn.socket, nullptr);
            close(conn.socket);
            m_activeConnections.pop();
        } else {
            break; // since the queue is ordered, we can stop checking further
        }
    }
}
