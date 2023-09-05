#include <stdio.h>
#include <server.h>
#include <client.h>


#define PORT   3001

int main(int argc, char *argv[])  {
    if (argc <= 1) {
        printf("Specify either 's' for server or 'c' for client\n");
        return 0;
    }
    
    if (argv[1][0] == 'c')
        client();

    if (argv[1][0] == 's')
        server(PORT);
}