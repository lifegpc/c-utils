#include "cstr_util.h"
#include "utils_config.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if HAVE_PRINTF_S
#define printf printf_s
#endif

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

void cstr_tolowercase2(char* str, size_t len) {
    if (!str) return;
    if (!len) len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
}

uint64_t cstr_read_uint64(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return big ? ((uint64_t)bytes[0] << 56) + ((uint64_t)bytes[1] << 48) + ((uint64_t)bytes[2] << 40) + ((uint64_t)bytes[3] << 32) + ((uint64_t)bytes[4] << 24) + ((uint64_t)bytes[5] << 16) + ((uint64_t)bytes[6] << 8) + (uint64_t)bytes[7] : (uint64_t)bytes[0] + ((uint64_t)bytes[1] << 8) + ((uint64_t)bytes[2] << 16) + ((uint64_t)bytes[3] << 24) + ((uint64_t)bytes[4] << 32) + ((uint64_t)bytes[5] << 40) + ((uint64_t)bytes[6] << 48) + ((uint64_t)bytes[7] << 56);
}

int64_t cstr_read_int64(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return cstr_read_uint64(bytes, big);
}

uint16_t cstr_read_uint16(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return big ? (bytes[0] << 8) + bytes[1] : bytes[0] + (bytes[1] << 8);
}

int16_t cstr_read_int16(const uint8_t* bytes, int big) {
    if (!bytes) return 0;
    return cstr_read_uint16(bytes, big);
}

int cstr_read_str(char* buf, char** dest, size_t* pos, size_t buf_len) {
    if (!buf || !dest || !pos) return 1;
    if (*pos >= buf_len) return 1;
    char* tmp = NULL;
    size_t p = *pos, tmp_len = 128, n = 0;
    tmp = malloc(tmp_len);
    if (!tmp) {
        printf("Out of memory.\n");
        return 1;
    }
    while (p < buf_len) {
        tmp[n] = buf[p];
        if (!buf[p]) break;
        p++;
        n++;
        if (n >= tmp_len) {
            tmp_len += 128;
            char* ntmp = realloc(tmp, tmp_len);
            if (!ntmp) {
                printf("Out of memory.\n");
                free(tmp);
                return 1;
            }
            tmp = ntmp;
        }
    }
    if (p == buf_len && tmp[n - 1]) {
        tmp[n] = 0;
    }
    *pos = p >= buf_len ? p : p + 1;
    char* ntmp = realloc(tmp, n + 1);
    *dest = ntmp ? ntmp : tmp;
    return 0;
}
