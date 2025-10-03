# TODO

- Fix closeConnection not being thread safe
    * Uh oh, this is actually a symptom of a much bigger problem (design issue)
    * Turns out I probably need to have all IO happen in the epoll thread, otherwise there will be race conditions that I haven't considered. For example, currently I don't consider that send can return EAGAIN if the OS buffer for the socket is full. Also If two threads are handling the samee client (multiple requests) then there is synchronization issues with them both sending data at the same time.
    
- Benchmark large concurrent connections
- Implement proper server shutdown using signals or equivalent
- Generally more test cases and unit tests
- Add caching for static files
- Add reverse proxy with config file
- Make a more extensive example server
- Optimize static file serving to 0 copies, not even to userspace memory
- Add better logging / actual logging library