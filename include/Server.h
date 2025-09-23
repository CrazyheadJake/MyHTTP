#pragma once

#include <string>
#include <filesystem>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "ResponseGenerator.h"
#include <optional>

class Server {
public:
    Server(int port, const std::filesystem::path& rootDir);
    void start();
    void stop();
    HTTPResponse handleRequest(const std::optional<HTTPRequest>& request);
private:
    int m_port;
    std::filesystem::path m_rootDir;
    bool m_running = false;
};
    