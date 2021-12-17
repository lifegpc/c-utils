#ifndef _UTILS_STR_UTIL_H
#define _UTILS_STR_UTIL_H
#include <string>
namespace str_util {
    /**
     * @brief Convert string to lowercase
     * @param ori Origin string
     * @param result Output string.
     * @return true if successed.
    */
    bool tolowercase(std::string ori, std::string& result);
}
#endif
