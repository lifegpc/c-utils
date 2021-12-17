#include "encoding.h"
#include <malloc.h>
#include "err.h"
#include <errno.h>
#if HAVE_ICONV
#include "iconv.h"
#endif
#include "str_util.h"
#include "wchar_util.h"
#include <stdio.h>
#include <regex>

#if _WIN32
#include <Windows.h>
#endif

#ifdef HAVE_SSCANF_S
#define sscanf sscanf_s
#endif

#if HAVE_ICONV
bool encoding::iconv_convert(std::string input, std::string& output, std::string ori_enc, std::string des_enc) {
    auto cd = iconv_open(des_enc.c_str(), ori_enc.c_str());
    if (cd == (iconv_t)-1) {
        return false;
    }
    char* buf = (char*)malloc(input.length());
    char* nbuf = buf;
    size_t buf_len = input.length();
    size_t buf_left = 0;
    std::string out;
    size_t avail_in = input.length();
    char* in = (char*)input.c_str();
    if (!buf) {
        iconv_close(cd);
        return false;
    }
    while (avail_in > 0) {
        buf_left = buf_len;
        nbuf = buf;
        // If libiconv is linked as a shared library on Windows. errno may always be 0.
        if (iconv(cd, &in, &avail_in, &nbuf, &buf_left) == -1 && errno != E2BIG) {
            free(buf);
            iconv_close(cd);
            return false;
        }
        out += std::string(buf, buf_len - buf_left);
    }
    free(buf);
    output = out;
    return true;
}
#endif

