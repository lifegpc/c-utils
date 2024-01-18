#ifndef STRPTIME_H
#define STRPTIME_H

#if __cplusplus
extern "C" {
#endif

#include <time.h>

#ifdef _WIN32
char* strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

#if __cplusplus
}
#endif

#endif // STRPTIME_H
