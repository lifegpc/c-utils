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
#include <dirent.h>
#include <unistd.h>
#include <utime.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include "err.h"
#include "str_util.h"
#include "wchar_util.h"
#include "time_util.h"
#include <regex>
#include <list>

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
#if __MINGW32__
    struct _stat stats;
    if (_wstat(fn, &stats)) {
#else
    struct __stat64 stats;
    if (_wstat64(fn, &stats)) {
#endif
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

HANDLE get_file_size_internal(wchar_t* fn) {
    return CreateFileW(fn, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
}

bool listdir_internal(wchar_t* fn, std::list<std::string>& list, bool ignore_hidden_file) {
    std::list<std::string> li;
    WIN32_FIND_DATAW data;
    HANDLE re = FindFirstFileW(fn, &data);
    if (re == INVALID_HANDLE_VALUE) return false;
    std::string tfn;
    do {
        if (!wchar_util::wstr_to_str(tfn, data.cFileName, CP_UTF8)) {
            FindClose(re);
            return false;
        }
        if ((ignore_hidden_file && tfn.find(".") != 0) || (!ignore_hidden_file && tfn != "." && tfn != "..")) li.push_back(tfn);
        BOOL r = FindNextFileW(re, &data);
        if (!r) {
            FindClose(re);
            if (GetLastError() == ERROR_NO_MORE_FILES) break;
            return false;
        }
    } while (1);
    list = li;
    return true;
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
    fd = ::open(fn.c_str(), oflag, pmode);
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

bool fileop::mkdirs(std::string path, int mode, bool allow_exists) {
    bool exists;
    if (!isdir(path, exists)) return false;
    if (exists) return allow_exists ? true : false;
    std::string dn;
#if _WIN32
    std::string sp = "\\";
#else
    std::string sp = "/";
#endif
    dn = path;
    if (!dn.length() || dn == ".") dn = "." + sp;
    if (!isdir(dn, exists)) return false;
    if (exists) return allow_exists ? true : false;
    std::list<std::string> li;
    li.push_back(dn);
    do {
        dn = dirname(dn);
        if (dn.length() == 0) {
            if (li.size() > 0) {
                auto en = *(li.rbegin());
                if (en == ("." + sp)) return false;
            }
            dn = ".";
        }
        if (!isdir(dn + sp, exists)) return false;
        if (!exists) li.push_back(dn + sp);
    } while (!exists);
    auto it = li.rbegin();
    for (; it != li.rend(); it++) {
        if (!mkdir(*it, mode)) return false;
    }
    return true;
}

bool fileop::get_file_size(std::string path, size_t& size) {
    if (!exists(path)) return false;
#if _WIN32
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    HANDLE file;
    for (i = 0; i < 3; i++) {
        if ((file = fileop_internal(path.c_str(), cp[i], &get_file_size_internal, INVALID_HANDLE_VALUE)) != INVALID_HANDLE_VALUE) {
            break;
        }
    }
    if (file == INVALID_HANDLE_VALUE) {
        file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
    }
    LARGE_INTEGER si;
    auto re = GetFileSizeEx(file, &si);
    if (re) size = si.QuadPart;
    CloseHandle(file);
    return re ? true : false;
#else
    struct stat stats;
    if (stat(path.c_str(), &stats)) {
        return false;
    }
    size = stats.st_size;
    return true;
#endif
}

int fileop::fseek(FILE* f, int64_t offset, int origin) {
#if _WIN32
    return ::_fseeki64(f, offset, origin);
#else
#if HAVE_FSEEKO64
    return ::fseeko64(f, offset, origin);
#elif HAVE_FSEEKO
    return ::fseeko(f, offset, origin);
#else
    return ::fseek(f, offset, origin);
#endif
#endif
}

bool fileop::mkdir_for_file(std::string path, int mode) {
    return mkdirs(dirname(path), mode, true);
}

int64_t fileop::ftell(FILE* f) {
#if _WIN32
    return ::_ftelli64(f);
#else
#if HAVE_FTELLO64
    return ::ftello64(f);
#elif HAVE_FTELLO
    return ::ftello(f);
#else
    return ::ftell(f);
#endif
#endif
}

bool fileop::listdir(std::string path, std::list<std::string>& filelist, bool ignore_hidden_file) {
#if _WIN32
    if (!path.length()) path = ".";
    path = str_util::str_replace(path, "/", "\\");
    path = join(path, "*");
    UINT cp[] = { CP_UTF8, CP_OEMCP, CP_ACP };
    int i;
    std::list<std::string> li;
    for (i = 0; i < 3; i++) {
        if (fileop_internal<bool, std::list<std::string>&, bool>(path.c_str(), cp[i], &listdir_internal, false, li, ignore_hidden_file)) {
            filelist = li;
            return true;
        }
    }
    WIN32_FIND_DATAA data;
    HANDLE re = FindFirstFileA(path.c_str(), &data);
    if (re == INVALID_HANDLE_VALUE) return false;
    std::string tfn;
    do {
        tfn = data.cFileName;
        if ((ignore_hidden_file && tfn.find(".") != 0) || (!ignore_hidden_file && tfn != "." && tfn != "..")) li.push_back(tfn);
        BOOL r = FindNextFileA(re, &data);
        if (!r) {
            FindClose(re);
            if (GetLastError() == ERROR_NO_MORE_FILES) break;
            return false;
        }
    } while (1);
    filelist = li;
    return true;
#else
    DIR* dir = opendir(path.c_str());
    if (!dir) return false;
    struct dirent* d = readdir(dir);
    std::list<std::string> li;
    std::string tfn;
    while (d) {
        tfn = d->d_name;
        if ((ignore_hidden_file && tfn.find(".") != 0) || (!ignore_hidden_file && tfn != "." && tfn != "..")) li.push_back(tfn);
        d = readdir(dir);
    }
    if (errno) {
        closedir(dir);
        return false;
    }
    closedir(dir);
    filelist = li;
    return true;
#endif
}

std::string fileop::filename(std::string path) {
    auto loc = path.find_last_of('.');
    return loc == -1 ? path : path.substr(0, loc);
}

int fileop::fcloseall() {
#if _WIN32
    return ::_fcloseall();
#else
    return ::fcloseall();
#endif
}
