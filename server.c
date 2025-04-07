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
#include "database.h"
#include "protocol.h"
#include "message.h"
#include "utils.h"
#include "network.h"
#include <time.h>
#include "lib/cJSON.h"

int parse_query_results(const char * results) {
    cJSON *parsed = cJSON_ParseWithLength(results, strlen(results));
}


int extract_message(const char * message, char * command, char args[][1024]) {
    u_int32_t len = strlen(message);
    u_int32_t arg_count = 0;
    u_int32_t current_section = 0;
    if (len == 0) return -1;
    if (message[0] == ':') return -1;
    for (int i = 0; i < len; i++) {
        if (message[i] == ':' && message[i-1] != '\\') {
            if (arg_count == 0) {
                command[current_section] = '\0';
            } else {
                args[arg_count-1][current_section] = '\0';
            }
            arg_count++;
            current_section = 0;
            continue;
        }

        if (arg_count == 0) {
            command[current_section] = message[i];
            current_section++;
        } else {
            if (message[i] == ':') {
                args[arg_count-1][current_section-1] = ':';
            } else {
                args[arg_count-1][current_section] = message[i];
                current_section++;
            }
        }
    }
    return arg_count;
}

void *serve_client(void *fd_ptr)
{
    char message[MAX_MESSAGE_LEN+1];
    char command[MAX_SEGMENT_LEN+1];
    char args[10][1024]; // We will not be using more than 10 args (we use much less, actually)

    int *fd = (int *)fd_ptr;
    int connection_fd = *fd;    //Ensures we copy the FD so we don't close another thread's connection.
    u_int32_t bytes_received = 0;
    u_int32_t arg_count = 0;

    memset(&message, 0, sizeof message);
    memset(&command, 0, sizeof command);
    for(int i = 0; i < 10; i++) memset(&args[i], 0, sizeof args[i]);

    int recv_status = recv_complete(connection_fd, message, MAX_MESSAGE_LEN, MAX_MESSAGE_LEN, 0);
    if (recv_status == -1) {
        pthread_exit("error");
        return NULL;
    } else if (recv_status == 1) {
        pthread_exit(NULL);
        return NULL;
    }

    printf("HERE\n");

    arg_count = extract_message(message, command, args);

    printf("command %s\n", command);
    printf("args %s\n", args[0]);


    if (strcmp(command, GET) == 0) {
        char * result;
        if (strcmp(args[0], ALL) == 0) {
            int operation = select_all_movies(&result);
            if (operation == 1) {
                send_complete(connection_fd, result, strlen(result), strlen(result), 0);
                printf("%s\n", result);
                free(result);
            }
            else send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 28, 28, 0);
        } else if (strcmp(args[0], ALL_DETAILED) == 0) {
            int operation = select_all_movies(&result);
            if (operation == 1) {
                send_complete(connection_fd, result, strlen(result), strlen(result), 0);
                free(result);
            }            else send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 28, 28, 0);
        } else if (strcmp(args[0], ALL_GENRE) == 0) {
            int operation = select_all_movies_by_genre(args[1], &result);
            if (operation == 1) {
                send_complete(connection_fd, result, strlen(result), strlen(result), 0);
                free(result);
            }            else send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 28, 28, 0);
        } else if (strcmp(args[0], SINGLE) == 0) {
            int id = 0;
            int is_int = str_to_int(args[0], &id);
            if (is_int) {
                int operation = select_movie_by_ID(id, &result);
                if (operation == 1) {
                    send_complete(connection_fd, result, strlen(result), strlen(result), 0);
                    free(result);
                }
                else if (operation == 0) send_complete(connection_fd, ERROR_MSG_DB_INVALID_ID, 41, 41, 0);
                else send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 50, 50, 0);
            } else {
                send_complete(connection_fd, ERROR_MSG_INVALID_GET, 20, 20, 0);
            }
        } else {
            send_complete(connection_fd, ERROR_MSG_INVALID_GET, 20, 20, 0);
        }
    } else if (strcmp(command, POST) == 0) {
        if (arg_count == 4) {
            int release_year = 0;
            int year_status = str_to_int(args[1], &release_year);
            if (year_status == 0) {
                send_complete(connection_fd, ERROR_MSG_INVALID_POST, 21, 21, 0);
            }
            int operation = create_movie(args[0], release_year, args[2], args[3]);
            if (operation == 1) {
                send_complete(connection_fd, SUCCESS_MSG_POST, 23, 23, 0);
            } else {
                send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 50, 50, 0);
            }
        } else {
            send_complete(connection_fd, ERROR_MSG_INVALID_POST, 21, 21, 0);
        }
    } else if (strcmp(command, PUT) == 0) {
        if (arg_count == 2) {
            int id = 0;
            int is_int = str_to_int(args[0], &id);
            if (is_int == 1) {
                int operation = update_movie_genre(id, args[1]);
                if (operation == 1) {
                    send_complete(connection_fd, SUCCESS_MSG_PUT, 23, 23, 0);
                } else if (operation == 0) {
                    send_complete(connection_fd, ERROR_MSG_DB_INVALID_ID, 41, 41, 0);
                } else if (operation == 2) {
                    send_complete(connection_fd, ERROR_MSG_DB_INVALID_ID, 38, 38, 0);
                } else {
                    send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 50, 50, 0);
                }
            }
        } else {
            send_complete(connection_fd, ERROR_MSG_INVALID_PUT, 20, 20, 0);
        }
    } else if (strcmp(command, DELETE) == 0) {
        if (arg_count == 1) {
            int id = 0;
            int is_int = str_to_int(args[0], &id);
            if (is_int == 1) {
                int operation = remove_movie(id);
                if (operation == 1) {
                    send_complete(connection_fd, SUCCESS_MSG_DELETE, 23, 23, 0);
                } else if (operation == 0) {
                    send_complete(connection_fd, ERROR_MSG_DB_INVALID_ID, 41, 41, 0);
                } else {
                    send_complete(connection_fd, ERROR_MSG_DB_MISC_DB, 50, 50, 0);
                }
            } else {
                send_complete(connection_fd, ERROR_MSG_INVALID_DELETE, 23, 23, 0);
            }
        } else {
            send_complete(connection_fd, ERROR_MSG_INVALID_DELETE, 23, 23, 0);
        }
    } else {
        // Invalid command
        send_complete(connection_fd, ERROR_MSG_INVALID_COMMAND, 24, 24, 0);
    }

    // if (send(connection_fd, "Hello, world!", 13, 0) == -1)
    // {
    //     perror("send");
    //     close(connection_fd);
    //     pthread_exit("error");
    //     return NULL;
    // }
    close_connection(connection_fd);
    pthread_exit(0);
    return NULL;
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

    srand((unsigned int)time(NULL));    // Initialize randomness. Used in DB for concurrency control.

    // DB config

    if (!database_already_created()) {
        int status = execute_sql_file("setup.sql");
        if (!status) exit(-1);
        mark_database_as_created();
        printf("Banco de dados criado e configurado.\n");
    } else {
        printf("Banco de dados já configurado.\n");
    }

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
        pthread_t thread;
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
        printf("Server: got connection from %s\n", s);
        pthread_create(&thread, NULL, &serve_client, &new_fd);
    }


}
