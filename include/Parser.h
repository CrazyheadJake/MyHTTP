#pragma once
#include <optional>
#include "HTTPRequest.h"

namespace Parser
{
    std::optional<HTTPRequest> parseRequest(std::string_view buf);
    std::optional<HTTPRequest::Method> getMethod(std::string_view method);
    std::optional<std::pair<std::string, std::string>> parseHeaderLine(std::string_view line);
}
