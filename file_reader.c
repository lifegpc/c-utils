#include "file_reader.h"

#include <malloc.h>
#include <stdio.h>
#include "cfileop.h"
#include "cstr_util.h"

typedef struct file_reader_file {
    void* f;
    file_reader_file_read read;
    file_reader_file_seek seek;
    file_reader_file_tell tell;
    /// 0 if little endian otherwise big endian
    unsigned char endian;
} file_reader_file;

size_t file_reader_default_read(void* f, size_t buf_len, char* buf) {
    return fread(buf, 1, buf_len, (FILE*)f);
}

int file_reader_default_seek(void* f, int64_t offset, int origin) {
    return fileop_fseek((FILE*)f, offset, origin);
}

int64_t file_reader_default_tell(void* f) {
    return fileop_ftell((FILE*)f);
}

file_reader_file* create_file_reader(FILE* f, unsigned char endian) {
    if (!f) return NULL;
    file_reader_file* r = malloc(sizeof(file_reader_file));
    if (!r) return NULL;
    r->f = (void*)f;
    r->read = &file_reader_default_read;
    r->seek = &file_reader_default_seek;
    r->tell = &file_reader_default_tell;
    r->endian = endian;
    return r;
}

file_reader_file* create_file_reader2(void* f, file_reader_file_read read, file_reader_file_seek seek, file_reader_file_tell tell, unsigned char endian) {
    if (!f || !read || !seek | !tell) return NULL;
    file_reader_file* r = malloc(sizeof(file_reader_file));
    if (!r) return NULL;
    r->f = f;
    r->read = read;
    r->seek = seek;
    r->tell = tell;
    r->endian = endian;
    return r;
}

void free_file_reader(file_reader_file* f) {
    if (!f) return;
    free(f);
}

void set_file_reader_endian(file_reader_file* f, unsigned char endian) {
    if (!f) return;
    f->endian = endian;
}

int file_reader_read_char(file_reader_file* f, char* re) {
    if (!f) return 1;
    char buf[1];
    if (!f->read(f->f, 1, buf)) {
        return 1;
    }
    if (re) *re = buf[0];
    return 0;
}

int file_reader_read_uint8(file_reader_file* f, uint8_t* re) {
    return file_reader_read_char(f, (char*)re);
}

int file_reader_read_int16(file_reader_file* f, int16_t* re) {
    if (!f) return 1;
    int64_t offset = f->tell(f->f);
    int16_t r = 0;
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    size_t c;
    uint8_t buf[2];
    if ((c = f->read(f->f, 2, buf)) < 2) {
        if (origin == SEEK_CUR) offset = -c;
        f->seek(f->f, offset, origin);
        return 1;
    }
    r = cstr_read_int16(buf, f->endian);
    if (re) *re = r;
    return 0;
}

int file_reader_read_int32(file_reader_file* f, int32_t* re) {
    if (!f) return 1;
    int64_t offset = f->tell(f->f);
    int32_t r = 0;
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    size_t c;
    uint8_t buf[4];
    if ((c = f->read(f->f, 4, buf)) < 4) {
        if (origin == SEEK_CUR) offset = -c;
        f->seek(f->f, offset, origin);
        return 1;
    }
    r = cstr_read_int32(buf, f->endian);
    if (re) *re = r;
    return 0;
}

int file_reader_read_uint32(file_reader_file* f, uint32_t* re) {
    return file_reader_read_int32(f, (int32_t*)re);
}

int file_reader_read_int64(file_reader_file* f, int64_t* re) {
    if (!f) return 1;
    int64_t offset = f->tell(f->f), r = 0;
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    size_t c;
    uint8_t buf[8];
    if ((c = f->read(f->f, 8, buf)) < 8) {
        if (origin == SEEK_CUR) offset = -c;
        f->seek(f->f, offset, origin);
        return 1;
    }
    r = cstr_read_int64(buf, f->endian);
    if (re) *re = r;
    return 0;
}

int file_reader_read_str(file_reader_file* f, char** buf) {
    if (!f) return 1;
    char* b = NULL;
    char bu[128];
    size_t blen = 128, n = 0, c = 0, tc = 0, pos = 0;
    if (buf) {
        b = malloc(blen);
        if (!b) return 1;
    }
    int64_t offset = f->tell(f->f);
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    while (1) {
        if (n >= c) {
            if (!(tc = f->read(f->f, 128, bu))) {
                if (b) free(b);
                if (origin == SEEK_CUR) {
                    offset = -c;
                }
                f->seek(f->f, offset, origin);
                return 1;
            }
            c += tc;
        }
        if (buf && n >= blen) {
            size_t nlen = blen + 128;
            char* nb = realloc(b, nlen);
            if (!nb) {
                if (b) free(b);
                if (origin == SEEK_CUR) {
                    offset = -c;
                }
                f->seek(f->f, offset, origin);
                return 1;
            }
            b = nb;
            blen = nlen;
        }
        pos = n - (c - tc);
        if (buf && b) {
            b[n] = bu[pos];
        }
        if (bu[pos] == 0) {
            break;
        }
        n++;
    }
    if (buf) {
        char* nb = realloc(b, n + 1);
        *buf = nb ? nb : b;
    }
    if (origin == SEEK_SET) {
        offset += n + 1;
    } else {
        offset = -(c - n - 1);
    }
    f->seek(f->f, offset, origin);
    return 0;
}
