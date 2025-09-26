#include "Server.h"
#include <filesystem>
#include <iostream>

const int PORT = 8080;

int main() {
    Server server(PORT, "../public/");
    server.start();
}