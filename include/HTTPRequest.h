#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <filesystem>

using Route = std::filesystem::path;

// Holds all information about an HTTP request
class HTTPRequest {
public:
    enum class Method {
        GET,
        POST,
        PUT,
        DELETE,
        HEAD,
        OPTIONS,
        PATCH,
        CONNECT,
        TRACE
    };

    HTTPRequest(Method method, Route route, std::string version, std::unordered_map<std::string, std::string> headers) 
    : m_method(method), m_route(route), m_version(version), m_headers(std::move(headers)) {}
    Method getMethod() const { return m_method; }
    const Route& getRoute() const { return m_route; }
    std::optional<std::string> getHeader(const std::string& key) const {
        auto it = m_headers.find(key);
        if (it != m_headers.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    const std::unordered_map<std::string, std::string>& getAllHeaders() const { return m_headers; }
    const std::string& getBody() const { return m_body; }
    void setBody(const std::string& body) { m_body = body; }
    const std::string& getVersion() const { return m_version; }
    std::string toString() const {
        std::string result = getMethodString(m_method) + " " + m_route.string() + " " + m_version + "\r\n";
        for (const auto& [key, value] : m_headers) {
            result += key + ": " + value + "\r\n";
        }
        result += "\r\n" + m_body;
        return result;
    }

    static std::string getMethodString(Method method) {
        switch (method) {
            case Method::GET: return "GET";
            case Method::POST: return "POST";
            case Method::PUT: return "PUT";
            case Method::DELETE: return "DELETE";
            case Method::HEAD: return "HEAD";
            case Method::OPTIONS: return "OPTIONS";
            case Method::PATCH: return "PATCH";
            case Method::CONNECT: return "CONNECT";
            case Method::TRACE: return "TRACE";
            default: return "UNKNOWN";
        }
    }

private:
    Method m_method;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    Route m_route;
    std::string m_version;
};