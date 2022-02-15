#ifndef _UTILS_ERR_H
#define _UTILS_ERR_H
#if __cplusplus
#include <string>

namespace err {
    /**
     * @brief Get error message from errno
     * @param out Output string
     * @param errnum errno
     * @returns true if successed
    */
    bool get_errno_message(std::string &out, int errnum);
}
extern "C" {
#endif
/**
 * @brief Get error message from errno
 * @param errnum errno
 * @return Output string. Need free memory by calling free.
*/
char* err_get_errno_message(int errnum);
#if __cplusplus
}
#endif
#endif
