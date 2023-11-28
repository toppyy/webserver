#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <server.h>
#include <respond.h>
#include <parse.h>


#define LISTEN_BACKLOG 100
#define BUFFSIZE 500

void server(int port, char* path) {
    // define file descriptors and other datastrucrures
    int                 sfd, cfd;
    socklen_t           peer_addr_size;
    struct sockaddr_in  my_addr, peer_addr;
    char buff[BUFFSIZE];


    // Open up a socket
    sfd = socket(AF_INET, SOCK_STREAM, 0); // TCP-socket

    if (sfd == -1) {
        printf("Could not open a socket");
        return;
    }
        
    // Init address data structure
    my_addr.sin_family  = AF_INET;              // IPv4 Internet protocols
    my_addr.sin_port    = htons(port);          // Set port in network byte order
    inet_aton("127.0.0.1", &my_addr.sin_addr);  // Set localhost as address
        
    // Try binding the file descriptor to addr
    if (bind(sfd, (struct sockaddr *) &my_addr,
                    sizeof(my_addr)) == -1) {
               printf("Could not bind to address\n");
               return;
    }

    // Listen to it and react accordingly
    if (listen(sfd, LISTEN_BACKLOG) == -1) {
            printf("Cannot listen\n");
            return;
    }
    printf("Listening to port %d\n", port);

    while (1) {
        // Accept connection        
        printf("Accepting connections..\n");
        peer_addr_size = sizeof(peer_addr);
        cfd = accept(sfd, (struct sockaddr *) &peer_addr,
                    &peer_addr_size);

        if (cfd == -1) {
            printf("Error accepting\n");
            return;
        }
        
        // Receive message  
        int recvd = recv(cfd, &buff, BUFFSIZE, 0);

        respond(recvd, parse_request(buff, recvd), cfd, path);

        if (close(cfd) == -1) {
            printf("Failed to close filehandle cfd");
            return;
        }
    
    }    

    // Wrap up by closing
    if (close(sfd) == -1) {
        printf("Failed to close filehandle sfd");
        return;
    }
    printf("Exited gracefully (closing socket)\n");
}



int main(int argc, char *argv[])  {
    if (argc <= 1) {
        printf("Specify a path to serve files from\n");
        return 0;
    }

    int port = 3000;

    if (argc > 2) {
        // Assume this is PORT
        // TODO: catch error
        port = atoi(argv[2]);
    }
    server(port, argv[1]);
}