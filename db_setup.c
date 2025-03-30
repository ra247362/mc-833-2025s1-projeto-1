#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "lib/sqlite3.h"
#include "lib/cJSON.h"

/*
what this needs to to:
- Open config and check if valid; -> DONE
- Check if DB was configured via config variable;
- If configured: open DB connection and return pointer;
- If not configured: create DB and setup data, return pointer;
*/

/*
    Change if required.
    Default is root folder for the project.
*/
const char* CONFIG_PATH = "";
const char* CONFIG_NAME = "config.json";

int parse_config(const char * const config) {
    int configured = -1;
    const cJSON *database_created = NULL;
    cJSON *config_json = cJSON_Parse(config);
    if (config_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, ERROR_CJSON_BEFORE_CHARACTER, error_ptr);
        }
        cJSON_Delete(config_json);
        return configured;
    }
    database_created = cJSON_GetObjectItem(config_json, "database_created");

    if (cJSON_IsBool(database_created)) {
        configured = cJSON_IsTrue(database_created);
    }

    cJSON_Delete(config_json);
    return configured;
}

int main(void) {
    char *config_source;
    FILE *fp = fopen(CONFIG_NAME, "r+");
    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0) {
            long buffer_size = ftell(fp);
            if (buffer_size == 1) {
                fprintf(stderr, ERROR_INVALID_FILE_SIZE);
                exit(-1);
            }
            config_source = malloc(sizeof(char) * (buffer_size + 1));
            if (fseek(fp, 0L, SEEK_SET) != 0) { 
                fprintf(stderr, ERROR_BUFFER_ALLOCATION);
                exit(-1);
             }

            size_t newLen = fread(config_source, sizeof(char), buffer_size, fp);

            if (ferror(fp) != 0 ) {
                fprintf(stderr, ERROR_FILE_READ);
            } else {
                config_source[newLen++] = '\0';
            }
        }

        fclose(fp);
    }
    printf("Result: %d\n", parse_config(config_source));
    free(config_source);
    return 0;
}
