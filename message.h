/*
    Error Messages
*/ 

// DB CONFIG

#define ERROR_INVALID_FILE_SIZE "Could not allocate buffer due to invalid file size."
#define ERROR_BUFFER_ALLOCATION "Improper buffer allocation."
#define ERROR_FILE_READ "Could not read file."
#define ERROR_CJSON_BEFORE_CHARACTER "cJSON error before: %s\n"

// DB

#define ERROR_MSG_DB_INVALID_ID "ERROR: movie of given ID does not exist."
#define ERROR_MSG_DB_INVALID_GENRE "ERROR: movie already has given genre."
#define ERROR_MSG_DB_MISC_DB "ERROR: database was unable to complete operation."
#define ERROR_DATABASE_OPEN "ERROR: Could not open database: %s\n"
#define ERROR_TABLE_CREATION "ERROR: Could not create table: %s\n"
#define ERROR_DB_SETUP "ERROR: SQLite failed: %s"

// CLIENT

// SERVER
#define ERROR_MSG_INVALID_COMMAND "ERROR: Invalid command."
#define ERROR_MSG_INVALID_GET "ERROR: Invalid GET."
#define ERROR_MSG_INVALID_POST "ERROR: Invalid POST."
#define ERROR_MSG_INVALID_PUT "ERROR: Invalid PUT."
#define ERROR_MSG_INVALID_DELETE "ERROR: Invalid DELETE."

/*
    Success Messages
*/

#define SUCCESS_MSG_GET "SUCCESS: Item(s) retrieved."
#define SUCCESS_MSG_POST "SUCCESS: Item created."
#define SUCCESS_MSG_PUT "SUCCESS: Item updated."
#define SUCCESS_MSG_DELETE "SUCCESS: Item deleted."