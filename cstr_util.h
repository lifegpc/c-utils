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
 * @brief Convert string to uppercase
 * @param str Origin string
 * @param input_len The length of origin string. If is 0, strlen will be called to calculate length.
 * @param output Output string. Need free memory by calling free.
 * @return 1 if successed otherwise 0.
*/
int cstr_touppercase(const char* str, size_t input_len, char** output);
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
/**
 * @brief Convert string to lowercase
 * @param str String needed to convert
 * @param len The length of the string. If is 0, strlen will be called to calculate length.
*/
void cstr_tolowercase2(char* str, size_t len);
/**
 * @brief Convert bytes to uint64
 * @param bytes Bytes (at least 8 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
uint64_t cstr_read_uint64(const uint8_t* bytes, int big);
/**
 * @brief Convert bytes to int64
 * @param bytes Bytes (at least 8 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
int64_t cstr_read_int64(const uint8_t* bytes, int big);
/**
 * @brief Convert bytes to uint16
 * @param bytes Bytes (at least 2 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
uint16_t cstr_read_uint16(const uint8_t* bytes, int big);
/**
 * @brief Convert bytes to int16
 * @param bytes Bytes (at least 2 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
int16_t cstr_read_int16(const uint8_t* bytes, int big);
/**
 * @brief Read string from buffer
 * @param buf Buffer
 * @param dest Result string
 * @param pos The start position in buffer. After a successed called, the position will be updated.
 * @param buf_len The length of buffer
 * @return 0 if successed otherwise 1
*/
int cstr_read_str(char* buf, char** dest, size_t* pos, size_t buf_len);
/**
 * @brief Convert bytes to float
 * @param bytes Bytes (at least 4 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
float cstr_read_float(const uint8_t* bytes, int big);
/**
 * @brief Convert bytes to double
 * @param bytes Bytes (at least 8 bytes)
 * @param big 0 if little endian otherwise big endian
 * @return result
*/
double cstr_read_double(const uint8_t* bytes, int big);
/**
 * @brief Performs a case-insensitive comparison of strings.
 * @param str1 Null-terminated strings to compare.
 * @param str2 Null-terminated strings to compare.
 * @return < 0 str1 less than str2, = 0 str1 identical to str2, > 0 str1 greater than str2
*/
int cstr_stricmp(const char* str1, const char* str2);
#define cstr_strcasecmp cstr_stricmp
/**
 * @brief Compares the specified number of characters of two strings without regard to case.
 * @param str1 Null-terminated strings to compare.
 * @param str2 Null-terminated strings to compare.
 * @param count Number of characters to compare.
 * @return < 0 str1 less than str2, = 0 str1 identical to str2, > 0 str1 greater than str2
*/
int cstr_strnicmp(const char* str1, const char* str2, size_t count);
#define cstr_strncasecmp cstr_strnicmp
#ifdef __cplusplus
}
#endif
#endif
