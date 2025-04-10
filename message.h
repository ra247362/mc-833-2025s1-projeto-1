/*
    Error Messages
*/ 

// DB CONFIG

#define ERROR_INVALID_FILE_SIZE "Could not allocate buffer due to invalid file size.\n"
#define ERROR_BUFFER_ALLOCATION "Improper buffer allocation.\n"
#define ERROR_FILE_READ "Could not read file.\n"
#define ERROR_CJSON_BEFORE_CHARACTER "cJSON error before: %s\n"

// DB

#define DB_ALREADY_SETUP "DATABASE: Already configured. All OK.\n"
#define DB_SETUP_COMPLETE "DATABASE: Configured. All OK.\n"

#define ERROR_MSG_DB_INVALID_ID "ERROR: movie of given ID does not exist.\n"
#define ERROR_MSG_DB_INVALID_GENRE "ERROR: movie already has given genre.\n"
#define ERROR_MSG_DB_MISC_DB "ERROR: database was unable to complete operation.\n"
#define ERROR_DATABASE_OPEN "ERROR: Could not open database: %s\n"
#define ERROR_TABLE_CREATION "ERROR: Could not create table: %s\n"
#define ERROR_DB_SETUP "ERROR: SQLite failed: %s\n"

#define DB_MSG_COLUMNS_SHORT  "ID | TITLE\n"
#define DB_MSG_COLUMNS_FULL "ID | TITLE | YEAR | GENRES | DIRECTOR\n"

// CLIENT
#define CLIENT_MENU "\n====== PIPOCA Client v%d.0 ======\n"
#define CLIENT_ADD_MOVIE "1. Add movie\n"
#define CLIENT_ADD_GENRE "2. Add genre to movie\n"
#define CLIENT_LIST_MOVIES "3.List all movies\n"
#define CLIENT_LIST_DETAILS "4.List all movies (detailed)\n"
#define CLIENT_LIST_GENRE "5. List movies by genre\n"
#define CLIENT_LIST_ID "6.List movie details by ID\n"
#define CLIENT_REMOVE "7. Remove movie by ID\n"
#define CLIENT_LEAVE "0. Leave\n"
#define CLIENT_SELECT_OPTION "Choose a operation by inserting its number: "
#define CLIENT_QUIT "Quitting...\n"
#define CLIENT_INVALID_OPTION "Invalid option!\n"

#define CLIENT_CONNECTING_TO_SERVER "CLIENT: Connected to server.\n"

#define CLIENT_NO_MOVIES_FOUND "No movies matched the given query.\n"

#define CLIENT_QUERY_BUILDER_TITLE "Please insert the movie's title: "
#define CLIENT_QUERY_BUILDER_YEAR "Please insert the movie's release year: "
#define CLIENT_QUERY_BUILDER_DIRECTOR "Please insert the movie's main director: "
#define CLIENT_QUERY_BUILDER_GENRE_NUMBER "Please insert the number of genres the movie has (max. %d): "
#define CLIENT_QUERY_BUILDER_GENRE "Please provide a genre number %d: "

#define CLIENT_QUERY_BUILDER_ID "Please provide the ID of the movie: "
#define CLIENT_QUERY_BUILDER_SINGLE_GENRE "Please provide the genre you would like to add to the movie: "
#define CLIENT_QUERY_BUILDER_SEARCH_GENRE "Please provide the provide to filter movies by: "

#define SERVER_CLOSED "STOPPED: Server closed connection.\n"

#define ERROR_CLIENT_SOCKET "CLIENT: Socket"
#define ERROR_CLIENT_CONNECT "CLIENT: Connect"
#define ERROR_CLIENT_CONNECTION "CLIENT: Failed to connect.\n"

// SERVER
#define ERROR_MSG_INVALID_COMMAND "ERROR: Invalid command.\n"
#define ERROR_MSG_INVALID_GET "ERROR: Invalid GET.\n"
#define ERROR_MSG_INVALID_POST "ERROR: Invalid POST.\n"
#define ERROR_MSG_INVALID_PUT "ERROR: Invalid PUT.\n"
#define ERROR_MSG_INVALID_DELETE "ERROR: Invalid DELETE.\n"
#define ERROR_MSG_CONNECTION_ERROR "ERROR: Connection error.\n"
#define ERROR_ACCEPT "ACCEPT"
#define ERROR_LISTEN "LISTEN"
#define ERROR_SERVER_SOCKET "SERVER: Socket"
#define ERROR_SETSOCKOPT "SETSOCKOPT"
#define ERROR_SERVER_BIND "SERVER: Bind"
#define ERROR_FAIL_TO_BIND "SERVER: Failed to bind.\n"
#define ERROR_MSG "ERROR"

#define CLIENT_DISCONNECTED "STOPPED: Client at FD %d disconnected.\n"
#define SERVER_DISCONNECTED "STOPPED: Server disconnected to client at FD %d\n"
#define SERVER_CONNECTED_FD "INIT: Server connected to client at %s on FD %d.\n"

/*
    Success Messages
*/

#define SUCCESS_MSG_GET "SUCCESS: Item(s) retrieved.\n"
#define SUCCESS_MSG_POST "SUCCESS: Item created.\n"
#define SUCCESS_MSG_PUT "SUCCESS: Item updated.\n"
#define SUCCESS_MSG_DELETE "SUCCESS: Item deleted.\n"
