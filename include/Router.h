#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <filesystem>
#include <functional>
#include "HTTPRequest.h"
#include "HTTPResponse.h"

using Route = std::filesystem::path;
using Handler = std::function<HTTPResponse(const HTTPRequest&)>;

class Router {
public:
    // Using a bitmask of HTTPRequest::Method for the method
    void addRoute(Route route, int method, Handler handler);
    std::optional<Handler> getHandler(const Route& route, HTTPRequest::Method method) const;

private:
    std::unordered_map<Route, std::vector<std::pair<int, Handler>>> m_routes;

};