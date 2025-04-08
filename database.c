#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "lib/cJSON.h"
#include "message.h"
#include <time.h>

#define CONFIG_FILE "config.json"
#define SETUP_FILE "setup.sql"
#define DB_FILE "movies.db"

static sqlite3 *db = NULL;

int execute_sql_file(const char * file_name) {
    char * error = 0;
    FILE *fp = fopen(SETUP_FILE, "r");
    if (!fp) return 0;

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        return 0;
    }

    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, ERROR_DATABASE_OPEN, sqlite3_errmsg(db));
        return 0;
    }

    int rc = sqlite3_exec(db, buffer, 0, 0, &error);
    if (rc != SQLITE_OK) {
        fprintf(stderr, ERROR_DB_SETUP, error);
        free(buffer);
        sqlite3_free(error);
        sqlite3_close(db);
        return 0;
    }
    free(buffer);
    sqlite3_free(error);
    sqlite3_close(db);
    return 1;
}


int database_already_created() {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) return 0;

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(fp);
        return 0;
    }

    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);
    if (!json) return 0;

    cJSON *db_created = cJSON_GetObjectItem(json, "database_created");
    int result = (cJSON_IsBool(db_created) && cJSON_IsTrue(db_created));
    cJSON_Delete(json);
    return result;
}

void mark_database_as_created() {
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "database_created", 1);

    char *rendered = cJSON_Print(json);

    FILE *fp = fopen(CONFIG_FILE, "w");
    if (fp) {
        fputs(rendered, fp);
        fclose(fp);
    }

    cJSON_free(rendered);
    cJSON_Delete(json);
}


//Opening a database connection. Genres are stored as a comma-separated string with no whitespace
int connect_database() {
    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, ERROR_DATABASE_OPEN, sqlite3_errmsg(db));
        return 0;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS movies ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "title TEXT NOT NULL, "
                      "release_year INTEGER, "
                      "genres TEXT, "
                      "director TEXT);";

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, ERROR_TABLE_CREATION, err_msg);
        sqlite3_free(err_msg);
        return 0;
    }

    return 1;
}

void disconnect_database() {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

//Using parameterized query to prevent SQL injection
int create_movie(const char * title, int release_year, const char * genres, const char * diretor) {
    if (!connect_database()) return -1;

    const char *sql = "INSERT INTO movies (title, release_year, genres, director) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, release_year);
    sqlite3_bind_text(stmt, 3, genres, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, diretor, -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_LOCKED) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;  // Wait between nothing 100 nsec and about 10 ms.
        rc = sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    disconnect_database();
    return rc == SQLITE_DONE;
}

int update_movie_genre(int id, const char * const genre) {
    if (!connect_database()) return -1;

    //Retrieve current genres
    const char *sql_select = "SELECT genres FROM movies WHERE id = ?;";
    sqlite3_stmt *stmt_select;

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt_select, NULL) != SQLITE_OK) {
        disconnect_database();
        return -1;
    }

    sqlite3_bind_int(stmt_select, 1, id);

    char *current_genres = NULL;
    int rc = sqlite3_step(stmt_select);
    while (rc == SQLITE_LOCKED) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
        rc = sqlite3_step(stmt_select);
    }
    if (rc == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt_select, 0);
        current_genres = text ? strdup((const char *)text) : strdup("");
    } else {
        sqlite3_finalize(stmt_select);
        disconnect_database();
        return -1;
    }

    sqlite3_finalize(stmt_select);

    //Check if genre already exists (exact words only)
    char *found = strstr(current_genres, genre);
    if (found) {
        size_t len = strlen(genre);
        if ((found == current_genres || *(found - 1) == ',') &&
            (*(found + len) == '\0' || *(found + len) == ',')) {
            free(current_genres);
            disconnect_database();
            return 2;  //Genre already exists
        }
    }

    //Formats string for DB
    char *new_genres = NULL;
    if (strlen(current_genres) == 0) {
        new_genres = strdup(genre);
    } else {
        asprintf(&new_genres, "%s,%s", current_genres, genre);
    }

    free(current_genres);

    //Update movie genres
    const char *sql_update = "UPDATE movies SET genres = ? WHERE id = ?;";
    sqlite3_stmt *stmt_update;

    if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, NULL) != SQLITE_OK) {
        free(new_genres);
        disconnect_database();
        return -1;
    }

    sqlite3_bind_text(stmt_update, 1, new_genres, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt_update, 2, id);

    rc = sqlite3_step(stmt_update);
    while (rc == SQLITE_LOCKED) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
        rc = sqlite3_step(stmt_select);
    }
    sqlite3_finalize(stmt_update);
    disconnect_database();
    free(new_genres);

    return rc == SQLITE_DONE;
}

int remove_movie(int id) {
    if (!connect_database()) return -1;

    const char *sql = "DELETE FROM movies WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    while (rc == SQLITE_LOCKED) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
        rc = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    disconnect_database();
    return rc == SQLITE_DONE;
}

