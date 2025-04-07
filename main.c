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

#define MAX_GENRES 10
#define MAX_LENGTH 128

int sockfd;
int status = 1;


void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void add_movie() {
    char title[MAX_LENGTH];
    int release_year;
    char director[MAX_LENGTH];
    char *genres[MAX_GENRES + 1]; //+1 for NULL terminator

    printf("Título: ");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; //New line remover

    printf("Ano de lançamento: ");
    scanf("%d", &release_year);
    clear_input();

    printf("Diretor: ");
    fgets(director, sizeof(director), stdin);
    director[strcspn(director, "\n")] = 0;

    printf("Quantos gêneros deseja adicionar? (até %d): ", MAX_GENRES);
    int num_genres;
    scanf("%d", &num_genres);
    clear_input();

    for (int i = 0; i < num_genres && i < MAX_GENRES; i++) {
        char temp[MAX_LENGTH];
        printf("Gênero %d: ", i + 1);
        fgets(temp, sizeof(temp), stdin);
        temp[strcspn(temp, "\n")] = 0;
        genres[i] = strdup(temp);
    }
    genres[num_genres] = NULL;

    char genre_str[512] = "";
    for (int i = 0; genres[i] != NULL; i++) {
        strcat(genre_str, genres[i]);
        if (genres[i + 1] != NULL) strcat(genre_str, ",");
    }

    if (create_movie(title, release_year, genre_str, director)) {
        printf("Filme criado com sucesso!\n");
    } else {
        printf("Erro ao criar o filme.\n");
    }

    for (int i = 0; i < num_genres; i++) {
        free(genres[i]);
    }
}

void add_genre_to_movie() {
    int movie_id;
    char genre[MAX_LENGTH];

    printf("ID do filme: ");
    scanf("%d", &movie_id);
    clear_input();

    printf("Escreva o gênero que deseja adicionar: ");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    int result = update_movie_genre(movie_id, genre);

    if (result == 1) {
        printf("Gênero adicionado com sucesso!\n");
    } else if (result == 2) {
        printf("O filme já possui esse gênero.\n");
    } else {
        printf("Erro ao adicionar o gênero.\n");
    }
}

void list_movies() {
    int size = 0;
    char json[MAX_CLIENT_RECEIVE_DATA];
    char request[] = "GET:ALL";

    //select_all_movies(&json);
    if (send_complete(sockfd, request, 8, MAX_MESSAGE_LEN, 0) == -1) {
        exit(-1);
    }
    
    status = recv_complete(sockfd, json, MAX_CLIENT_RECEIVE_DATA, MAX_CLIENT_RECEIVE_DATA, 0);
    if (status == -1){
	    exit(1);
	} else if (status == 1) close(sockfd);

    if (json) {
        printf("Filmes:\n%s\n", json);
    } else {
        printf("Nenhum filme encontrado.\n");
    }
}

void list_movies_with_details() {
    char *json;
    select_all_movies_details(&json);
    if (json) {
        printf("Filmes (detalhado):\n%s\n", json);
        free(json);
    } else {
        printf("Nenhum filme encontrado.\n");
    }
}

void list_movies_by_genre() {
    char genre[MAX_LENGTH];

    printf("Gênero: ");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    char *json;
    select_all_movies_by_genre(genre, &json);
    if (json) {
        printf("Filmes com gênero '%s':\n%s\n", genre, json);
        free(json);
    } else {
        printf("Nenhum filme encontrado com esse gênero.\n");
    }
}

void list_movie_details_by_id() {
    int id;

    printf("ID do filme: ");
    scanf("%d", &id);
    clear_input();

    char *json;
    select_movie_by_ID(id, &json);
    if (json) {
        printf("Detalhes do filme:\n%s\n", json);
        free(json);
    } else {
        printf("Filme não encontrado.\n");
    }
}

void remove_movie_by_id() {
    int id;

    printf("ID do filme a remover: ");
    scanf("%d", &id);
    clear_input();

    if (remove_movie(id)) {
        printf("Filme removido com sucesso!\n");
    } else {
        printf("Erro ao remover o filme.\n");
    }
}

int main(int argc, char *argv[]) {
    int numbytes, option;  
	char buf[MAX_CLIENT_RECEIVE_DATA];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	// if (argc != 2) {
	//     fprintf(stderr,"usage: ./client HOST_NAME\n");
	//     exit(1);
	// }


    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.0", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

    do {
        printf("\n===== MENU =====\n");
        printf("1. Adicionar filme\n");
        printf("2. Adicionar gênero a filme\n");
        printf("3. Listar todos os filmes\n");
        printf("4. Listar todos os filmes (detalhado)\n");
        printf("5. Listar filmes por gênero\n");
        printf("6. Listar detalhes de filme por ID\n");
        printf("7. Remover filme por ID\n");
        printf("0. Sair\n");
        printf("Escolha o número da opção desejada: ");
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
                printf("Saindo...\n");
                close(sockfd);
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (option != 0 && status != 1);

    return 0;
}