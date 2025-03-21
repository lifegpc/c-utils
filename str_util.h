#ifndef _UTILS_STR_UTIL_H
#define _UTILS_STR_UTIL_H
#include <list>
#include <stddef.h>
#include <string>
#include <vector>
namespace str_util {
    /**
     * @brief Convert string to lowercase
     * @param ori Origin string
     * @param result Output string.
     * @return true if successed.
    */
    bool tolowercase(std::string ori, std::string& result);
    /**
     * @brief Convert string to uppercase
     * @param ori Origin string
     * @param result Output string.
     * @return true if successed.
    */
    bool touppercase(std::string ori, std::string& result);
    std::string tolower(std::string ori);
    std::string toupper(std::string ori);
    /**
     * @brief Replace all pattern to new_content
     * @param input Input string
     * @param pattern Pattern
     * @param new_content New content
     * @return Result string
    */
    std::string str_replace(std::string input, std::string pattern, std::string new_content);
    /**
     * @brief Split string with pattern.
     * @param input Input string
     * @param pattern Partten
     * @param max Maximum count of result.
     * @param break_quote Break quote.
     * @return Result.
    */
    std::list<std::string> str_split(std::string input, std::string pattern, size_t max = -1, bool break_quote = true);
    /**
     * @brief Split string with pattern.
     * @param input Input string
     * @param pattern Partten
     * @param max Maximum count of result.
     * @param break_quote Break quote.
     * @return Result.
    */
    std::vector<std::string> str_splitv(std::string input, std::string pattern, size_t max = -1, bool break_quote = true);
    /**
     * @brief Convert data to a lowercase hexadecimal string
     * @param input Input data
     * @return Output
    */
    std::string str_hex(std::string input);
    /**
     * @brief Check if a string ends with a pattern
     * @param input Input data
     * @param pattern Pattern
     * @return true if input ends with pattern
    */
    bool str_endswith(std::string input, std::string pattern);
    /**
     * @brief Trim a string
     * @param input Input string
     * @return Result
    */
    std::string str_trim(std::string input);
    /**
     * @brief Remove quote from a string
     * @param input Input string
     * @return Result
    */
    std::string remove_quote(std::string input);
    /**
     * @brief Join a list of string with a pattern
     * @param input Input list
     * @param pattern Pattern
     * @return Result
    */
    std::string str_join(std::list<std::string> input, std::string pattern);
    /**
     * @brief Parse a string to a boolean
     * @param input Input string
     * @return Result
     */
    bool parse_bool(std::string input);
}
#endif
