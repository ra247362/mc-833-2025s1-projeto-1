#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

#define MAX_GENRES 10
#define MAX_LENGTH 128


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

    if (create_movie(title, release_year, (const char * const *)genres, director)) {
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
    char *json;
    select_all_movies(json);
    if (json) {
        printf("Filmes:\n%s\n", json);
        free(json);
    } else {
        printf("Nenhum filme encontrado.\n");
    }
}

void list_movies_with_details() {
    char *json;
    select_all_movies_details(json);
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
    select_all_movies_by_genre(genre, json);
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
    select_movie_by_ID(id, json);
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

int main() {
    if (!database_already_created()) {
        execute_sql_file("setup.sql");
        mark_database_as_created();
        printf("Banco de dados criado e configurado.\n");
    } else {
        printf("Banco de dados já configurado.\n");
    }
    int opcao;

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
        scanf("%d", &opcao);
        clear_input();

        switch (opcao) {
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
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != 0);

    return 0;
}