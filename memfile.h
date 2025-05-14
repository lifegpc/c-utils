#ifndef _UTIL_MEMFILE_H
#define _UTIL_MEMFILE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
typedef struct MemFile {
    char* data;
    size_t len;
    size_t loc;
} MemFile;
typedef struct CMemFile {
    const char* data;
    size_t len;
    size_t loc;
} CMemFile;

/**
 * @brief Create a new memory file
 * @param data Data. Will allocate new memory for this data.
 * @param len The size of data.
 * @return MemFile struct if succeessed otherwise NULL.
*/
MemFile* new_memfile(const char* data, size_t len);
/**
 * @brief Create a new memory file
 * @param data Data. Will not allocate new memory for data. Make sure it can be used.
 * @param len The size of data.
 * @return CMemFile struct if succeessed otherwise NULL.
*/
CMemFile* new_cmemfile(const char* data, size_t len);
void free_memfile(MemFile* f);
void free_cmemfile(CMemFile* f);
size_t memfile_read(MemFile* f, char* buf, size_t buf_len);
size_t cmemfile_read(CMemFile* f, size_t buf_len, char* buf);
int memfile_seek(MemFile* f, int64_t offset, int origin);
int cmemfile_seek(CMemFile* f, int64_t offset, int origin);
int64_t memfile_tell(MemFile* f);
int64_t cmemfile_tell(CMemFile* f);
int memfile_readpacket(void* f, uint8_t* buf, int buf_size);
size_t cmemfile_read2(void* f, size_t buf_len, char* buf);
int cmemfile_seek2(void* f, int64_t offset, int origin);
int64_t cmemfile_tell2(void* f);
#ifdef __cplusplus
}
#endif
#endif
