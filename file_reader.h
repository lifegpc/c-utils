#ifndef _UTILS_FILE_READER_H
#define _UTILS_FILE_READER_H
#include <stdint.h>
#include <stdio.h>
#if __cplusplus
extern "C" {
#endif
typedef struct file_reader_file file_reader_file;
/**
 * @brief Create a reader from a stream
 * @param f File stream
 * @param endian 0 if little endian otherwise big endian
 * @return the reader, NULL if OOM or stream is NULL.
*/
file_reader_file* create_file_reader(FILE* f, unsigned char endian);
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
#if __cplusplus
}
#endif
#endif
