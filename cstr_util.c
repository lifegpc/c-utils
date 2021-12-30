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

int cstr_tolowercase(const char* str, size_t input_len, char** output) {
    if (!str || !output) return 0;
    if (!input_len) input_len = strlen(str);
    if (input_len == (size_t)-1) return 0;
    char* tmp = malloc(input_len + 1);
    if (!tmp) return 0;
    size_t i = 0;
    for (; i < input_len; i++) {
        tmp[i] = tolower(str[i]);
    }
    tmp[input_len] = 0;
    *output = tmp;
    return 1;
}

uint32_t cstr_read_uint32(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return big ? (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3] : bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
}

int32_t cstr_read_int32(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return cstr_read_uint32(bytes, big);
}
