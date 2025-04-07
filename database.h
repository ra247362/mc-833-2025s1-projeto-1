#include "lib/sqlite3.h"

/*
Functions:
    - Create movie (ID (SQLite handled), string Name, int ReleaseYear, string[] Genres, string Director)
    - Update genre (ID_Movie, string Genre)
    - Remove movie (ID_Movie)

    - List all movies (ID and movie title)
    - List all movies with details (All details)
    - List all movie by genre (movie title)
    - List details from movie by ID (ID_Movie)
*/

//CHECKS IF THE DATABASE HAS ALREADY BEEN CREATED
int database_already_created();
//MARK DATABASE AS CREATED ON FIRST EXECUTION
void mark_database_as_created();
//EXECUTE THE SQL FILE CONTAINING THE FIRST MOVIES
void execute_sql_file(const char * file_name);
// POST TITLE RELEASE_YEAR GENRES[COMMA SEPARATED, NO WHITESPACE] DIRECTOR
int create_movie(const char * title, int release_year, const char * const genres, const char * director);
// PUT ID GENRE
int update_movie_genre(int id, const char * const genre);
// DELETE ID
int remove_movie(int id);
// GET ALL
int select_all_movies(char * result);
// GET ALL_DETAILED
int  select_all_movies_details(char * result);
// GET ALL_GENRE GENRE
int select_all_movies_by_genre(const char * const genre, char * result);
// GET SINGLE ID
int select_movie_by_ID(int id, char * result);