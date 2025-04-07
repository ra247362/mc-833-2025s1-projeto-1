#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int send_complete(int __fd, const char const  *__buf, int __n, int __max_message_len, int __flags) {
    u_int32_t bytes_sent;
    u_int32_t err_count = 0;
    while(bytes_sent < __max_message_len) {
        u_int32_t sent = send(__fd, &__buf+bytes_sent, __max_message_len, 0);
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

int recv_complete(int __fd, const char const *__buf, int __n, int __max_message_len, int __flags) {
    u_int32_t bytes_received;
    u_int32_t err_count = 0;
    while(bytes_received < __max_message_len) {
        u_int32_t received = recv(__fd, &__buf+bytes_received, __max_message_len, 0);
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

