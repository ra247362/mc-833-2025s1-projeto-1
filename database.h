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

void create_movie(const char * const title, int release_year, const char * const * const genres, const char * diretor);
void update_movie_genre(int id, const char * const genre);
void remove_movie(int id);
void select_all_movies();
void select_all_movies_details();
void select_all_movies_by_genre(const char * const genre);
void select_movie_by_ID(int id);