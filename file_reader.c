#include "file_reader.h"

#include <malloc.h>
#include <stdio.h>
#include "cfileop.h"
#include "cstr_util.h"

typedef struct file_reader_file {
    FILE* f;
    /// 0 if little endian otherwise big endian
    unsigned char endian;
} file_reader_file;

file_reader_file* create_file_reader(FILE* f, unsigned char endian) {
    if (!f) return NULL;
    file_reader_file* r = malloc(sizeof(file_reader_file));
    if (!r) return NULL;
    r->f = f;
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

int file_reader_read_int32(file_reader_file* f, int32_t* re) {
    if (!f) return 1;
    int64_t offset = fileop_ftell(f->f);
    int32_t r = 0;
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    size_t c;
    uint8_t buf[4];
    if ((c = fread(buf, 1, 4, f->f)) < 4) {
        if (origin == SEEK_CUR) offset = -c;
        fileop_fseek(f->f, offset, origin);
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
    int64_t offset = fileop_ftell(f->f), r = 0;
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    size_t c;
    uint8_t buf[8];
    if ((c = fread(buf, 1, 8, f->f)) < 8) {
        if (origin == SEEK_CUR) offset = -c;
        fileop_fseek(f->f, offset, origin);
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
    int64_t offset = fileop_ftell(f->f);
    int origin = SEEK_SET;
    if (offset == -1) {
        origin = SEEK_CUR;
    }
    while (1) {
        if (n >= c) {
            if (!(tc = fread(bu, 1, 128, f->f))) {
                if (b) free(b);
                if (origin == SEEK_CUR) {
                    offset = -c;
                }
                fileop_fseek(f->f, offset, origin);
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
                fileop_fseek(f->f, offset, origin);
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
    fileop_fseek(f->f, offset, origin);
    return 0;
}