int select_all_movies(char ** result) {
    if (!connect_database()) return -1;

    const char *sql = "SELECT id, title FROM movies;";
    sqlite3_stmt *stmt;
    cJSON *json = cJSON_CreateObject();
    cJSON *query_results = cJSON_CreateArray();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        while (rc == SQLITE_LOCKED) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
            rc = sqlite3_step(stmt);
        }
        while (rc == SQLITE_ROW) {
            cJSON *movie = cJSON_CreateObject();
            cJSON_AddNumberToObject(movie, "id", sqlite3_column_int(stmt, 0));
            cJSON_AddStringToObject(movie, "title", (const char *)sqlite3_column_text(stmt, 1));
            cJSON_AddItemToArray(query_results, movie);
            rc = sqlite3_step(stmt);
        }
    }
    cJSON_AddItemToObject(json, "query_results", query_results);

    sqlite3_finalize(stmt);
    disconnect_database();

    *result = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return 1;
}

int select_all_movies_details(char ** result) {
    if (!connect_database()) return -1;

    const char *sql = "SELECT id, title, release_year, genres, director FROM movies;";
    sqlite3_stmt *stmt;

    cJSON *json = cJSON_CreateObject();
    cJSON *query_results = cJSON_CreateArray();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        while (rc == SQLITE_LOCKED) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
            rc = sqlite3_step(stmt);
        }
        while (rc == SQLITE_ROW) {
            cJSON *movie = cJSON_CreateObject();
            cJSON_AddNumberToObject(movie, "id", sqlite3_column_int(stmt, 0));
            cJSON_AddStringToObject(movie, "title", (const char *)sqlite3_column_text(stmt, 1));
            cJSON_AddNumberToObject(movie, "release_year", sqlite3_column_int(stmt, 2));
            cJSON_AddStringToObject(movie, "genres", (const char *)sqlite3_column_text(stmt, 3));
            cJSON_AddStringToObject(movie, "director", (const char *)sqlite3_column_text(stmt, 4));
            cJSON_AddItemToArray(query_results, movie);
            rc = sqlite3_step(stmt);
        }
    }
    cJSON_AddItemToObject(json, "query_results", query_results);


    sqlite3_finalize(stmt);
    disconnect_database();
    *result = cJSON_PrintUnformatted(json);

    cJSON_Delete(json);
    return 1;
}

int select_movie_by_ID(int id, char ** result) {
    if (!connect_database()) return -1;

    const char *sql = "SELECT id, title, release_year, genres, director FROM movies WHERE id = ?;";
    sqlite3_stmt *stmt;

    cJSON *movie = cJSON_CreateObject();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        int rc = sqlite3_step(stmt);
        while (rc == SQLITE_LOCKED) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
            rc = sqlite3_step(stmt);
        }
        if (rc == SQLITE_ROW) {
            cJSON_AddNumberToObject(movie, "id", sqlite3_column_int(stmt, 0));
            cJSON_AddStringToObject(movie, "title", (const char *)sqlite3_column_text(stmt, 1));
            cJSON_AddNumberToObject(movie, "release_year", sqlite3_column_int(stmt, 2));
            cJSON_AddStringToObject(movie, "genres", (const char *)sqlite3_column_text(stmt, 3));
            cJSON_AddStringToObject(movie, "director", (const char *)sqlite3_column_text(stmt, 4));
            rc = sqlite3_step(stmt);
        }
    }

    sqlite3_finalize(stmt);
    disconnect_database();

    *result = cJSON_PrintUnformatted(movie);
    cJSON_Delete(movie);
    return 1;
}

int select_all_movies_by_genre(const char * const genre, char ** result) {
    if (!connect_database()) return -1;

    const char *sql = "SELECT id, title FROM movies WHERE genres LIKE ?;";
    sqlite3_stmt *stmt;

    cJSON *json = cJSON_CreateObject();
    cJSON *query_results = cJSON_CreateArray();
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        char like_param[128];
        snprintf(like_param, sizeof(like_param), "%%%s%%", genre);
        sqlite3_bind_text(stmt, 1, like_param, -1, SQLITE_STATIC);
        int rc = sqlite3_step(stmt);
        while (rc == SQLITE_LOCKED) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (rand() % 1000l) * 100000l + 100l;
            rc = sqlite3_step(stmt);
        }
        while (rc == SQLITE_ROW) {
            cJSON *movie = cJSON_CreateObject();
            cJSON_AddNumberToObject(movie, "id", sqlite3_column_int(stmt, 0));
            cJSON_AddStringToObject(movie, "title", (const char *)sqlite3_column_text(stmt, 1));
            cJSON_AddItemToArray(query_results, movie);
            rc = sqlite3_step(stmt);
        }
    }
    cJSON_AddItemToObject(json, "query_results", query_results);

    sqlite3_finalize(stmt);
    disconnect_database();

    *result = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return 1;
}

// int main() {
//     char *json;
//     select_all_movies(&json);
//     if (json) {
//         printf("Filmes:\n%s\n", json);
//         free(json);
//     } else {
//         printf("Nenhum filme encontrado.\n");
//     }
//     return 0;
// }