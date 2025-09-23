#pragma once

#include <unordered_map>
#include <string>
#include <optional>
#include <filesystem>

// Holds all information about an HTTP response
class HTTPResponse {
public:
    enum class Status {
        OK = 200,
        CREATED = 201,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501
    };

    HTTPResponse(Status status, std::unordered_map<std::string, std::string> headers) 
    : m_status(status), m_headers(std::move(headers)) {}
    Status getStatus() const { return m_status; }
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
    std::string toString() const {
        std::string result = m_version + " " + std::to_string(static_cast<int>(m_status)) + " " + getStatusText(m_status) + "\r\n";
        for (const auto& [key, value] : m_headers) {
            result += key + ": " + value + "\r\n";
        }
        result += "\r\n" + m_body;
        return result;
    }

    static std::string getStatusText(Status status) {
        switch (status) {
            case Status::OK: return "OK";
            case Status::CREATED: return "Created";
            case Status::NO_CONTENT: return "No Content";
            case Status::BAD_REQUEST: return "Bad Request";
            case Status::NOT_FOUND: return "Not Found";
            case Status::INTERNAL_SERVER_ERROR: return "Internal Server Error";
            case Status::NOT_IMPLEMENTED: return "Not Implemented";
            default: return "Unknown Status";
        }
    }
private:
    Status m_status;
    std::unordered_map<std::string, std::string> m_headers;
    std::string m_body;
    std::string m_version = "HTTP/1.1";
};