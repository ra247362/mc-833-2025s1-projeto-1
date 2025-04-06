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

// POST TITLE RELEASE_YEAR GENRES[COMMA SEPARATED, NO WHITESPACE] DIRECTOR
int create_movie(const char * const title, int release_year, const char * const * const genres, const char * diretor);
// PUT ID GENRE
int update_movie_genre(int id, const char * const genre);
// DELETE ID
int remove_movie(int id);
// GET ALL
char * select_all_movies();
// GET ALL_DETAILED
char * select_all_movies_details();
// GET ALL_GENRE GENRE
char * select_all_movies_by_genre(const char * const genre);
// GET SINGLE ID
char * select_movie_by_ID(int id);