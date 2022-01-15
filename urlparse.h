#ifndef _UTIL_URLPARSE_H
#define _UTIL_URLPARSE_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UrlParseResult {
    char* scheme;
    char* netloc;
    char* path;
    char* params;
    char* query;
    char* fragment;
} UrlParseResult;
/**
 * @brief Parse a URL into 6 components: <scheme>://<netloc>/<path>;<params>?<query>#<fragment>
 * @param url URL
 * @param sch Provides the default value of the scheme component when no scheme is found in url.
 * @param allow_fragments If is 0, no attempt is made to separate the fragment component from the previous component, which can be either path or query.
 * @return 
*/
UrlParseResult* urlparse(const char* url, const char* sch, char allow_fragments);
void free_url_parse_result(UrlParseResult* r);
void dump_url_parse_result(UrlParseResult* r, int indent_now);
#ifdef __cplusplus
}
#endif
#endif
