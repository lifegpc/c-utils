#include "time_util.h"

#if _WIN32
#include <Windows.h>
#endif

#if _WIN32
void time_util::time_t_to_file_time(time_t t, LPFILETIME pft) {
    ULARGE_INTEGER time_value;
    time_value.QuadPart = (t * 10000000LL) + 116444736000000000LL;
    pft->dwLowDateTime = time_value.LowPart;
    pft->dwHighDateTime = time_value.HighPart;
}
#endif
