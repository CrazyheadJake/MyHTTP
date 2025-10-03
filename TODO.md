# TODO

- Fix closeConnection not being thread safe
    * To fix this, we're going to use EPOLLONESHOT so that any worker thread is guaranteed to be the only worker interacting with the socket (ie we can close and remove it from epoll safely)
    * Still probably need an extra mutex for removing old data unless we let the main epoll thread do that
    
- Benchmark large concurrent connections
- Implement proper server shutdown using signals or equivalent
- Generally more test cases and unit tests
- Add caching for static files
- Add reverse proxy with config file
- Make a more extensive example server
- Optimize static file serving to 0 copies, not even to userspace memory
- Add better logging / actual logging library