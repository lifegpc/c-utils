#if HAVE_UTILS_CONFIG_H
#include "utils_config.h"
#endif

#include "cpp2c.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>

#if HAVE_PRINTF_S
#define printf printf_s
#endif

bool cpp2c::string2char(std::string inp, char*& out) {
    size_t le = inp.length();
    char* temp = (char*)malloc(le + 1);
    if (!temp) {
        printf("Out of memory.\n");
        return false;
    }
    memcpy(temp, inp.c_str(), le);
    temp[le] = 0;
    out = temp;
    return true;
}

bool cpp2c::string2char(std::wstring inp, wchar_t*& out) {
    size_t le = inp.length();
    wchar_t* temp = (wchar_t*)malloc((le + 1) * sizeof(wchar_t));
    if (!temp) {
        printf("Out of memory.\n");
        return false;
    }
    memcpy(temp, inp.c_str(), le * sizeof(wchar_t));
    temp[le] = 0;
    out = temp;
    return true;
}
