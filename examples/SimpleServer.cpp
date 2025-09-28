#include "Server.h"
#include <filesystem>
#include <iostream>
#include "HTTPRequest.h"
#include "HTTPResponse.h"

const int PORT = 8080;

int main() {
    srand(time(0));
    Server server(PORT, "../public/");
    server.addRoute("/random", HTTPRequest::Method::GET, [](const HTTPRequest& req) {
        HTTPResponse res(HTTPResponse::Status::OK, {{"Content-Type", "text/html"}});
        int randomNumber = rand() % 100; // Generate a random number between 0 and 99
        res.setBody(std::to_string(randomNumber) + "\n<a href=\"/\">Link back to the homepage</a>");
        return res;
    });
    server.start();
}