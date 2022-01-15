#include "cstr_util.h"
#include "utils_config.h"

#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

typedef enum float_format {
    undetected_endian,
    ieee_big_endian,
    ieee_little_endian,
    unknown_endian,
} float_format_type;

static float_format_type double_format = undetected_endian;
static float_format_type float_format = undetected_endian;

void detect_double_format() {
    if (sizeof(double) != 8) double_format = unknown_endian;
    else {
        double x = 9006104071833999.0;
        if (!memcmp(&x, "C?\xff\x01\x02\x03\t\x8f", 8)) {
            double_format = ieee_big_endian;
        } else if (!memcmp(&x, "\x8f\t\x03\x02\x01\xff?C", 8)) {
            double_format = ieee_little_endian;
        } else {
            double_format = unknown_endian;
        }
    }
}

void detect_float_format() {
    if (sizeof(float) != 4) float_format = unknown_endian;
    else {
        float x = 213213216.0;
        if (!memcmp(&x, "MKV\x02", 4)) {
            float_format = ieee_big_endian;
        } else if (!memcmp(&x, "\x02VKM", 4)) {
            float_format = ieee_little_endian;
        } else {
            float_format = unknown_endian;
        }
    }
}

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

float cstr_read_float(const uint8_t* p, int big) {
    if (float_format == undetected_endian) detect_float_format();
    if (float_format == unknown_endian) {
        unsigned char sign;
        int e;
        unsigned int f;
        float x;
        int incr = 1;
        if (!big) {
            p += 3;
            incr = -1;
        }
        sign = (*p >> 7) & 1;
        e = (*p & 0x7F) << 1;
        p += incr;
        e |= (*p >> 7) & 1;
        f = (*p & 0x7F) << 16;
        p += incr;
        f |= *p << 8;
        p += incr;
        f |= *p;
        if (e == 255) {
            if (f == 0) return sign ? -INFINITY : INFINITY;
            else return NAN;
        }
        x = (float)f / 8388608.0;
        if (e == 0) {
            e = -126;
        } else {
            x += 1.0;
            e -= 127;
        }
        x = ldexpf(x, e);
        if (sign) x = -x;
        return x;
    } else {
        float x;
        if ((float_format == ieee_little_endian && big) || (float_format == ieee_big_endian && !big)) {
            char buf[4];
            char* d = &(buf[3]);
            int i;
            for (i = 0; i < 4; i++) {
                *d-- = *p++;
            }
            memcpy(&x, buf, 4);
        } else {
            memcpy(&x, p, 4);
        }
        return x;
    }
}

double cstr_read_double(const uint8_t* p, int big) {
    if (double_format == undetected_endian) detect_double_format();
    if (double_format == unknown_endian) {
        unsigned char sign;
        int e;
        unsigned int fhi, flo;
        double x;
        int incr = 1;
        if (!big) {
            p += 7;
            incr = -1;
        }
        sign = (*p >> 7) & 1;
        e = (*p & 0x7F) << 4;
        p += incr;
        e |= (*p >> 4) & 0xF;
        fhi = (*p & 0xF) << 24;
        p += incr;
        fhi |= *p << 16;
        p += incr;
        fhi |= *p << 8;
        p += incr;
        fhi |= *p;
        p += incr;
        flo = *p << 16;
        p += incr;
        flo |= *p << 8;
        p += incr;
        flo |= *p;
        if (e == 2047) {
            if (flo == 0 && fhi == 0) {
                return sign ? -INFINITY : INFINITY;
            } else {
                float a = NAN;
                return a;
            }
        }
        x = (double)fhi + (double)flo / 16777216.0;
        x /= 268435456.0;
        if (e == 0)
            e = -1022;
        else {
            x += 1.0;
            e -= 1023;
        }
        x = ldexp(x, e);
        if (sign) x = -x;
        return x;
    } else {
        double x;
        if ((double_format == ieee_little_endian && big) || (double_format == ieee_big_endian && !big)) {
            char buf[8];
            char* d = &(buf[7]);
            int i;
            for (i = 0; i < 8; i++) {
                *d-- = *p++;
            }
            memcpy(&x, buf, 8);
        } else {
            memcpy(&x, p, 8);
        }
        return x;
    }
}

int cstr_stricmp(const char* str1, const char* str2) {
#if HAVE__STRICMP
    return _stricmp(str1, str2);
#elif HAVE_STRCASECMP
    return strcasecmp(str1, str2);
#else
    size_t le = strlen(str1), le2 = strlen(str2);
    size_t mle = min(le, le2);
    for (size_t i = 0; i < mle; i++) {
        int a = tolower(str1[i]), b = tolower(str2[i]);
        if (a < b) return -1;
        else if (a > b) return 1;
    }
    if (le > le2) return 1;
    else if (le < le2) return -1;
    return 0;
#endif
}

int cstr_strnicmp(const char* str1, const char* str2, size_t count) {
#if HAVE__STRNICMP
    return _strnicmp(str1, str2, count);
#elif HAVE_STRNCASECMP
    return strncasecmp(str1, str2, count);
#else
    size_t le = strlen(str1), le2 = strlen(str2);
    size_t mle = min(le, le2);
    for (size_t i = 0; i < mle; i++) {
        if (i == count) return 0;
        int a = tolower(str1[i]), b = tolower(str2[i]);
        if (a < b) return -1;
        else if (a > b) return 1;
    }
    if (mle == count) return 0;
    if (le > le2) return 1;
    else if (le < le2) return -1;
    return 0;
#endif
}
