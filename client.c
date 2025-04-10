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
#include "lib/sqlite3.h"

#define VERSION 1
#define MAX_GENRES 10
#define MAX_LENGTH 128

int sockfd;
int status = 1;

void parse_query_results_short(const char * results) {
    cJSON *parsed = cJSON_Parse(results);
    cJSON *query_results = cJSON_GetObjectItemCaseSensitive(parsed, "query_results");
    cJSON *movie = NULL;
    printf(DB_MSG_COLUMNS_SHORT);
    cJSON_ArrayForEach(movie, query_results) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(movie, "id");
        cJSON *title = cJSON_GetObjectItemCaseSensitive(movie, "title");
        printf("%d | %s\n", id->valueint, title->valuestring);
    }
    cJSON_Delete(parsed);
}

void parse_query_results_full(const char * results) {
    cJSON *parsed = cJSON_ParseWithLength(results, strlen(results));
    cJSON *query_results = cJSON_GetObjectItemCaseSensitive(parsed, "query_results");
    cJSON *movie;
    printf(DB_MSG_COLUMNS_FULL);
    cJSON_ArrayForEach(movie, query_results) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(movie, "id");
        cJSON *title = cJSON_GetObjectItemCaseSensitive(movie, "title");
        cJSON *release_year = cJSON_GetObjectItemCaseSensitive(movie, "release_year");
        cJSON *genres = cJSON_GetObjectItemCaseSensitive(movie, "genres");
        cJSON *director = cJSON_GetObjectItemCaseSensitive(movie, "director");
        printf("%d | %s | %d | %s | %s\n", id->valueint, title->valuestring, release_year->valueint, genres->valuestring, director->valuestring);
    }
    cJSON_Delete(parsed);
}

void parse_query_results_single(const char * results) {
    cJSON *parsed = cJSON_ParseWithLength(results, strlen(results));
    cJSON *id = cJSON_GetObjectItemCaseSensitive(parsed, "id");
    cJSON *title = cJSON_GetObjectItemCaseSensitive(parsed, "title");
    cJSON *release_year = cJSON_GetObjectItemCaseSensitive(parsed, "release_year");
    cJSON *genres = cJSON_GetObjectItemCaseSensitive(parsed, "genres");
    cJSON *director = cJSON_GetObjectItemCaseSensitive(parsed, "director");
    printf("%s%d | %s | %d | %s | %s\n", DB_MSG_COLUMNS_FULL, id->valueint, title->valuestring, release_year->valueint, genres->valuestring, director->valuestring);
    cJSON_Delete(parsed);
}

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void add_movie() {
    char response[MAX_CLIENT_RECEIVE_DATA+1];
    char title[MAX_LENGTH];
    int release_year;
    char director[MAX_LENGTH];
    char *genres[MAX_GENRES + 1]; //+1 for NULL terminator

    memset(&response, 0, sizeof response);

    printf(CLIENT_QUERY_BUILDER_TITLE);
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; //New line remover

    printf(CLIENT_QUERY_BUILDER_YEAR);
    scanf("%d", &release_year);
    clear_input();

    printf(CLIENT_QUERY_BUILDER_DIRECTOR);
    fgets(director, sizeof(director), stdin);
    director[strcspn(director, "\n")] = 0;

    printf(CLIENT_QUERY_BUILDER_GENRE_NUMBER, MAX_GENRES);
    int num_genres;
    scanf("%d", &num_genres);
    clear_input();

    for (int i = 0; i < num_genres && i < MAX_GENRES; i++) {
        char temp[MAX_LENGTH];
        printf(CLIENT_QUERY_BUILDER_GENRE, i + 1);
        fgets(temp, sizeof(temp), stdin);
        temp[strcspn(temp, "\n")] = 0;
        genres[i] = strdup(temp);
    }
    if (num_genres <= MAX_GENRES) genres[num_genres] = NULL;
    else genres[MAX_GENRES] = NULL;

    char genre_str[512] = "";
    for (int i = 0; genres[i] != NULL; i++) {
        strcat(genre_str, genres[i]);
        if (genres[i + 1] != NULL) strcat(genre_str, ",");
    }

    char *request = NULL;
    asprintf(&request, "POST:%s:%d:%s:%s", title, release_year, genre_str, director);

    if (send_complete(sockfd, request, strlen(request), MAX_MESSAGE_LEN, 0) == -1) {
        printf(ERROR_MSG);
        free(request);
        exit(-1);
    }
    free(request);
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }

    printf("%s", response);

    for (int i = 0; i < num_genres; i++) {
        free(genres[i]);
    }
}

void add_genre_to_movie() {
    int movie_id;
    char genre[MAX_LENGTH];
    char response[MAX_CLIENT_RECEIVE_DATA + 1];

    memset(&response, 0, sizeof response);

    printf(CLIENT_QUERY_BUILDER_ID);
    scanf("%d", &movie_id);
    clear_input();

    printf(CLIENT_QUERY_BUILDER_SINGLE_GENRE);
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    int result = update_movie_genre(movie_id, genre);

    char *request = NULL;
    asprintf(&request, "PUT:%d:%s", movie_id, genre);

    if (send_complete(sockfd, request, strlen(request), MAX_MESSAGE_LEN, 0) == -1) {
        printf(ERROR_MSG);
        free(request);
        exit(-1);
    }
    free(request);
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }

    printf("%s", response);
}

