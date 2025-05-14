#include "memfile.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef min
#define min(x, y) (((x) < (y)) ? (x) : (y))
#endif

MemFile* new_memfile(const char* data, size_t len) {
    if (!data || !len) return NULL;
    MemFile* f = malloc(sizeof(MemFile));
    if (!f) return NULL;
    memset(f, 0, sizeof(MemFile));
    f->data = malloc(len);
    if (!f->data) {
        free(f);
        return NULL;
    }
    memcpy(f->data, data, len);
    f->len = len;
    f->loc = 0;
    return f;
}

CMemFile* new_cmemfile(const char* data, size_t len) {
    if (!data || !len) return NULL;
    CMemFile* f = malloc(sizeof(CMemFile));
    if (!f) return NULL;
    f->data = data;
    f->len = len;
    f->loc = 0;
    return f;
}

void free_memfile(MemFile* f) {
    if (!f) return;
    if (f->data) {
        free(f->data);
    }
    free(f);
}

void free_cmemfile(CMemFile* f) {
    if (!f) return;
    free(f);
}

size_t memfile_read(MemFile* f, char* buf, size_t buf_len) {
    if (!f || !buf) return (size_t)-1;
    if (!buf_len || f->loc >= f->len) return 0;
    size_t le = min(buf_len, f->len - f->loc);
    memcpy(buf, f->data + f->loc, le);
    f->loc += le;
    return le;
}

size_t cmemfile_read(CMemFile* f, size_t buf_len, char* buf) {
    if (!f || !buf) return 0;
    if (!buf_len || f->loc >= f->len) return 0;
    size_t le = min(buf_len, f->len - f->loc);
    memcpy(buf, f->data + f->loc, le);
    f->loc += le;
    return le;
}

int memfile_seek(MemFile* f, int64_t offset, int origin) {
    if (!f) return 1;
    int64_t npos = 0;
    if (origin == SEEK_SET) {
        npos = offset;
    } else if (origin == SEEK_CUR) {
        npos = f->loc + offset;
    } else if (origin == SEEK_END) {
        npos = f->len + offset;
    } else {
        return 1;
    }
    if (npos < 0 || npos > f->len) return 1;
    f->loc = npos;
    return 0;
}

int cmemfile_seek(CMemFile* f, int64_t offset, int origin) {
    if (!f) return 1;
    int64_t npos = 0;
    if (origin == SEEK_SET) {
        npos = offset;
    } else if (origin == SEEK_CUR) {
        npos = f->loc + offset;
    } else if (origin == SEEK_END) {
        npos = f->len + offset;
    } else {
        return 1;
    }
    if (npos < 0 || npos > f->len) return 1;
    f->loc = npos;
    return 0;
}

int64_t cmemfile_tell(CMemFile* f) {
    if (!f) return -1;
    return f->loc;
}

int64_t memfile_tell(MemFile* f) {
    if (!f) return -1;
    return f->loc;
}

#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define FFERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))
#define AVERROR_EOF FFERRTAG( 'E','O','F',' ')
#define AVERROR(e) (-(e))

int memfile_readpacket(void* f, uint8_t* buf, int buf_size) {
    size_t ret = memfile_read((MemFile*)f, (char*)buf, buf_size);
    if (ret == 0) return AVERROR(EINVAL);
    else if (ret == (size_t)-1) return -1;
    else return ret;
}

size_t cmemfile_read2(void* f, size_t buf_len, char* buf) {
    return cmemfile_read((CMemFile*)f, buf_len, buf);
}

int cmemfile_seek2(void* f, int64_t offset, int origin) {
    return cmemfile_seek((CMemFile*)f, offset, origin);
}

int64_t cmemfile_tell2(void* f) {
    return cmemfile_tell((CMemFile*)f);
}
