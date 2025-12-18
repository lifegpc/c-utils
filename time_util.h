#ifndef _UTIL_TIME_UTIL_H
#define _UTIL_TIME_UTIL_H
#include <time.h>
#if _WIN32
#include <Windows.h>
#endif
#include <stdint.h>

#if __cplusplus

namespace time_util {
#if _WIN32
    /**
     * @brief Convert from time_t to FILETIME
     * @param t UNIX Timestamp
     * @param pft Result
    */
    void time_t_to_file_time(time_t t, LPFILETIME pft);
#endif
    char* strptime(const char* s, const char* format, struct tm* tm);
    long get_timezone();
    time_t timegm(struct tm* tm);
    size_t time_ns();
    int64_t time_ns64();
    int mssleep(unsigned int ms);
}

extern "C" {
#endif

size_t time_time_ns();
int mssleep(unsigned int ms);

#if __cplusplus
}
#endif

#endif
