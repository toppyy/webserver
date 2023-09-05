#include <unistd.h>
#include <stdio.h>
#include <client.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

void client() {
    printf("Starting client\n");
    int                 sfd, s;
    struct addrinfo     hints;
    struct addrinfo     *result, *rp;


    // Use hints to limit the set of addresses returned by getaddrinfo()
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_flags = AI_PASSIVE;        // For wildcard IP address
    hints.ai_protocol = 0;              // Any protocol
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    // Try to find the server
    s = getaddrinfo("127.0.0.1", "3001", &hints, &result);

    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    }

    // getaddrinfo stored a linked list to where *result points to
    // go through the list and try to connect to each
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(sfd);
    }

    // Free the linked list pointed to by *result. 
    // Special function goes through the list and frees each item
    freeaddrinfo(result);

    if (rp == NULL) {
        // Could not establish a connection
        fprintf(stderr, "Could not connect\n");
    }

    char msg[5] = "EXIT";

    send(sfd, (void*) msg, 5, 0);

    close(sfd);

}

