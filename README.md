# MyHTTP
A lightweight, high-performance HTTP server built in modern C++ using one epoll thread and a worker thread pool.

## Features
- Concurrent connection handling with epoll (I/O multiplexing)
- Thread pool for efficient request processing
- Static file serving (HTML, CSS, JS, etc.)
- Support for HTTP/1.1 requests (GET, headers, keep-alive)
- Extensible routing system for dynamic endpoints
- Written in C++23 for performance and clarity

## Installation
```bash
git clone https://github.com/CrazyheadJake/MyHTTP.git
cd MyHTTP
mkdir build && cd build
cmake ..
make
```

## Usage
### Example Server
Included is a sample server. To run it, just run
```bash
./SimpleServer
```
Then visit http://localhost:8080 in your browser

Example Requests
```bash
# Fetch a static file
curl http://localhost:8080/index.html  

# Call a dynamic route
curl http://localhost:8080/random
```

### Using as a Static Library
You can integrate MyHTTP directly into your own C++ project by linking against the static library:
```cmake
# CMakeLists.txt
add_subdirectory(MyHTTP)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp PRIVATE MyHTTP)
```

In your code, create a server object and run it like so:
```cpp
#include "Server.h"

int main() {
    Server server(8080, "path/to/public_dir/"); // Will automatically look for index.html in this directory
    server.start();
}
```

Use `server.addRoute(...)` before calling `server.start()` to add a dynamic route to your http server.

## Testing
To run the built in tests, run the following command.
```bash
ctest --output-on-failure
```

This will run all unit tests and integration tests automatically and output any failures.

## License
This project is licensed under the [Apache 2.0 License](LICENSE).