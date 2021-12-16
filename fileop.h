#ifndef _UTIL_FILEOP_H
#define _UTIL_FILEOP_H
#include <string>
#include <time.h>
#include <stdio.h>

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
    /**
     * @brief Check if path is absoulte path.
     * @param path The path.
     * @return 
    */
    bool isabs(std::string path);
    /**
     * @brief Join two more pathname components
     * @param path 
     * @param path2 
     * @return result
    */
    std::string join(std::string path, std::string path2);
    /**
     * @brief check if path is an existing directory.
     * @param path Path
     * @param result true if path is an existing directory.
     * @return false if error occured
    */
    bool isdir(std::string path, bool& result);
#if _WIN32
    /**
     * @brief Check if a path only contains drive.
     * @param path Path
     * @return Result
    */
    bool isdrive(std::string path);
#endif
    /**
     * @brief Creates a new directory.
     * @param path Path for a new directory.
     * @param mode Directory permission. (Linux only)
     * @return true if successed.
    */
    bool mkdir(std::string path, int mode);
    /**
     * @brief Sets the date and time that the specified file or directory was created, last accessed, or last modified.
     * @param path The path of directory or file
     * @param ctime creation date (Windows Only)
     * @param actime Last access date
     * @param modtime Last modification date
     * @return true if successed.
    */
    bool set_file_time(std::string path, time_t ctime, time_t actime, time_t modtime);
    /**
     * @brief Associates a stream with a file that was previously opened.
     * @param fd File descriptor of the open file.
     * @param mode Type of file access.
     * @return A pointer to the open stream.
    */
    FILE* fdopen(int fd, std::string mode);
    /**
     * @brief Closes a file.
     * @param fd File descriptor referring to the open file.
     * @return 
    */
    bool close(int fd);
    /**
     * @brief Closes a stream
     * @param f Pointer to FILE structure.
     * @return true if the stream is successfully closed
    */
    bool fclose(FILE* f);
}
#endif
