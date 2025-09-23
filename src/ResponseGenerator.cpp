#include "ResponseGenerator.h"

HTTPResponse ResponseGenerator::generateNotFoundResponse()
{
     std::string message = "404 Not Found";
   std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(message.size())}
    };
    HTTPResponse response(HTTPResponse::Status::NOT_FOUND, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateBadRequestResponse()
{
    std::string message = "400 Bad Request";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(message.size())}
    };
    HTTPResponse response(HTTPResponse::Status::BAD_REQUEST, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateInternalServerErrorResponse()
{
    std::string message = "500 Internal Server Error";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(message.size())}
    };
    HTTPResponse response(HTTPResponse::Status::INTERNAL_SERVER_ERROR, std::move(headers));
    response.setBody(message);
    return response;
}

HTTPResponse ResponseGenerator::generateNotImplementedResponse()
{
    std::string message = "501 Not Implemented";
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/plain"},
        {"Content-Length", std::to_string(message.size())}
    };
    HTTPResponse response(HTTPResponse::Status::NOT_IMPLEMENTED, std::move(headers));
    response.setBody(message);
    return response;}

HTTPResponse ResponseGenerator::generateHTMLResponse(const std::string &htmlContent)
{
    std::unordered_map<std::string, std::string> headers = {
        {"Content-Type", "text/html"},
        {"Content-Length", std::to_string(htmlContent.size())}
    };
    HTTPResponse response(HTTPResponse::Status::OK, std::move(headers));
    response.setBody(htmlContent);
    return response;
}
