#include "Server.h"

Server::Server(int port, const std::filesystem::path &rootDir) : m_port(port), m_rootDir(rootDir)
{
}

void Server::start()
{
}

void Server::stop()
{
}

HTTPResponse Server::handleRequest(const std::optional<HTTPRequest> &request)
{
    if (!request) {
        return ResponseGenerator::generateBadRequestResponse();
    }
    if (request->getRoute() == "/") {
        std::string htmlContent = "<html><body><h1>Welcome to MyHTTP Server</h1></body></html>";
        return ResponseGenerator::generateHTMLResponse(htmlContent);
    }
    else {
        return ResponseGenerator::generateNotFoundResponse();
    }
}
