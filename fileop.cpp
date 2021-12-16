#if HAVE_UTILS_CONFIG_H
#include "utils_config.h"
#endif

#include "fileop.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <fileapi.h>
#else
#include <unistd.h>
#include <utime.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include "err.h"
#include "wchar_util.h"
#include "time_util.h"
#include <regex>

#ifdef _WIN32
#if HAVE__ACCESS_S
#define access _access_s
#endif
#if HAVE__WACCESS_S
#define _waccess _waccess_s
#endif
#if HAVE_PRINTF_S
#define printf printf_s
#endif
#if HAVE_SSCANF_S
#define sscanf sscanf_s
#endif
#endif

#ifdef _WIN32
bool exists_internal(wchar_t* fn) {
    return !_waccess(fn, 0);
}

bool remove_internal(wchar_t* fn, bool print_error) {
    int ret = _wremove(fn);
    if (ret && print_error && errno != ENOENT) {
        std::string o;
        std::string tfn;
        if (err::get_errno_message(o, errno) && wchar_util::wstr_to_str(tfn, fn, CP_UTF8)) {
            printf("Can not remove file \"%s\": %s.\n", tfn.c_str(), o.c_str());
        }
    }
    return !ret;
}

int open_internal(wchar_t* fn, int* fd, int oflag, int shflag, int pmode) {
    return _wsopen_s(fd, fn, oflag, shflag, pmode);
}

bool isdir_internal(wchar_t* fn, bool& result) {
    struct __stat64 stats;
    if (_wstat64(fn, &stats)) {
        return false;
    }
    result = stats.st_mode & S_IFDIR;
    return true;
}

bool mkdir_internal(wchar_t* fn) {
    return !_wmkdir(fn);
}

