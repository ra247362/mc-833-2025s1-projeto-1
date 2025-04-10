#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

/*
    Library that contains versions of send and recv that ensure that
    all bytes are send / read.

    For send, this is straightforward, as send returns the number of bytes read.
    For recv, we need the maximum size of the buffer and look for a '\0' at the message.
    If no '\0' is found, we stop at the buffer size to prevent overflow.
*/

int send_complete(int __fd, const char  *__buf, int __n, int __max_message_len, int __flags) {
    u_int32_t bytes_sent = 0;
    u_int32_t err_count = 0;    // We stop after 32 errors (arbitrary number, no specific reason)
    // Send until everything is sent or maximum message size is reached.
    while(bytes_sent < __max_message_len) {
        u_int32_t sent = send(__fd, __buf+bytes_sent, __max_message_len, 0);
        if (sent < 0) {
            perror("send");
            err_count++;
            if (err_count > 32) {
                close(__fd);
                return -1;
            }
            continue;
        }
        bytes_sent += sent;
        if (__buf[bytes_sent] == '\0') break;// Early end transmission
    }
    return 0;
}

int recv_complete(int __fd, const char *__buf, int __n, int __max_message_len, int __flags) {
    u_int32_t bytes_received = 0;
    u_int32_t err_count = 0;    // We stop after 32 errors (arbitrary number, no specific reason)
    // Receive until message is complete or buffer size is reached.
    while(bytes_received < __max_message_len) {
        u_int32_t received = recv(__fd, __buf+bytes_received, __max_message_len, 0);
        if (!received) {
            close(__fd);
            return 1;
        } else if (received < 0) {
            perror("receive");
            if (err_count > 32) {
                close(__fd);
                return -1;
            }
            continue;
        }
        bytes_received += received;
        if (__buf[bytes_received] == '\0') break;// Early end transmission
    }
    return 0;
}

/*
    Returns address of appropriate family (IPv4 or IPv6)
*/

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}