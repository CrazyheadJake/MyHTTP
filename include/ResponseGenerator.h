#pragma once
#include "HTTPResponse.h"

namespace ResponseGenerator {
    // Methods to generate different types of HTTP responses can be added here
    HTTPResponse generateHTMLResponse(const std::string& htmlContent);
    HTTPResponse generateNotFoundResponse();
    HTTPResponse generateBadRequestResponse();
    HTTPResponse generateInternalServerErrorResponse();
    HTTPResponse generateNotImplementedResponse();
};