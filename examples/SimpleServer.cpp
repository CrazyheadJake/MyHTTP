#include "Server.h"
#include <filesystem>
#include <iostream>
#include "HTTPRequest.h"
#include "HTTPResponse.h"

const int PORT = 8080;

int main() {
    srand(time(0));
    Server server(PORT, "../../public_html/", 32);
    server.addRoute("/random", HTTPRequest::Method::GET, [](const HTTPRequest& req) {
        int randomNumber = rand() % 100; // Generate a random number between 0 and 99
        HTTPResponse res = ResponseGenerator::generateHTMLResponse(
            std::to_string(randomNumber) + "\n<a href=\"/\">Link back to the homepage</a>"
        );
        return res;
    });
    server.addRoute("/cs290/HW3-moleskij/contact.html", HTTPRequest::Method::POST, [](const HTTPRequest& req) {
        HTTPResponse res = ResponseGenerator::generateHTMLResponse(
            req.getBody() + "\n<a href=\"/cs290/HW3-moleskij/contact.html\">Link back to the contacts</a>"
        );
        return res;
    });
    server.start();
}