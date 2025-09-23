#include "Parser.h"
#include "HTTPRequest.h"
#include <filesystem>

// Tries to parse an HTTP request from a buffer
std::optional<HTTPRequest> Parser::parseRequest(std::string_view buf)
{
    size_t start = 0;
    size_t end = buf.find(" ");

    // Get the method
    std::string_view method = buf.substr(start, end);
    auto httpMethod = getMethod(method);
    if (!httpMethod) return std::nullopt;
    start = end + 1;
    
    // Get the path
    end = buf.find(" ", start);
    std::string_view path = buf.substr(start, end - start);
    Route route = Route(std::string(path));
    start = end + 1;

    // Get the version
    end = buf.find("\r\n", start);
    std::string_view version = buf.substr(start, end - start);
    start = end + 2;
    
    // Get headers
    std::unordered_map<std::string, std::string> headers;
    while (true) {
        end = buf.find("\r\n", start);
        if (end == start) {
            // End of headers
            start += 2;
            break;
        }
        std::string_view line = buf.substr(start, end - start);
        auto header = parseHeaderLine(line);
        if (header) {
            headers[header->first] = header->second;
        }
        start = end + 2;
    }

    // Create the HTTPRequest object
    HTTPRequest request = HTTPRequest(*httpMethod, route, std::string(version), std::move(headers));

    // The rest is the body
    std::string_view body = buf.substr(start);
    if (!body.empty())
        request.setBody(std::string(body));
    
    return std::optional<HTTPRequest>(request);
}

// Parses a string method into an HTTPRequest::Method enum, returns nullopt if method is invalid
std::optional<HTTPRequest::Method> Parser::getMethod(std::string_view method)
{
    if (method == "GET") return HTTPRequest::Method::GET;
    if (method == "POST") return HTTPRequest::Method::POST;
    if (method == "PUT") return HTTPRequest::Method::PUT;
    if (method == "DELETE") return HTTPRequest::Method::DELETE;
    if (method == "HEAD") return HTTPRequest::Method::HEAD;
    if (method == "OPTIONS") return HTTPRequest::Method::OPTIONS;
    if (method == "PATCH") return HTTPRequest::Method::PATCH;
    if (method == "CONNECT") return HTTPRequest::Method::CONNECT;
    if (method == "TRACE") return HTTPRequest::Method::TRACE;
    return std::nullopt;
}

// Parses a header line into a key-value pair, returns nullopt if parsing fails
std::optional<std::pair<std::string, std::string>> Parser::parseHeaderLine(std::string_view line)
{
    int end = line.find(": ");
    if (end == std::string_view::npos) return std::nullopt;
    std::string key = std::string(line.substr(0, end));
    std::string value = std::string(line.substr(end + 2));
    return std::make_pair(key, value);
}
