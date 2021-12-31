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
    /**
     * @brief Replace all pattern to new_content
     * @param input Input string
     * @param pattern Pattern
     * @param new_content New content
     * @return Result string
    */
    std::string str_replace(std::string input, std::string pattern, std::string new_content);
}
#endif
