#ifndef _UTIL_TIME_UTIL_H
#define _UTIL_TIME_UTIL_H
#include <time.h>
#if _WIN32
#include <Windows.h>
#endif

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
}
#endif
