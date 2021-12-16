#include "cfileop.h"
#include "fileop.h"
#include "cpp2c.h"
#include <errno.h>

int fileop_exists(const char* fn) {
    if (!fn) return 0;
    return fileop::exists(fn) ? 1 : 0;
}

int fileop_remove(const char* fn) {
    if (!fn) return 0;
    return fileop::remove(fn) ? 1 : 0;
}

char* fileop_dirname(const char* fn) {
    if (!fn) return nullptr;
    auto re = fileop::dirname(fn);
    char* tmp = nullptr;
    return cpp2c::string2char(re, tmp) ? tmp : nullptr;
}

int fileop_is_url(const char* fn, int* re) {
    if (!fn || !re) return 0;
    *re = fileop::is_url(fn) ? 1 : 0;
    return 1;
}

char* fileop_basename(const char* fn) {
    if (!fn) return nullptr;
    auto re = fileop::basename(fn);
    char* tmp = nullptr;
    return cpp2c::string2char(re, tmp) ? tmp : nullptr;
}

int fileop_parse_size(const char* size, size_t* fs, int is_byte) {
    if (!size || !fs) return 0;
    size_t tmp;
    auto re = fileop::parse_size(size, tmp, is_byte);
    if (re) *fs = tmp;
    return re ? 1 : 0;
}

int fileop_open(const char* fn, int* fd, int oflag, int shflag, int pmode) {
    if (!fn || !fd) return EINVAL;
    int tfd;
    if (!shflag) shflag = 0x10;
    int re = fileop::open(fn, tfd, oflag, shflag, pmode);
    *fd = tfd;
    return re;
}

int fileop_isabs(const char* path) {
    if (!path) return 0;
    return fileop::isabs(path) ? 1 : 0;
}

char* fileop_join(const char* path, const char* path2) {
    if (!path || !path2) return nullptr;
    auto re = fileop::join(path, path2);
    char* tmp = nullptr;
    return cpp2c::string2char(re, tmp) ? tmp : nullptr;
}

int fileop_isdir(const char* path, int* result) {
    if (!path || !result) return 0;
    bool re;
    auto r = fileop::isdir(path, re);
    if (r) *result = re ? 1 : 0;
    return r ? 1 : 0;
}

int fileop_mkdir(const char* path, int mode) {
    if (!path) return 0;
    return fileop::mkdir(path, mode) ? 1 : 0;
}

int fileop_set_file_time(const char* path, time_t ctime, time_t actime, time_t modtime) {
    if (!path) return 0;
    return fileop::set_file_time(path, ctime, actime, modtime) ? 1 : 0;
}
