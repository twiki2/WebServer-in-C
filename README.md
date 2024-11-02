# Simple Multi-threaded HTTP Server in C
This is a basic multi-threaded HTTP server written in C. It handles basic GET and POST requests, parses HTTP headers, and sends responses accordingly. This project demonstrates core network programming concepts using sockets, threading, and HTTP protocol parsing.

## Workflow
1. Server Initialization: The server creates a TCP socket, binds it to port 8080, and listens for incoming connections.
2. Connection Handling: Each client connection is accepted and handled in a separate thread, allowing concurrent handling of multiple clients.
3. Request Parsing:
   * The request's method, path, and protocol are extracted from the request line.
   * HTTP headers are parsed and stored.
     
4.Response: Based on the HTTP method:
   * GET requests return a "Hello, World!" message.
   * POST requests return a confirmation message.
   * Other methods receive a "Bad Request" response.
