#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define PORT "2025"

/*
    Parameters might be args if necessary.
*/
int main(void)
{
    int status; // Address status;
    struct addrinfo hints;
    struct addrinfo *serverinfo;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "Error %s\n", gai_strerror(status));
        exit(1);
    }


    freeaddrinfo(serverinfo);
}
