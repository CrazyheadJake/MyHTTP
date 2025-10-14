#include <gtest/gtest.h>
#include "ResponseGenerator.h"
#include "HTTPResponse.h"

TEST(ResponseGeneratorTest, StaticFile) {
    std::cout << std::filesystem::current_path();
    HTTPResponse res = ResponseGenerator::generateFileResponse("../../public_html/test.txt");
    EXPECT_TRUE(res.getBody().find("EndOfTest") != std::string::npos);
}