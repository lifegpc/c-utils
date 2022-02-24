#ifndef _UTILS_ERR_H
#define _UTILS_ERR_H
#if __cplusplus
extern "C" {
#include <stdint.h>
}
#include <string>

namespace err {
    /**
     * @brief Get error message from errno
     * @param out Output string
     * @param errnum errno
     * @returns true if successed
    */
    bool get_errno_message(std::string &out, int errnum);
#if _WIN32
    /**
     * @brief Get error message for HRESULT
     * @param out Output string
     * @param errnum HRESULT
     * @return true if successed
    */
    bool get_winerror(std::string &out, int32_t errnum);
#endif
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
