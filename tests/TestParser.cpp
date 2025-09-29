#include <gtest/gtest.h>
#include "Parser.h"
#include "HTTPRequest.h"

TEST(ParserTest, ParsesGETRequest) {
    std::string rawRequest = "GET /hello HTTP/1.1\r\nHost: localhost\r\n\r\n";
    auto req = Parser::parseRequest(rawRequest);
    ASSERT_TRUE(req.has_value());
    EXPECT_EQ(req->getMethod(), HTTPRequest::Method::GET);
    EXPECT_EQ(req->getRoute(), "/hello");
}

TEST(ParserTest, FailsInvalidRequest) {
    std::string rawRequest = "BADREQUEST";
    auto req = Parser::parseRequest(rawRequest);
    EXPECT_FALSE(req.has_value());
}