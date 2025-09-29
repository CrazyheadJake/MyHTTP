#pragma once
#include "HTTPResponse.h"
#include <filesystem>

// Methods to generate different types of HTTP responses can be added here
namespace ResponseGenerator {

    /// @brief Generate a simple HTML response with the given content.
    HTTPResponse generateHTMLResponse(const std::string& htmlContent);
    HTTPResponse generateNotFoundResponse();
    HTTPResponse generateBadRequestResponse();
    HTTPResponse generateForbiddenResponse();
    HTTPResponse generateInternalServerErrorResponse();
    HTTPResponse generateNotImplementedResponse();
    /// @brief Generate a response that serves the contents of the specified file.
    HTTPResponse generateFileResponse(const std::filesystem::path& filePath);

    const std::unordered_map<std::string, std::string> MIME_TYPES = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".ico", "image/x-icon"},
        {".svg", "image/svg+xml"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".txt", "text/plain"},
        {".pdf", "application/pdf"},
        {".doc", "application/msword"}};
        
    std::string getContentType(const std::filesystem::path& filePath);
};