void list_movies() {
    int size = 0;
    char response[MAX_CLIENT_RECEIVE_DATA + 1];
    char request[] = "GET:ALL";

    memset(&response, 0, sizeof response);

    if (send_complete(sockfd, request, 8, MAX_MESSAGE_LEN, 0) == -1) {
        printf(ERROR_MSG);
        exit(-1);
    }
    
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }

    if (strcmp(response, "{\"query_results\":[]}") != 0) {
        parse_query_results_short(response);
    } else {
        printf(CLIENT_NO_MOVIES_FOUND);
    }
}

void list_movies_with_details() {
    int size = 0;
    char response[MAX_CLIENT_RECEIVE_DATA + 1];
    char request[] = "GET:ALL_DETAILED";

    memset(&response, 0, sizeof response);

    if (send_complete(sockfd, request, 17, MAX_MESSAGE_LEN, 0) == -1) {
        exit(-1);
    }

    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }

    if (strcmp(response, "{\"query_results\":[]}") != 0) {
        parse_query_results_full(response);
    } else {
        printf(CLIENT_NO_MOVIES_FOUND);
    }
}

void list_movies_by_genre() {
    char genre[MAX_LENGTH];
    char response[MAX_CLIENT_RECEIVE_DATA + 1];

    memset(&response, 0, sizeof response);

    printf(CLIENT_QUERY_BUILDER_SEARCH_GENRE);
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    char *request = NULL;
    asprintf(&request, "GET:ALL_GENRE:%s", genre);

    if (send_complete(sockfd, request, strlen(request), MAX_MESSAGE_LEN, 0) == -1) {
        free(request);
        exit(-1);
    }
    free(request);
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }
    if (strcmp(response, "{\"query_results\":[]}") != 0) {
        parse_query_results_short(response);
        //printf("Filmes:\n%s\n", response);
    } else {
        printf(CLIENT_NO_MOVIES_FOUND);
    }
}

void list_movie_details_by_id() {
    int id;
    char *request = NULL;
    char response[MAX_CLIENT_RECEIVE_DATA + 1];

    memset(&response, 0, sizeof response);

    printf(CLIENT_QUERY_BUILDER_ID);
    scanf("%d", &id);
    clear_input();

    asprintf(&request, "GET:SINGLE:%d", id);
    if (send_complete(sockfd, request, strlen(request), MAX_MESSAGE_LEN, 0) == -1) {
        free(request);
        exit(-1);
    }
    free(request);
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
        printf(ERROR_MSG);
	    exit(1);
	} else if (status == 1) {
        printf(SERVER_CLOSED);
        close(sockfd);
    }
    if (strcmp(response, "{}") != 0) {
        parse_query_results_single(response);
    } else {
        printf(CLIENT_NO_MOVIES_FOUND);
    }
}

void remove_movie_by_id() {
    char response[MAX_CLIENT_RECEIVE_DATA + 1];
    int id;

    memset(&response, 0, sizeof response);

    printf(CLIENT_QUERY_BUILDER_ID);
    scanf("%d", &id);
    clear_input();

    char * request;
    asprintf(&request, "DELETE:%d", id);
    if (send_complete(sockfd, request, strlen(request), MAX_MESSAGE_LEN, 0) == -1) {
        free(request);
        exit(-1);
    }
    free(request);
    status = recv_complete(sockfd, response, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    printf("%s", response);
}

int main(int argc, char *argv[]) {
    int numbytes, option;  
	char buf[MAX_CLIENT_RECEIVE_DATA];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"Usage: ./client HOST_NAME\n");
	    exit(1);
	}

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror(ERROR_CLIENT_SOCKET);
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror(ERROR_CLIENT_CONNECT);
			close(sockfd);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, ERROR_CLIENT_CONNECTION);
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

	printf(CLIENT_CONNECTING_TO_SERVER);

    /*
        Main execution loop.
        Client chooses commands. Each command sends a request and
        receives a response from the server.

        Client exits at will or during server malfunctions.
    */
    do {
        printf(CLIENT_MENU, VERSION);
        printf(CLIENT_ADD_MOVIE);
        printf(CLIENT_ADD_GENRE);
        printf(CLIENT_LIST_MOVIES);
        printf(CLIENT_LIST_DETAILS);
        printf(CLIENT_LIST_GENRE);
        printf(CLIENT_LIST_ID);
        printf(CLIENT_REMOVE);
        printf(CLIENT_LEAVE);
        printf(CLIENT_SELECT_OPTION);
        scanf("%d", &option);
        clear_input();

        switch (option) {
            case 1:
                add_movie();
                break;
            case 2:
                add_genre_to_movie();
                break;
            case 3:
                list_movies();
                break;
            case 4:
                list_movies_with_details();
                break;
            case 5:
                list_movies_by_genre();
                break;
            case 6:
                list_movie_details_by_id();
                break;
            case 7:
                remove_movie_by_id();
                break;
            case 0:
                printf(CLIENT_QUIT);
                close(sockfd);
                break;
            default:
                printf(CLIENT_INVALID_OPTION);
        }
    } while (option != 0 && status != 1);

    return 0;
}