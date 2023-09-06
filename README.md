# A web server

Socket-programming practices. A web server that can return static files (responding to simple GET-requests). 

To build, run `make`.

To serve files from "./public/", use:

    ./server ./public/

You can specify a port (defaults to 3000):

    ./server ./public/ 8000