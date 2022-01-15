#include "urlparse.h"
#include "utils_config.h"

#include <array>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "cstr_util.h"
#include "str_util.h"

#if HAVE_PRINTF_S
#define printf printf_s
#endif

bool is_valid_scheme_chars(char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == '+' || c == '-' || c == '.') return true;
    return false;
}

bool scheme_is_use_params(std::string name) {
    const std::array<std::string, 15> arr = { "", "ftp", "hdl", "prospero", "http", "imap", "https", "shttp", "rtsp", "rtspu", "sip", "sips", "mms", "sftp", "tel" };
    for (auto i = arr.begin(); i != arr.end(); i++) {
        if (name == *i) return true;
    }
    return false;
}

UrlParseResult* urlparse(const char* url, const char* sch, char allow_fragments) {
    if (!url) return nullptr;
    std::string u(url);
    std::string scheme, netloc, params, query, fragment;
    if (sch) scheme = sch;
    u = str_util::str_replace(u, "\t", "");
    u = str_util::str_replace(u, "\r", "");
    u = str_util::str_replace(u, "\n", "");
    auto i = u.find(':');
    if (i > 0 && i != -1) {
        bool ok = true;
        for (size_t j = 0; j < i; j++) {
            if (!is_valid_scheme_chars(u[j])) {
                ok = false;
                break;
            }
        }
        if (ok) {
            std::string tmp = u.substr(0, i);
            if (!str_util::tolowercase(tmp, scheme)) return nullptr;
            u = u.substr(i + 1);
        }
    }
    if (u.find("//") == 0) {
        auto delim = u.size();
        const char* s = "/?#";
        for (int i = 0; i < 3; i++) {
            auto wdelim = u.find(s[i], 2);
            if (wdelim != -1) {
                delim = std::min(delim, wdelim);
            }
        }
        netloc = u.substr(2, delim - 2);
        u = u.substr(delim);
    }
    if (allow_fragments) {
        auto i = u.find('#');
        if (i != -1) {
            fragment = u.substr(i + 1);
            u = u.substr(0, i);
        }
    }
    i = u.find('?');
    if (i != -1) {
        query = u.substr(i + 1);
        u = u.substr(0, i);
    }
    i = u.find(';');
    if (i != -1 && scheme_is_use_params(scheme)) {
        auto j = u.rfind('/');
        if (j == -1) {
            params = u.substr(i + 1);
            u = u.substr(0, i);
        } else {
            i = u.find(';', j);
            if (i != -1) {
                params = u.substr(i + 1);
                u = u.substr(0, i);
            }
        }
    }
    auto r = (UrlParseResult*)malloc(sizeof(UrlParseResult));
    if (!r) {
        return nullptr;
    }
    memset(r, 0, sizeof(UrlParseResult));
    if (cstr_util_copy_str(&r->scheme, scheme.c_str())) {
        goto end;
    }
    if (cstr_util_copy_str(&r->netloc, netloc.c_str())) {
        goto end;
    }
    if (cstr_util_copy_str(&r->path, u.c_str())) {
        goto end;
    }
    if (cstr_util_copy_str(&r->params, params.c_str())) {
        goto end;
    }
    if (cstr_util_copy_str(&r->query, query.c_str())) {
        goto end;
    }
    if (cstr_util_copy_str(&r->fragment, fragment.c_str())) {
        goto end;
    }
    return r;
end:
    free_url_parse_result(r);
    return nullptr;
}

void free_url_parse_result(UrlParseResult* r) {
    if (!r) return;
    if (r->scheme) free(r->scheme);
    if (r->netloc) free(r->netloc);
    if (r->path) free(r->path);
    if (r->params) free(r->params);
    if (r->query) free(r->query);
    if (r->fragment) free(r->fragment);
    free(r);
}

void dump_url_parse_result(UrlParseResult* r, int indent_now) {
    if (!r) return;
    std::string ind(indent_now, ' ');
    if (r->scheme) printf("%sScheme: %s\n", ind.c_str(), r->scheme);
    if (r->netloc) printf("%sNetloc: %s\n", ind.c_str(), r->netloc);
    if (r->path) printf("%sPath: %s\n", ind.c_str(), r->path);
    if (r->params) printf("%sParams: %s\n", ind.c_str(), r->params);
    if (r->query) printf("%sQuery: %s\n", ind.c_str(), r->query);
    if (r->fragment) printf("%sFragment: %s\n", ind.c_str(), r->fragment);
}
