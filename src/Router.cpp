#include "Router.h"

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
