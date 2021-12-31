#include "str_util.h"
#include "cstr_util.h"
#include <malloc.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

bool str_util::tolowercase(std::string ori, std::string& result) {
    char* tmp = nullptr;
    auto re = cstr_tolowercase(ori.c_str(), ori.length(), &tmp);
    if (re) {
        result = std::string(tmp, ori.length());
        free(tmp);
        return true;
    } else {
        return false;
    }
}

std::string str_util::str_replace(std::string input, std::string pattern, std::string new_content) {
    auto loc = input.find(pattern, 0);
    auto len = pattern.length();
    auto len2 = new_content.length();
    while (loc != -1) {
        input.replace(loc, len, new_content);
        if (loc + len2 < input.length()) loc = input.find(pattern, max(0, loc + len2));
        else break;
    }
    return input;
}