HANDLE set_file_time_internal(wchar_t* fn) {
    return CreateFileW(fn, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
}

template <typename T, typename ... Args>
T fileop_internal(const char* fname, UINT codePage, T(*callback)(wchar_t* fn, Args... args), T failed, Args... args) {
    int wlen;
    wchar_t* fn;
    DWORD opt = wchar_util::getMultiByteToWideCharOptions(MB_ERR_INVALID_CHARS, codePage);
    wlen = MultiByteToWideChar(codePage, opt, fname, -1, nullptr, 0);
    if (!wlen) return failed;
    fn = (wchar_t*)malloc(sizeof(wchar_t) * wlen);
    if (!MultiByteToWideChar(codePage, opt, fname, -1, fn, wlen)) {
        free(fn);
        return failed;
    }
    T re = callback(fn, args...);
    free(fn);
    return re;
}
#endif

bool fileop::exists(std::string fn) {
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    for (i = 0; i < 3; i++) {
        if (fileop_internal(fn.c_str(), cp[i], &exists_internal, false)) return true;
    }
    return !access(fn.c_str(), 0);
#else
    return !access(fn.c_str(), 0);
#endif
}

bool fileop::remove(std::string fn, bool print_error) {
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    for (i = 0; i < 3; i++) {
        if (fileop_internal(fn.c_str(), cp[i], &remove_internal, false, print_error)) return true;
    }
#endif
    int ret = ::remove(fn.c_str());
    if (ret && print_error) {
        std::string o;
        if (err::get_errno_message(o, errno)) {
            printf("Can not remove file \"%s\": %s.\n", fn.c_str(), o.c_str());
        }
    }
    return !ret;
}

std::string fileop::dirname(std::string fn) {
    auto i = fn.find_last_of('/');
    auto i2 = fn.find_last_of('\\');
    i = (i == std::string::npos || (i2 != std::string::npos && i2 > i)) ? i2 : i;
    return i == std::string::npos ? "" : fn.substr(0, i);
}

bool fileop::is_url(std::string fn) {
    return fn.find("://") != std::string::npos;
}

std::string fileop::basename(std::string fn) {
    if (!is_url(fn)) {
        auto i = fn.find_last_of('/');
        auto i2 = fn.find_last_of('\\');
        i = (i == std::string::npos || (i2 != std::string::npos && i2 > i)) ? i2 : i;
        return i == std::string::npos ? fn : fn.substr(i + 1, fn.length() - i - 1);
    } else {
        auto iq = fn.find_first_of('?');
        auto iq2 = fn.find_first_of('#');
        iq = (iq == std::string::npos || (iq2 != std::string::npos && iq2 < iq)) ? iq2 : iq;
        auto i = fn.find_last_of('/', iq);
        auto i2 = fn.find_last_of('\\', iq);
        i = (i == std::string::npos || (i2 != std::string::npos && i2 > i)) ? i2 : i;
        if (i == std::string::npos && iq == std::string::npos) {
            return fn;
        } else if (i == std::string::npos) {
            return fn.substr(0, iq);
        } else if (iq == std::string::npos) {
            return fn.substr(i + 1, fn.length() - i - 1);
        } else {
            return fn.substr(i + 1, iq - i - 1);
        }
    }
}

bool fileop::parse_size(std::string size, size_t& fs, bool is_byte) {
    const std::regex REG(R"(^(\d+)([kmgtpezy])?(i)?(b)?$)", std::regex::icase);
    std::smatch m;
    if (!std::regex_search(size, m, REG)) {
        return false;
    }
    auto bs = m[1].str();
    size_t base;
    if (sscanf(bs.c_str(), "%zu", &base) != 1) {
        return false;
    }
    size_t pow = 0;
    if (m[2].matched) {
        auto sp = std::tolower(m[2].str()[0]);
        if (sp == 'k') pow = 1;
        else if (sp == 'm') pow = 2;
        else if (sp == 'g') pow = 3;
        else if (sp == 't') pow = 4;
        else if (sp == 'p') pow = 5;
        else if (sp == 'e') pow = 6;
        else if (sp == 'z') pow = 7;
        else if (sp == 'y') pow = 8;
    }
    if (m[3].matched) {
        pow = 1ull << (pow * 10);
    } else {
        size_t tmp = 1;
        while (pow > 0) {
            tmp *= 1000ull;
            pow--;
        }
        pow = tmp;
    }
    if (m[4].matched) {
        auto b = m[4].str();
        if (b == "b") {
            fs = is_byte ? base * pow / 8ull : base * pow;
        } else {
            fs = is_byte ? base * pow : base * pow * 8ull;
        }
    } else {
        fs = base * pow;
    }
    return true;
}

int fileop::open(std::string fn, int& fd, int oflag, int shflag, int pmode) {
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    int tmfd;
    int err;
    for (i = 0; i < 3; i++) {
        if (!fileop_internal(fn.c_str(), cp[i], &open_internal, EINVAL, &tmfd, oflag, shflag, pmode)) {
            fd = tmfd;
            return 0;
        }
    }
    err = ::_sopen_s(&tmfd, fn.c_str(), oflag, shflag, pmode);
    fd = tmfd;
    return fd == -1 ? err : 0;
#else
    fd = ::open(fn.c_str(), oflag);
    return fd == -1 ? errno : 0;
#endif
}

bool fileop::isabs(std::string path) {
    if (!path.length()) return false;
#if _WIN32
    if (path.length() <= 2) return false;
    if (isalpha(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) return true; else return false;
#else
    return path[0] == '/' ? true : false;
#endif
}

std::string fileop::join(std::string path, std::string path2) {
    auto l1 = path.length(), l2 = path2.length();
    if (!l1) return path2;
    if (!l2) return path;
    if (isabs(path2)) return path2;
#if _WIN32
    if (l2 >= 2 && isalpha(path2[0]) && path2[1] == ':') return path2;
    if (l1 >= 2 && isalpha(path[0]) && path[1] == ':') {
        if (path2[0] == '/' || path2[0] == '\\') return path.substr(0, 2) + path2;
        return (path[l1 - 1] == '/' || path[l1 - 1] == '\\') ? path + path2 : path + "\\" + path2;
    }
    if (path2[0] == '/' || path2[0] == '\\') return path2;
    return (path[l1 - 1] == '/' || path[l1 - 1] == '\\') ? path + path2 : path + "\\" + path2;
#else
    return path[l1 - 1] == '/' ? path + path2 : path + "/" + path2;
#endif
}

bool fileop::isdir(std::string path, bool& result) {
    if (!exists(path)) {
        result = false;
        return true;
    }
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    for (i = 0; i < 3; i++) {
        if (fileop_internal<bool, bool&>(path.c_str(), cp[i], &isdir_internal, false, result)) return true;
    }
    struct __stat64 stats;
    if (_stat64(path.c_str(), &stats)) {
#else
    struct stat stats;
    if (stat(path.c_str(), &stats)) {
#endif
        return false;
    }
    result = stats.st_mode & S_IFDIR;
    return true;
}

#if _WIN32
bool fileop::isdrive(std::string path) {
    auto l = path.length();
    if (l == 2 && isalpha(path[0]) && path[1] == ':') return true;
    if (l == 3 && isalpha(path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\')) return true;
    return false;
}
#endif

bool fileop::mkdir(std::string path, int mode) {
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    for (i = 0; i < 3; i++) {
        if (fileop_internal(path.c_str(), cp[i], &mkdir_internal, false)) return true;
    }
    return !::_mkdir(path.c_str());
#else
    return !::mkdir(path.c_str(), mode);
#endif
}

bool fileop::set_file_time(std::string path, time_t ctime, time_t actime, time_t modtime) {
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    HANDLE file;
    for (i = 0; i < 3; i++) {
        if ((file = fileop_internal(path.c_str(), cp[i], &set_file_time_internal, INVALID_HANDLE_VALUE)) != INVALID_HANDLE_VALUE) {
            break;
        }
    }
    if (file == INVALID_HANDLE_VALUE) {
        file = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
    }
    FILETIME c, ac, mod;
    time_util::time_t_to_file_time(ctime, &c);
    time_util::time_t_to_file_time(actime, &ac);
    time_util::time_t_to_file_time(modtime, &mod);
    auto re = SetFileTime(file, &c, &ac, &mod);
    CloseHandle(file);
    return re ? true : false;
#else
    struct utimbuf t = { actime, modtime };
    return !utime(path.c_str(), &t);
#endif
}

FILE* fileop::fdopen(int fd, std::string mode) {
#if _WIN32
    return ::_fdopen(fd, mode.c_str());
#else
    return ::fdopen(fd, mode.c_str());
#endif
}

bool fileop::close(int fd) {
#if _WIN32
    return !::_close(fd);
#else
    return !::close(fd);
#endif
}

bool fileop::fclose(FILE* f) {
    if (!f) return false;
    return !::fclose(f);
}
