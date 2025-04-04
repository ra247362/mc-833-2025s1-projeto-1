#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

// To be ported to JSON config.
#define PORT "2025"
#define BACKLOG 5

pthread_t threads[10];
unsigned int next_thread = 0;

void *serve_client(void *fd_ptr)
{
    int *fd = (int *)fd_ptr;
    int connection_fd = *fd;    //Ensures we copy the FD so we don't close another thread's connection.
    if (send(connection_fd, "Hello, world!", 13, 0) == -1)
    {
        perror("send");
        close(connection_fd);
        pthread_exit("error");
        return NULL;
    }
    close(connection_fd);
    pthread_exit(0);
    return NULL;
}

void *get_in_addr(struct sockaddr *socket_addr) {
    if (socket_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)socket_addr)->sin_addr);
    }

    return &(((struct sockaddr_in6*)socket_addr)->sin6_addr);

}

/*
    Parameters might be args if necessary.
    Possible refactors:
    - Move main loop to another function
    - Implement error handling
*/
int main(void)
{
    int addr_status; // Address status;
    int socket_fd;
    int yes = 1;
    
    struct addrinfo hints;              // To be filled with relevant info
    struct addrinfo *serverinfo;        // To be filled by 'getaddrinfo'
    struct addrinfo *p;                 // To be an item of the linked list

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;        // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // TPC
    hints.ai_flags = AI_PASSIVE;        // Local IP

    if ((addr_status = getaddrinfo(NULL, PORT, &hints, &serverinfo)) != 0) {
        fprintf(stderr, "Error %s\n", gai_strerror(addr_status));
        exit(1);
    }

    // Loop thropugh results and bind to first we can
    // If we get an error, we skip.
    // We could have multiple addresses for a domain, which is why we do this.
    // Source: Beej's book and man pages.
    for (p = serverinfo; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socket_fd, serverinfo->ai_addr, serverinfo->ai_addrlen) == -1) {
            close(socket_fd);
            perror("server: bind");
            continue;
        }

        break; // We were able to connect.
    }

    freeaddrinfo(serverinfo);

    // Error: none of the results of the linked list worked.
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // Begin listening
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /*
    Overall structure that needs to be implemented:
    1 - Call Socket and create a new socket. 
    2 - Bind socket to port.
    3 - Listen to connections
    4 - Create thread for every client on connection established.
    5 - Kill thread and connection once comms are over.
    */

    //listen(socket_fd, BACKLOG);

    while(true) {
        /*
        What the main loop does:
        - Accept connection
        - Create thread with service function and input the new fd
        */
        struct sockaddr_storage client_addr;
        socklen_t addr_size;
        addr_size = sizeof client_addr;
        char s[INET6_ADDRSTRLEN];

        int new_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (new_fd == -1) 
        {
            perror("accept");
            continue;
        }

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
        printf("Server: got connection from %s\n", s);
        pthread_create(&threads[next_thread], NULL, &serve_client, &new_fd);
        next_thread = (next_thread + 1) % 10;
    }


}
