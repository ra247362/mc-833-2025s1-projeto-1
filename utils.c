#include "utils.h"
#include <string.h>
#include <ctype.h>

// Verifies that number is an int.
int is_int(const char * str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

unsigned int str_to_int(const char * str) {

}