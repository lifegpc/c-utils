#include "str_util.h"
#include "cstr_util.h"
#include <malloc.h>

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
