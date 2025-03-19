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

bool str_util::touppercase(std::string ori, std::string& result) {
    char* tmp = nullptr;
    auto re = cstr_touppercase(ori.c_str(), ori.length(), &tmp);
    if (re) {
        result = std::string(tmp, ori.length());
        free(tmp);
        return true;
    } else {
        return false;
    }
}

std::string str_util::tolower(std::string ori) {
    std::string result;
    tolowercase(ori, result);
    return result;
}

std::string str_util::toupper(std::string ori) {
    std::string result;
    touppercase(ori, result);
    return result;
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

std::list<std::string> str_util::str_split(std::string input, std::string pattern, size_t max, bool break_quote) {
    if (max == 0) return {};
    std::list<std::string> li;
    auto loc = input.find(pattern, 0);
    auto last_loc = 0;
    auto pre_loc = 0;
    auto len = pattern.length();
    while (loc != -1 && li.size() < max) {
        auto s = input.substr(pre_loc, loc - pre_loc);
        if (break_quote || (s.front() != '"' && s.front() != '\'') || s.back() == s.front()) {
            li.push_back(s);
            pre_loc = loc + len;
        }
        last_loc = loc + len;
        if (last_loc < input.length()) loc = input.find(pattern, max(0, last_loc));
        else break;
    }
    if (last_loc <= input.length()) li.push_back(input.substr(pre_loc, input.length() - pre_loc));
    return li;
}

std::vector<std::string> str_util::str_splitv(std::string input, std::string pattern, size_t max, bool break_quote) {
    auto list = str_split(input, pattern, max, break_quote);
    std::vector<std::string> vec(list.begin(), list.end());
    return vec;
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

bool str_util::str_endswith(std::string input, std::string pattern) {
    auto ilen = input.length();
    auto plen = pattern.length();
    if (ilen < plen) return false;
    auto i = input.rfind(pattern);
    return i == ilen - plen; 
}

std::string str_util::remove_quote(std::string input) {
    if (input.length() < 2) return input;
    if (input[0] == '"' && input[input.length() - 1] == '"') {
        return input.substr(1, input.length() - 2);
    } else if (input[0] == '\'' && input[input.length() - 1] == '\'') {
        return input.substr(1, input.length() - 2);
    } else {
        return input;
    }
}

std::string str_util::str_trim(std::string input) {
    while (!input.empty() && (input.front() == ' ' || input.front() == '\t' || input.front() == '\r' || input.front() == '\n')) {
        input.erase(input.begin());
    }
    while (!input.empty() && (input.back() == ' ' || input.back() == '\t' || input.back() == '\r' || input.back() == '\n')) {
        input.erase(input.end() - 1);
    }
    return input;
}

std::string str_util::str_join(std::list<std::string> input, std::string pattern) {
    std::string output;
    for (auto i = input.begin(); i != input.end();) {
        output += *i;
        i++;
        if (i != input.end()) output += pattern;
    }
    return output;
}

bool str_util::parse_bool(std::string input) {
    input = tolower(input);
    if (input == "true" || input == "yes" || input == "on" || input == "1") {
        return true;
    } else {
        return false;
    }
}
