#ifndef _UTIL_FILEOP_H
#define _UTIL_FILEOP_H
#include <string>

namespace fileop {
    /**
     * @brief Check file exists
     * @param fn File name
     * @returns Whether file exists or not
    */
    bool exists(std::string fn);
    /**
     * @brief Remove file
     * @param fn File name
     * @param print_error Print error message
     * @returns true if successed
    */
    bool remove(std::string fn, bool print_error = false);
    /**
     * @brief Get directory name from path
     * @param fn Path
     * @returns Directory. If path does not contain a slash, will return "".
    */
    std::string dirname(std::string fn);
    /**
     * @brief Detect if path is a url.
     * @param fn Path
     * @return true if is a url.
    */
    bool is_url(std::string fn);
    /**
     * @brief Get file name from path
     * @param fn Path
     * @return File name.
    */
    std::string basename(std::string fn);
    /**
     * @brief Parse size string
     * @param size size string. Such as "10KiB", "10", "34B", "48K"
     * @param fs size
     * @param is_byte Whether to return bits or bytes
     * @return true if successed.
    */
    bool parse_size(std::string size, size_t& fs, bool is_byte = true);
    /**
     * @brief Opens a file.
     * @param fn File name.
     * @param fd file descriptor. -1 if error occured.
     * @param oflag The kind of operations allowed.
     * @param shflag The kind of sharing allowed. (Windows Only)
     * @param pmode Permission mode. (Windows Only)
     * @return errno
    */
    int open(std::string fn, int& fd, int oflag, int shflag = 0x10, int pmode = 0);
}
#endif
