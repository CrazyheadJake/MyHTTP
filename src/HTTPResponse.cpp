#include "HTTPResponse.h"

void HTTPResponse::setBody(const std::string &body)
{
    m_body = body;
    m_headers["Content-Length"] = std::to_string(body.size());
}