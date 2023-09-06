# A web server

Socket-programming practices. A web server that can return static files (responding to simple GET-requests). 

To serve files from "./public/", use:

    cd src && make && ./server ./public/

You can specify a port (defaults to 3000):

    ./server ./public/ 8000