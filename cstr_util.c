#include "cstr_util.h"

#include <malloc.h>
#include <string.h>
#include <ctype.h>

int cstr_util_copy_str(char** dest, const char* str) {
    if (!dest || !str) return 1;
    size_t le = strlen(str);
    char* temp = malloc(le + 1);
    if (!temp) {
        return 2;
    }
    memcpy(temp, str, le);
    temp[le] = 0;
    *dest = temp;
    return 0;
}

int cstr_is_integer(const char* str, int allow_sign) {
    if (!str) return 0;
    size_t le = strlen(str), i = 0;
    if (!le) return 0;
    if (allow_sign && le > 1 && (str[0] == '+' || str[0] == '-')) i++;
    for (; i < le; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}
