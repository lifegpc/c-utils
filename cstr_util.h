#ifndef _UTILS_CSTR_UTIL_H
#define _UTILS_CSTR_UTIL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
/**
 * @brief Copy string to another string
 * @param dest The pointer of output string
 * @param str The input string
 * @return 0 if successed. `1` - args contains NULL. `2` - Out of memory.
*/
int cstr_util_copy_str(char** dest, const char* str);
/**
 * @brief Check if a string is a integer.
 * @param str String
 * @param allow_sign Allow +/- at the first.
 * @return 1 if is a interger otherwise 0
*/
int cstr_is_integer(const char* str, int allow_sign);
/**
 * @brief Convert string to lowercase
 * @param str Origin string
 * @param input_len The length of origin string. If is 0, strlen will be called to calculate length.
 * @param output Output string. Need free memory by calling free.
 * @return 1 if successed otherwise 0.
*/
int cstr_tolowercase(const char* str, size_t input_len, char** output);
/**
 * @brief Convert bytes to uint32
 * @param bytes Bytes (at least 4 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
uint32_t cstr_read_uint32(const uint8_t* bytes, int big);
/**
 * @brief Convert bytes to int32
 * @param bytes Bytes (at least 4 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
int32_t cstr_read_int32(const uint8_t* bytes, int big);
#ifdef __cplusplus
}
#endif
#endif
