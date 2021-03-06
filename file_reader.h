#ifndef _UTILS_FILE_READER_H
#define _UTILS_FILE_READER_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if __cplusplus
extern "C" {
#endif
typedef struct file_reader_file file_reader_file;
typedef size_t(*file_reader_file_read)(void* f, size_t buf_len, char* buf);
typedef int(*file_reader_file_seek)(void* f, int64_t offset, int origin);
typedef int64_t(*file_reader_file_tell)(void* f);
/**
 * @brief Create a reader from a stream
 * @param f File stream
 * @param endian 0 if little endian otherwise big endian
 * @return the reader, NULL if OOM or stream is NULL.
*/
file_reader_file* create_file_reader(FILE* f, unsigned char endian);
/**
 * @brief Create a reader from a custom IO stream
 * @param f The data to pass to function
 * @param read The read function. Must not be NULL.
 * @param seek The seek function. Must not be NULL.
 * @param tell The tell function. Must not be NULL.
 * @param endian 0 if little endian otherwise big endian
 * @return the reader, NULL if OOM or any parameters is NULL.
*/
file_reader_file* create_file_reader2(void* f, file_reader_file_read read, file_reader_file_seek seek, file_reader_file_tell tell, unsigned char endian);
/**
 * @brief Free a reader. This will not close stream.
 * @param f the pointer to reader struct
*/
void free_file_reader(file_reader_file* f);
/**
 * @brief Set endian for reader
 * @param f reader
 * @param endian 0 if little endian otherwise big endian
*/
void set_file_reader_endian(file_reader_file* f, unsigned char endian);
/**
 * @brief Aligin the current position
 * @param f reader
 * @return 0 if successed otherwise 1
*/
int file_reader_align(file_reader_file* f);
size_t file_reader_read(file_reader_file* f, size_t buf_len, char* buf);
int file_reader_seek(file_reader_file* f, int64_t offset, int origin);
int64_t file_reader_tell(file_reader_file* f);
/**
 * @brief Read char from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_char(file_reader_file* f, char* re);
/**
 * @brief Read double from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_double(file_reader_file* f, double* re);
/**
 * @brief Read float from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_float(file_reader_file* f, float* re);
/**
 * @brief Read uint8 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_uint8(file_reader_file* f, uint8_t* re);
/**
 * @brief Read int16 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_int16(file_reader_file* f, int16_t* re);
/**
 * @brief Read uint16 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_uint16(file_reader_file* f, uint16_t* re);
/**
 * @brief Read int32 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_int32(file_reader_file* f, int32_t* re);
/**
 * @brief Read uint32 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_uint32(file_reader_file* f, uint32_t* re);
/**
 * @brief Read int64 from reader
 * @param f reader
 * @param re result
 * @return 0 if successed otherwise 1
*/
int file_reader_read_int64(file_reader_file* f, int64_t* re);
/**
 * @brief Read 0 terminal string from reader
 * @param f reader
 * @param buf Result. Need free memory by using free.
 * @return 0 if successed otherwise 1
*/
int file_reader_read_str(file_reader_file* f, char** buf);
/**
 * @brief Read a line (ends with \\r\\n or \\n) from reader
 * @param f reader
 * @param buf Result. (\\r\\n or \\n is removed) Need free memory by using free.
 * @param buf_size The size of the result.
 * @return 0 if successed otherwise 1
*/
int file_reader_read_line(file_reader_file* f, char** buf, size_t* buf_size);
#if __cplusplus
}
#endif
#endif
