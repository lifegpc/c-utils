#ifndef _UTIL_HTTP_CLIENT_H
#define _UTIL_HTTP_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "cstr_util.h"
#include "utils_config.h"

#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#if HAVE_OPENSSL
#include "openssl/ssl.h"
#endif

struct HeaderNameCompare {
    bool operator()(const std::string& a, const std::string& b) const {
        return cstr_stricmp(a.c_str(), b.c_str()) < 0;
    }
};

typedef std::map<std::string, std::string, HeaderNameCompare> HeaderMap;

std::string decodeURIComponent(std::string str);

class HttpClientOptions {
public:
    bool https = false;
};

class HttpBody {
public:
    virtual size_t pullData(char* buf, size_t len) = 0;
    virtual bool isFinished() = 0;
    virtual std::string contentType() = 0;
    virtual bool hasLength() = 0;
    virtual size_t length() = 0;
};

class HttpFullBody: public HttpBody {
public:
    virtual size_t pullData(char* buf, size_t len);
    virtual bool isFinished();
    virtual bool hasLength();
    virtual std::string contentType();
    virtual size_t length();
protected:
    virtual std::string body() = 0;
private:
    void pull();
    std::string buff;
    bool pulled;
    size_t len;
};

class QueryData: public HttpFullBody {
public:
    QueryData();
    QueryData(std::string data);
    std::string contentType();
    std::map<std::string, std::vector<std::string>> data;
    std::string toQuery();
    void append(std::string key, std::string value);
    void set(std::string key, std::string value);
protected:
    std::string body();
};

class AIException: std::exception {
public:
    AIException(int code);
    const char* what();
private:
    int code;
};

class SocketError: std::exception {
public:
    SocketError();
    const char* what();
private:
    int code;
    std::string message;
};

class Socket {
public:
    Socket(std::string host, std::string protocol);
    Socket(Socket& socket) {
        *this = socket;
        socket.moved = true;
    }
    ~Socket();
    void connect();
    size_t send(const char* data, size_t len, int flags = 0);
    template<size_t T>
    size_t send(const char(&data)[T], int flags = 0) {
        return this->send(data, T, flags);
    }
    size_t send(std::string data, int flags = 0);
    size_t recv(char* data, size_t len, int flags = 0);
    template<size_t T>
    size_t recv(char(&data)[T], int flags = 0) {
        return this->recv(data, T, flags);
    }
    std::string recv(size_t len, int flags = 0);
    void close();
private:
#if HAVE_OPENSSL
    SSL_CTX* ssl_ctx = nullptr;
    BIO* web = nullptr;
    SSL* ssl = nullptr;
#endif
    bool moved = false;
    std::string host;
    std::string protocol;
    int socket = -1;
    bool closed = false;
    addrinfo* addr = nullptr;
};

typedef struct Response Response;

class Request {
public:
    Request(std::string host, std::string path, std::string method, HeaderMap headers, HttpClientOptions options);
    ~Request();
    Response send();
    HeaderMap headers;
    HttpClientOptions options;
    void setBody(HttpBody* body);
    std::string host;
    std::string path;
    std::string method;
private:
    HttpBody* body = nullptr;
};

class Response {
public:
    Response() = delete;
    explicit Response(Socket socket);
    HeaderMap headers;
    uint16_t code = 0;
    std::string reason;
    std::string read();
    std::string readAll();
private:
    std::string readLine();
    void parseHeader();
    void parseStatus();
    bool pullData();
    bool headerParsed = false;
    bool chunked = false;
    Socket socket;
    std::string buff;
};

class HttpClient {
public:
    HttpClient(std::string host);
    Request request(std::string path, std::string method);
    HttpClientOptions options;
    HeaderMap headers;
private:
    std::string host;
};

#endif
