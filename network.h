#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// To be ported to JSON config.
#define PORT "2025"
#define BACKLOG 5
#define MAX_MESSAGE_LEN 1024
#define MAX_SEGMENT_LEN 256
#define MAX_ARG_LEN 768

#define MAX_CLIENT_RECEIVE_DATA 4096

void close_connection(int __fd);
int send_complete(int __fd, const char const  *__buf, int __n, int __max_message_len, int __flags);
int recv_complete(int __fd, const char const *__buf, int __n, int __max_message_len, int __flags);
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);