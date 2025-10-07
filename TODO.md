# TODO

- Implement reading content body from clients
- Implement proper server shutdown using signals or equivalent
- Generally more test cases and unit tests
- Add caching for static files
- Add reverse proxy with config file
- Make a more extensive example server
- Optimize static file serving to 0 copies, not even to userspace memory
- Add better logging / actual logging library
- send can block in worker thread (temp fixed)