#if _WIN32
bool encoding::encodingToCp(std::string encoding, unsigned int& cp) {
    std::string enc;
    if (!str_util::tolowercase(encoding, enc)) return false;
#define ref(x) return cp = (x), true
    static const std::regex reg(R"(^(cp|x-cp|ibm|windows-|iso-8859-)(\d+)$)");
    std::smatch re;
    if (std::regex_match(enc, re, reg)) {
        auto typ = re[1];
        auto res = re[2];
        auto ts = res.str();
        auto cs = ts.c_str();
        if (cs) {
            unsigned int tcp;
            if (sscanf(cs, "%u", &tcp) == 1) {
                if (typ == "cp") {
                    switch (tcp)
                    {
                    case 1025:
                        ref(21025U);
                    default:
                        ref(tcp);
                    }
                } else if (typ == "x-cp") {
                    ref(tcp);
                } else if (typ == "ibm") {
                    switch (tcp)
                    {
                    case 273:
                    case 277:
                    case 278:
                    case 280:
                    case 284:
                    case 285:
                    case 290:
                    case 297:
                    case 420:
                    case 423:
                    case 424:
                    case 871:
                    case 880:
                    case 905:
                    case 924:
                        ref(tcp + 20000U);
                    default:
                        ref(tcp);
                    }
                } else if (typ == "windows-") {
                    ref(tcp);
                } else if (typ == "iso-8859-") {
                    ref(tcp + 28590U);
                }
            }
        }
    }
    if (enc == "asmo-708") ref(708U);
    if (enc == "dos-720") ref(720U);
    if (enc == "dos-862") ref(862U);
    if (enc == "gb2312") ref(936U);
    if (enc == "ks_c_5601-1987") ref(949U);
    if (enc == "big5") ref(950U);
    if (enc == "utf16" || enc == "utf-16" || enc == "utf-16le" || enc == "utf16le") ref(1200U);
    if (enc == "unicodefffe" || enc == "utf-16be" || enc == "utf16be") ref(1201U);
    if (enc == "johab") ref(1361U);
    if (enc == "macintosh" || enc == "macroman") ref(10000U);
    if (enc == "x-mac-japanese") ref(10001U);
    if (enc == "x-mac-chinesetrad") ref(10002U);
    if (enc == "x-mac-korean") ref(10003U);
    if (enc == "x-mac-arabic" || enc == "macarabic") ref(10004U);
    if (enc == "x-mac-hebrew" || enc == "machebrew") ref(10005U);
    if (enc == "x-mac-greek" || enc == "macgreek") ref(10006U);
    if (enc == "x-mac-cyrillic" || enc == "maccyrillic") ref(10007U);
    if (enc == "x-mac-chinesesimp") ref(10008U);
    if (enc == "x-mac-romanian" || enc == "macromania") ref(10010U);
    if (enc == "x-mac-ukrainian" || enc == "macukraine") ref(10017U);
    if (enc == "x-mac-thai" || enc == "macthai") ref(10021U);
    if (enc == "x-mac-ce") ref(10029U);
    if (enc == "x-mac-icelandic" || enc == "maciceland") ref(10079U);
    if (enc == "x-mac-turkish" || enc == "macturkish") ref(10081U);
    if (enc == "x-mac-croatian" || enc == "maccroatian") ref(10082U);
    if (enc == "utf32" || enc == "utf-32" || enc == "utf-32le" || enc == "utf32le") ref(12000U);
    if (enc == "utf-32be" || enc == "utf32be") ref(12001U);
    if (enc == "x-chinese_cns") ref(20000U);
    if (enc == "x_chinese-eten") ref(20002U);
    if (enc == "x-ia5") ref(20105U);
    if (enc == "x-ia5-german") ref(20106U);
    if (enc == "x-ia5-swedish") ref(20107U);
    if (enc == "x-ia5-norwegian") ref(20108U);
    if (enc == "ascii" || enc == "us-ascii") ref(20127U);
    if (enc == "x-ebcdic-koreanextended") ref(20833U);
    if (enc == "ibm-thai") ref(20838U);
    if (enc == "koi8-r") ref(20866U);
    if (enc == "euc-jp") ref(20932U);
    if (enc == "koi8-u") ref(21866U);
    if (enc == "x-europa") ref(29001U);
    if (enc == "iso-8859-8-i") ref(38598U);
    if (enc == "iso-2022-jp") ref(50222U);
    if (enc == "csiso2022jp") ref(50221U);
    if (enc == "iso-2022-kr") ref(50225U);
    if (enc == "euc-cn") ref(51936U);
    if (enc == "euc-kr") ref(51949U);
    if (enc == "hz-gb-2312") ref(52936U);
    if (enc == "gb18030") ref(54936U);
    if (enc == "x-iscii-de") ref(57002U);
    if (enc == "x-iscii-be") ref(57003U);
    if (enc == "x-iscii-ta") ref(57004U);
    if (enc == "x-iscii-te") ref(57005U);
    if (enc == "x-iscii-as") ref(57006U);
    if (enc == "x-iscii-or") ref(57007U);
    if (enc == "x-iscii-ka") ref(57008U);
    if (enc == "x-iscii-ma") ref(57009U);
    if (enc == "x-iscii-gu") ref(57010U);
    if (enc == "x-iscii-pa") ref(57011U);
    if (enc == "utf-7" || enc == "utf7") ref(CP_UTF7);
    if (enc == "utf-8" || enc == "utf8") ref(CP_UTF8);
    if (enc == "shift_jis" || enc == "shiftjis" || enc == "shift-jis") ref(932U);
#undef ref
    return false;
}
#endif

bool encoding::convert(std::string input, std::string& output, std::string ori_enc, std::string des_enc) {
#if HAVE_ICONV
    if (iconv_convert(input, output, ori_enc, des_enc)) return true;
#endif
#if _WIN32
    unsigned int ori_cp, des_cp;
    if (encodingToCp(ori_enc, ori_cp) && encodingToCp(des_enc, des_cp)) {
        std::wstring tmp;
        if (wchar_util::str_to_wstr(tmp, input, ori_cp) && wchar_util::wstr_to_str(output, tmp, des_cp)) return true;
    }
#endif
    return false;
}
