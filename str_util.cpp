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

std::list<std::string> str_util::str_split(std::string input, std::string pattern, size_t max) {
    if (max == 0) return {};
    std::list<std::string> li;
    auto loc = input.find(pattern, 0);
    auto last_loc = 0;
    auto len = pattern.length();
    while (loc != -1 && li.size() < max) {
        li.push_back(input.substr(last_loc, loc - last_loc));
        last_loc = loc + len;
        if (last_loc < input.length()) loc = input.find(pattern, max(0, last_loc));
        else break;
    }
    if (last_loc <= input.length()) li.push_back(input.substr(last_loc, input.length() - last_loc));
    return li;
}

std::string str_util::str_hex(std::string input) {
    if (input.empty()) return "";
    const char* t = "0123456789abcdef";
    std::string output;
    for (auto i = input.begin(); i != input.end(); i++) {
        unsigned char c = *i;
        output += t[c / 16];
        output += t[c % 16];
    }
    return output;
}
