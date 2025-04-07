#include "utils.h"
#include <string.h>
#include <ctype.h>

/*
Verifies that number is an unsigned int.
If yes, returns 1 and result points to the number represented.
If not, returns 0 and result points to NULL.
*/
int str_to_int(const char * str, unsigned int * result) {
    *result = 0;
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!isdigit(str[i])) {
            result = NULL;
            return 0;
        }
    }
    
    for (int i = 0; i < len; i++) {
        // Check for overflow
        if (*result > __UINT32_MAX__ - (str[i]-'0')) {
            result = NULL;
            return 0;
        }
        *result += (str[i]-'0');
        if (i < len-1) {
            if (*result > __UINT32_MAX__ / 10U) {
                result = NULL;
                return 0;
            }
            *result = *result * 10U;
        }
    }
    return 1;
}
