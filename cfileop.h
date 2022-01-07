#ifndef _UTIL_CFILEOP_H
#define _UTIL_CFILEOP_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
/**
 * @brief Check file exists
 * @param fn File name
 * @returns 1 if file is exists otherwise 0
*/
int fileop_exists(const char* fn);
/**
 * @brief Remove file
 * @param fn File name
 * @returns 1 if successed otherwise 0
*/
int fileop_remove(const char* fn);
/**
 * @brief Get directory name from path
 * @param fn Path
 * @returns Directory. If path does not contain a slash, will return "". Need free memory by using free.
*/
char* fileop_dirname(const char* fn);
/**
 * @brief Detect if path is a url.
 * @param fn Path
 * @param re result
 * @return 1 if successed otherwise 0
*/
int fileop_is_url(const char* fn, int* re);
/**
 * @brief Get file name from path
 * @param fn Path
 * @return File name. Need free memory by calling free.
*/
char* fileop_basename(const char* fn);
/**
 * @brief Parse size string
 * @param size Such as "10KiB", "10", "34B", "48K"
 * @param fs size
 * @param is_byte Whether to return bits or bytes
 * @return 1 if successed otherwise 0
*/
int fileop_parse_size(const char* size, size_t* fs, int is_byte);
/**
 * @brief Opens a file.
 * @param fn File name.
 * @param fd file descriptor. -1 if error occured.
 * @param oflag The kind of operations allowed.
 * @param shflag The kind of sharing allowed. (Windows Only) 0 will be replace with 0x10.
 * @param pmode Permission mode. (Windows Only)
 * @return errno
*/
int fileop_open(const char* fn, int* fd, int oflag, int shflag, int pmode);
/**
 * @brief Check if path is absoulte path.
 * @param path The path.
 * @return 1 if path is absoulte path otherwise 0
*/
int fileop_isabs(const char* path);
/**
 * @brief Join two more pathname components
 * @param path 
 * @param path2 
 * @return Result. Need free memory by using free.
*/
char* fileop_join(const char* path, const char* path2);
/**
 * @brief check if path is an existing directory.
 * @param path Path
 * @param result 1 if path is an existing directory otherwise 0
 * @return 0 if error occured otherwise 1
*/
int fileop_isdir(const char* path, int* result);
/**
 * @brief Creates a new directory.
 * @param path Path for a new directory.
 * @param mode Directory permission. (Linux only)
 * @return 1 if created successfully otherwise 0
*/
int fileop_mkdir(const char* path, int mode);
/**
 * @brief Sets the date and time that the specified file or directory was created, last accessed, or last modified.
 * @param path The path of directory or file
 * @param ctime creation date (Windows Only)
 * @param actime Last access date
 * @param modtime Last modification date
 * @return 1 if successed otherwise 0.
*/
int fileop_set_file_time(const char* path, time_t ctime, time_t actime, time_t modtime);
#if _WIN32
#define fileop_fdopen _fdopen
#else
#define fileop_fdopen fdopen
#endif
#if _WIN32
#define fileop_close(fd) (!_close(fd))
#else
#define fileop_close(fd) (!close(fd))
#endif
#define fileop_fclose(stream) (!fclose(stream))
/**
 * @brief Creates a new directory recursively.
 * @param path Path for a new directory.
 * @param mode Directory permission. (Linux only)
 * @param allow_exists If directory is already exists return 1 rather than 0.
 * @return 1 if create successfully otherwise 0
*/
int fileop_mkdirs(const char* path, int mode, int allow_exists);
/**
 * @brief Retrieves the size of the specified file, in bytes.
 * @param path The path of file.
 * @param size Result.
 * @return 1 if successed otherwise 0
*/
int fileop_get_file_size(const char* path, size_t* size);
/**
 * @brief Moves the file pointer to a specified location.
 * @param f File
 * @param offset Number of bytes from origin.
 * @param origin Initial position.
 * @return 0 if successed otherwise a nonzero value
*/
int fileop_fseek(FILE* f, int64_t offset, int origin);
/**
 * @brief Make sure file's directory is already exists, if not exists, try create it.
 * @param path File's path
 * @param mode Directory permission. (Linux only)
 * @return 1 if file's directory is exists now otherwise 0.
*/
int fileop_mkdir_for_file(const char* path, int mode);
/**
 * @brief Gets the current position of a file pointer.
 * @param f Target FILE structure.
 * @return The current position
*/
int64_t fileop_ftell(FILE* f);
#ifdef __cplusplus
}
#endif
#endif
