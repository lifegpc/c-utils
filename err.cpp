#if HAVE_UTILS_CONFIG_H
#include "utils_config.h"
#endif

#include "err.h"
#include "wchar_util.h"
#include <string.h>
#if _WIN32
#include "Windows.h"
#endif
#include "cpp2c.h"

bool err::get_errno_message(std::string &out, int errnum) {
#if _WIN32
#if HAVE__WCSERROR_S
    wchar_t buf[512];
    if(!_wcserror_s(buf, 512, errnum)) {
        std::wstring ws(buf);
        if(wchar_util::wstr_to_str(out, ws, CP_UTF8)) return true;
    }
#else
    wchar_t* buf = _wcserror(errnum);
    if (buf != nullptr) {
        std::wstring ws(buf);
        if(wchar_util::wstr_to_str(out, ws, CP_UTF8)) return true;
    }
#endif
#if HAVE_STRERROR_S
    char nbuf[1024];
    if (!strerror_s(nbuf, 1024, errnum)) {
        out = nbuf;
        return true;
    }
#else
    char* nbuf = strerror(errnum);
    if (nbuf != nullptr) {
        out = nbuf;
        return true;
    }
#endif
    return false;
#else
#if HAVE_GNU_STRERROR_R
    char buf[1024];
    char* re = strerror_r(errnum, buf, 1024);
    out = re;
    return true;
#else
#if HAVE_STRERROR_R
    char buf[1024];
    if (!strerror_r(errnum, buf, 1024)) {
        out = buf;
        return true;
    }
#endif
    char *ret = strerror(errnum);
    if (ret != nullptr) {
        out = ret;
    }
    return ret != nullptr;
#endif 
#endif
}

char* err_get_errno_message(int errnum) {
    std::string msg;
    if (!err::get_errno_message(msg, errnum)) return nullptr;
    char* tmp;
    if (!cpp2c::string2char(msg, tmp)) return nullptr;
    return tmp;
}
