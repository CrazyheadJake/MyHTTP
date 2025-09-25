#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Parser.h"
#include "HTTPRequest.h"
#include "Server.h"

const int PORT = 8080;

int main() {
    Server server(PORT, ".");
    server.start();
}