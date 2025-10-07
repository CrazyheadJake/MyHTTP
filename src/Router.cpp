#include "Router.h"
#include "ResponseGenerator.h"
#include <fstream>

void Router::addRoute(Route route, int method, Handler handler)
{
    m_routes[route].push_back(std::make_pair(method, handler));
}

std::optional<Handler> Router::getHandler(const Route &route, HTTPRequest::Method method) const
{
    auto it = m_routes.find(route);
    if (it == m_routes.end()) return std::nullopt;
    for (const auto &pair : it->second) {
        if (pair.first == method) return pair.second;
    }
    return std::nullopt;
}

std::optional<HTTPResponse> Router::getStaticFile(const HTTPRequest &request) const
{
    if (request.getMethod() != HTTPRequest::Method::GET)
        return std::nullopt;
    Route route = request.getRoute();

    // Add leading "/" so its consistent
    std::string urlPath = route.string();
    if (!urlPath.empty() && urlPath.front() != '/')
        urlPath.insert(0, "/");   
    
    std::filesystem::path fullPath = m_rootDir.string() + urlPath;

    // Security check: canonical must start with rootDir
    std::filesystem::path canonical = std::filesystem::weakly_canonical(fullPath);
    if (canonical.string().rfind(m_rootDir.string(), 0) != 0) {
        return ResponseGenerator::generateForbiddenResponse();
    }

    // Handle directory requests
    if (std::filesystem::is_directory(canonical)) {
        // Fixes bugs where the client doesn't realize it's a directory
        if (!urlPath.empty() && urlPath.back() != '/')
            return ResponseGenerator::generateRedirectResponse(urlPath + "/");
        canonical.concat("/index.html");
    }

    // Check if file exists
    if (!std::filesystem::exists(canonical)) {
        return std::nullopt;
    }

    return ResponseGenerator::generateFileResponse(canonical);
}
