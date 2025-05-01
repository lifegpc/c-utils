#ifndef _UTIL_REG_UTIL_H
#define _UTIL_REG_UTIL_H
#ifdef _WIN32
#include <windows.h>
#include <string>
namespace reg_util {
    /**
     * @brief Get string(REG_SZ) value from registry.
     * @param result Result string
     * @param hKey Registry key handle
     * @param subKey Subkey path
     * @param value Value name
     * @return true if success, false otherwise.
     */
    bool get_string(std::wstring& result, HKEY hKey, std::wstring subKey, std::wstring value);
    /**
     * @brief Get DWORD(REG_DWORD) value from registry.
     * @param result Result DWORD
     * @param hKey Registry key handle
     * @param subKey Subkey path
     * @param value Value name
     * @return true if success, false otherwise.
     */
    bool get_dword(DWORD& result, HKEY hKey, std::wstring subKey, std::wstring value);
}
#endif
#endif
