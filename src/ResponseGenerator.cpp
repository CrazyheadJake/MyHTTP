#include "ResponseGenerator.h"
#include <fstream>

HTTPResponse ResponseGenerator::generateNotFoundResponse()
{
    std::string message = "404 Not Found";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"}
    };
    HTTPResponse response(HTTPResponse::Status::NOT_FOUND, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateBadRequestResponse()
{
    std::string message = "400 Bad Request";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"}
    };
    HTTPResponse response(HTTPResponse::Status::BAD_REQUEST, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateForbiddenResponse()
{
    std::string message = "403 Forbidden";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"}
    };
    HTTPResponse response(HTTPResponse::Status::FORBIDDEN, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateInternalServerErrorResponse()
{
    std::string message = "500 Internal Server Error";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"}
    };
    HTTPResponse response(HTTPResponse::Status::INTERNAL_SERVER_ERROR, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateNotImplementedResponse()
{
    std::string message = "501 Not Implemented";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"}
    };
    HTTPResponse response(HTTPResponse::Status::NOT_IMPLEMENTED, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateFileResponse(const std::filesystem::path &filePath)
{
    // Right now, this is sort of slow. We copy file contents into memory and then copy from 
    // there into the response body. Then we copy from the response body into the socket buffer.
    // For large files, we should consider using sendfile or
    // memory-mapped files to avoid extra copies.
    std::ifstream file(filePath, std::ios::binary);
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", getContentType(filePath)}
    };
    HTTPResponse response(HTTPResponse::Status::OK, std::move(headers));
    response.setBody(fileContent);
    return response;
}

HTTPResponse ResponseGenerator::generateHTMLResponse(const std::string &htmlContent)
{
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/html"}
    };
    HTTPResponse response(HTTPResponse::Status::OK, std::move(headers));
    response.setBody(htmlContent);
    return response;
}

HTTPResponse ResponseGenerator::generateRedirectResponse(const std::string &location)
{
    std::string message = "301 Moved Permanently";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"},
        {"Location", location}
    };
    HTTPResponse response(HTTPResponse::Status::MOVED_PERMANENTLY, std::move(headers));
    response.setBody(message);
    return response;
}

std::string ResponseGenerator::getContentType(const std::filesystem::path &filePath)
{
    auto it = MIME_TYPES.find(filePath.extension().string());
    if (it != MIME_TYPES.end()) {
        return it->second;
    }
    return "application/octet-stream";
}