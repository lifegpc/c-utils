#include "time_util.h"
#include "utils_config.h"

#if _WIN32
#include <Windows.h>
#endif

#include <stdio.h>

#if !HAVE_STRPTIME
#include "strptime/strptime.h"
#endif

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#include "err.h"

#if HAVE_USLEEP
#include <unistd.h>
#endif

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
#if HAVE_TIMEZONE && HAVE_TZSET
    tzset();
    return timezone;
#elif HAVE__GET_TIMEZONE
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

size_t time_util::time_ns() {
#if _WIN32
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    size_t t = ((size_t)ft.dwHighDateTime << 32) | (size_t)ft.dwLowDateTime;
    return t;
#elif HAVE_CLOCK_GETTIME
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
#else
    return time(NULL) * 1000000000LL;
#endif
}

size_t time_time_ns() {
    return time_util::time_ns();
}

int time_util::mssleep(unsigned int ms) {
#if _WIN32
    Sleep(ms);
    return 0;
#elif HAVE_USLEEP
    return usleep(ms);
#elif HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    return nanosleep(&ts, NULL);
#else
    printf("mssleep failed: not implemented\n");
    return -1;
#endif
}

int mssleep(unsigned int ms) {
    return time_util::mssleep(ms);
}
