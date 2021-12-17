#ifndef _UTILS_ENCODING_H
#define _UTILS_ENCODING_H
#include <string>
#include "utils_config.h"
namespace encoding {
#if HAVE_ICONV
    /**
     * @brief Convert string from a encoding to another encoding
     * @param input input string
     * @param output output string
     * @param ori_enc origin encoding
     * @param des_enc target encoding
     * @return true if sccessed.
    */
    bool iconv_convert(std::string input, std::string& output, std::string ori_enc, std::string des_enc);
#endif
#if _WIN32
    /**
     * @brief Convert encoding name to code page
     * @param encoding Encoding name
     * @param cp Code page
     * @return
    */
    bool encodingToCp(std::string encoding, unsigned int& cp);
#endif
    /**
     * @brief Convert string from a encoding to another encoding
     * @param input input string
     * @param output output string
     * @param ori_enc origin encoding
     * @param des_enc target encoding
     * @return true if sccessed.
    */
    bool convert(std::string input, std::string& output, std::string ori_enc, std::string des_enc);
}
#endif
