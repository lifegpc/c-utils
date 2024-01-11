#include "time_util.h"
#include "utils_config.h"

#if _WIN32
#include <Windows.h>
#endif

#include <stdio.h>

#ifndef HAVE_STRPTIME
#include "strptime/strptime.h"
#endif

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#include "err.h"

#if _WIN32
void time_util::time_t_to_file_time(time_t t, LPFILETIME pft) {
    ULARGE_INTEGER time_value;
    time_value.QuadPart = (t * 10000000LL) + 116444736000000000LL;
    pft->dwLowDateTime = time_value.LowPart;
    pft->dwHighDateTime = time_value.HighPart;
}
#endif

char* time_util::strptime(const char* s, const char* format, struct tm* tm) {
    return ::strptime(s, format, tm);
}

long time_util::get_timezone() {
#if HAVE__GET_TIMEZONE
    long t = 0;
    int err = _get_timezone(&t);
    if (!err) {
        std::string msg ;
        if (!err::get_errno_message(msg, err)) {
            msg = "Unknown error";
        }
        printf("get_timezone failed: %s\n", msg.c_str());
        return 0;
    } else {
        return t;
    }
#elif HAVE_TIMEZONE && HAVE_TZSET
    tzset();
    return timezone;
#else
    printf("get_timezone failed: not implemented\n");
    return 0;
#endif
}

time_t time_util::timegm(struct tm* tm) {
#if HAVE__MKGMTIME
    return _mkgmtime(tm);
#elif HAVE_TIMEGM
    return ::timegm(tm);
#else
    time_t now = ::mktime(tm);
    return now + get_timezone();
#endif
}
