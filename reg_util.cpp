#include "reg_util.h"
#ifdef _WIN32
bool reg_util::get_string(std::wstring& result, HKEY hKey, std::wstring subKey, std::wstring value) {
    DWORD len;
    if (RegGetValueW(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &len) != ERROR_SUCCESS) {
        return false;
    }
    wchar_t* buffer = new wchar_t[len / sizeof(wchar_t)];
    if (RegGetValueW(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, buffer, &len) != ERROR_SUCCESS) {
        delete[] buffer;
        return false;
    }
    result = std::wstring(buffer);
    delete[] buffer;
    return true;
}

bool reg_util::get_dword(DWORD& result, HKEY hKey, std::wstring subKey, std::wstring value) {
    DWORD len = sizeof(DWORD);
    if (RegGetValueW(hKey, subKey.c_str(), value.c_str(), RRF_RT_REG_DWORD, nullptr, &result, &len) != ERROR_SUCCESS) {
        return false;
    }
    return true;
}
#endif
