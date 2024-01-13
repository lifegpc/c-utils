#include "http_client.h"

#ifndef _WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#if _WIN32
#include <Windows.h>
#endif

#include "cstr_util.h"
#include "err.h"
#include "fileop.h"
#include "file_reader.h"
#include "str_util.h"
#include "urlparse.h"
#include "time_util.h"

#include <malloc.h>
#include <stdexcept>
#include <string>
#include "inttypes.h"

#include <time.h>

#if _WIN32
static bool inited = false;
static WSADATA wsaData = { 0 };
#endif

#if HAVE_OPENSSL
static bool ssl_inited = false;
#endif

#if HAVE_PRINTF_S
#define printf printf_s
#endif

#if HAVE_SSCANF_S
#define sscanf sscanf_s
#endif

#if HAVE_FPRINTF_S
#define fprintf fprintf_s
#endif

#ifndef _O_BINARY
#if _WIN32
#define _O_BINARY 0x8000
#else
#define _O_BINARY 0
#endif
#endif

#ifndef _SH_DENYWR
#define _SH_DENYWR 0x20
#endif

#ifndef _S_IWRITE
#define _S_IWRITE 0x80
#endif

#ifndef _S_IREAD
#define _S_IREAD 0x100
#endif

AIException::AIException(int code) {
    this->code = code;
}

const char* AIException::what() {
#if _WIN32
    return gai_strerrorA(this->code);
#else
    return gai_strerror(this->code);
#endif
}

std::string getDefaultAcceptEncoding() {
    std::string ae = "";
#if HAVE_ZLIB
    ae += "deflate, gzip";
#endif
    return ae;
}

void make_sure_http_client_inited() {
#if _WIN32
    if (!inited) {
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        inited = true;
    }
#endif
#if HAVE_OPENSSL
    if (!ssl_inited) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        SSL_library_init();
#else
        SSL_load_error_strings();
#endif
        ssl_inited = true;
    }
#endif
}

std::string decodeURIComponent(std::string str) {
    std::string re;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%') {
            if (i + 2 >= str.length()) {
                throw std::runtime_error("Invalid Percent-encoding");
            }
            char buf[3] = { str[i + 1], str[i + 2], 0 };
            int c = strtol(buf, nullptr, 16);
            re += (char)c;
            i += 2;
        } else {
            re += str[i];
        }
    }
    return re;
}

std::string encodeURIComponent(std::string str) {
    std::string re;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == ' ') {
            re += '+';
        } else if (str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~' ||
            (str[i] >= 'a' && str[i] <= 'z') ||
            (str[i] >= 'A' && str[i] <= 'Z') ||
            (str[i] >= '0' && str[i] <= '9')) {
            re += str[i];
        } else {
            char buf[4] = { '%', 0 };
            snprintf(buf + 1, 3, "%02X", (unsigned char)str[i]);
            re += buf;
        }
    }
    return re;
}

SocketError::SocketError() {
#if _WIN32
    this->code = WSAGetLastError();
#else
    this->code = errno;
#endif
#if _WIN32
    if (!err::get_winerror(this->message, this->code)) {
#else
    if (!err::get_errno_message(this->message, this->code)) {
#endif
        char buf[32];
        snprintf(buf, 32, "Unknown error %d", this->code);
        this->message = std::string(buf);
    }
}

const char* SocketError::what() {
    return this->message.c_str();
}

Socket::Socket(std::string host, std::string port, bool https) {
    this->host = host;
    this->port = port;
    this->https = https;
    addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int re = getaddrinfo(host.c_str(), port.c_str(), &hints, &this->addr);
    if (re) {
        throw AIException(re);
    }
}

Socket::~Socket() {
    if (this->moved) return;
    this->close();
#if HAVE_OPENSSL
    if (this->ssl) {
        SSL_free(this->ssl);
        this->ssl = nullptr;
    }
    if (this->ssl_ctx) {
        SSL_CTX_free(this->ssl_ctx);
        this->ssl_ctx = nullptr;
    }
#endif
    if (this->addr) {
        freeaddrinfo(this->addr);
        this->addr = nullptr;
    }
}

void Socket::connect() {
    if (this->socket != -1 || !this->addr) {
        return;
    }
    this->socket = ::socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol);
#if _WIN32
    if (this->socket == INVALID_SOCKET) {
#else
    if (this->socket == -1) {
#endif
        throw SocketError();
    }
    int re = ::connect(this->socket, this->addr->ai_addr, this->addr->ai_addrlen);
#if _WIN32
    if (re == SOCKET_ERROR) {
#else
    if (re == -1) {
#endif
        throw SocketError();
    }
#if HAVE_OPENSSL
    if (this->https) {
        this->ssl_ctx = SSL_CTX_new(TLS_client_method());
        if (!this->ssl_ctx) {
            throw std::runtime_error("SSL_CTX_new failed");
        }
        this->ssl = SSL_new(this->ssl_ctx);
        if (!this->ssl) {
            throw std::runtime_error("SSL_new failed");
        }
        SSL_set_fd(this->ssl, this->socket);
        SSL_set_tlsext_host_name(this->ssl, this->host.c_str());
        re = SSL_connect(this->ssl);
        if (re != 1) {
            throw std::runtime_error("SSL_connect failed");
        }
    }
#endif
}

size_t Socket::send(const char* data, size_t len, int flags) {
    if (this->socket == -1) {
        throw std::runtime_error("Socket not connected");
    }
#if HAVE_OPENSSL
    if (this->https) {
        int sented = SSL_write(this->ssl, data, (int)len);
        if (sented <= 0) {
            throw std::runtime_error("BIO_write failed");
        }
        return sented;
    }
#endif
#if _WIN32
    int sented = ::send(this->socket, data, (int)len, flags);
    if (sented == SOCKET_ERROR) {
#else
    ssize_t sented = ::send(this->socket, data, len, flags);
    if (sented == -1) {
#endif
        throw SocketError();
    }
    return sented;
}

size_t Socket::send(std::string data, int flags) {
    return this->send(data.c_str(), data.length(), flags);
}

size_t Socket::recv(char* data, size_t len, int flags) {
    if (this->socket == -1) {
        throw std::runtime_error("Socket not connected");
    }
#if HAVE_OPENSSL
    if (this->https) {
        int recved = SSL_read(this->ssl, data, (int)len);
        if (recved < 0) {
            throw std::runtime_error("SSL_read failed");
        }
        return recved;
    }
#endif
#if _WIN32
    int recved = ::recv(this->socket, data, (int)len, flags);
    if (recved == SOCKET_ERROR) {
#else
    ssize_t recved = ::recv(this->socket, data, len, flags);
    if (recved == -1) {
#endif
        throw SocketError();
    }
    return recved;
}

std::string Socket::recv(size_t len, int flags) {
    char* buf = new char[len];
    try {
        size_t recved = this->recv(buf, len, flags);
        std::string re(buf, recved);
        delete[] buf;
        return re;
    } catch (std::exception& e) {
        delete[] buf;
        throw e;
    }
}

void Socket::close() {
    if (this->socket != -1 && !closed) {
#if HAVE_OPENSSL
        if (this->https) {
            SSL_set_shutdown(this->ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
            SSL_shutdown(this->ssl);
        }
#endif
#if _WIN32
        closesocket(this->socket);
#else
        ::close(this->socket);
#endif
        closed = true;
    }
}

Request::Request(std::string host, std::string port, bool https, std::string path, std::string method, HeaderMap headers, HttpClientOptions options) {
    this->host = host;
    this->port = port;
    this->https = https;
    this->path = path;
    this->method = method;
    this->headers = headers;
    this->options = options;
}

Response Request::send() {
    if (!this->options.use_custom_cookie) {
        std::string cookie;
        if (this->cookies) {
            cookie = this->cookies->getCookieHeader(this->host, this->path, this->https);
        }
        if (!cookie.empty()) {
            this->headers["Cookie"] = cookie;
        }
    }
    std::string data;
    data += this->method + " " + this->path + " HTTP/1.1\r\n";
    auto hasBody = this->body && !this->body->isFinished();
    if (hasBody) {
        auto type = this->body->contentType();
        if (!type.empty()) {
            this->headers["Content-Type"] = type;
        }
        if (this->body->hasLength()) {
            this->headers["Content-Length"] = std::to_string(this->body->length());
        }
    }
    for (auto& header : this->headers) {
        data += header.first + ": " + header.second + "\r\n";
    }
    data += "\r\n";
#if HAVE_OPENSSL
    Socket socket(this->host, this->port, this->https);
#else
    Socket socket(this->host, "http");
#endif
    socket.connect();
    socket.send(data);
    if (hasBody) {
        while (!this->body->isFinished()) {
            char buf[1024];
            size_t len = this->body->pullData(buf, 1024);
            socket.send(buf, len, 0);
        }
        socket.send("\r\n");
    }
    return Response(socket, *this);
}

void Request::setBody(HttpBody* body) {
    if (this->body != nullptr) delete this->body;
    this->body = body;
}

HttpClient::HttpClient(std::string host) {
    auto pos = host.find("://");
    bool is_number_port = false;
    if (pos == std::string::npos) {
        this->host = host;
    } else {
        this->host = host.substr(pos + 3);
        auto protocol = host.substr(0, pos);
        if (!cstr_stricmp(protocol.c_str(), "http")) {
            this->https = false;
        } else if (!cstr_stricmp(protocol.c_str(), "https")) {
            this->https = true;
        } else {
            throw std::runtime_error("Unspported protocol");
        }
    }
    pos = this->host.find(":");
    if (pos != std::string::npos) {
        this->port = this->host.substr(pos + 1);
        this->host = this->host.substr(0, pos);
        is_number_port = true;
    } else {
        this->port = this->https ? "https" : "http";
    }
    make_sure_http_client_inited();
    if (is_number_port) {
        this->headers["Host"] = this->host + ":" + this->port;
    } else {
        this->headers["Host"] = this->host;
    }
    this->headers["User-Agent"] = "simple-http-client";
    this->headers["Accept"] = "*/*";
    this->headers["Accept-Encoding"] = getDefaultAcceptEncoding();
}

Request HttpClient::request(std::string path, std::string method) {
    Request req(this->host, this->port, this->https, path, method, this->headers, this->options);
    req.cookies = this->cookies;
    return req;
}

void HttpFullBody::pull() {
    if (!this->pulled) {
        this->buff = this->body();
        this->len = this->buff.length();
        this->pulled = true;
    }
}

size_t HttpFullBody::pullData(char* buf, size_t len) {
    this->pull();
    size_t rlen = std::min(this->buff.length(), len);
    memcpy(buf, this->buff.c_str(), rlen);
    this->buff = this->buff.substr(rlen);
    return rlen;
}

bool HttpFullBody::isFinished() {
    this->pull();
    return this->buff.length() == 0;
}

bool HttpFullBody::hasLength() {
    return true;
}

std::string HttpFullBody::contentType() {
    return "";
}

size_t HttpFullBody::length() {
    this->pull();
    return this->len;
}

QueryData::QueryData() {}

QueryData::QueryData(std::string data) {
    auto list = str_util::str_split(data, "&");
    for (auto& item : list) {
        auto kv = str_util::str_splitv(item, "=", 2);
        auto k = decodeURIComponent(kv[0]);
        auto v = decodeURIComponent(kv[1]);
        this->append(k, v);
    }
}

std::string QueryData::contentType() {
    return "application/x-www-form-urlencoded";
}

std::string QueryData::toQuery() {
    std::string re;
    for (auto& item : this->data) {
        for (auto& v : item.second) {
            if (re.length() > 0) {
                re += "&";
            }
            re += encodeURIComponent(item.first) + "=" + encodeURIComponent(v);
        }
    }
    return re;
}

void QueryData::append(std::string key, std::string value) {
    if (this->data.find(key) != this->data.end()) {
        this->data[key].push_back(value);
    } else {
        this->data[key] = std::vector<std::string>({ value });
    }
}

void QueryData::set(std::string key, std::string value) {
    this->data[key] = std::vector<std::string>({ value });
}

std::string QueryData::body() {
    return this->toQuery();
}

Request::~Request() {
    if (this->body) {
        delete this->body;
    }
}

Response::Response(Socket socket, Request& req): socket(socket) {
#if HAVE_ZLIB
    memset(&this->zstream, 0, sizeof(z_stream));
#endif
    parseStatus();
    parseHeader(req);
}

bool Response::pullData() {
    if (!this->buff.empty()) return false;
    this->buff = this->socket.recv(1024);
    if (this->buff.empty()) {
        this->eof = true;
    }
    return this->buff.empty();
}

void Response::parseStatus() {
    if (this->code) return;
    auto line = this->readLine();
    auto parts = str_util::str_splitv(line, " ", 3);
    if (parts.size() < 3) {
        throw std::runtime_error("Invalid HTTP status line");
    }
    if (cstr_stricmp(parts[0].c_str(), "http/1.1")) {
        throw std::runtime_error("Unspported HTTP version");
    }
    if (sscanf(parts[1].c_str(), "%" SCNu16, &this->code) != 1) {
        throw std::runtime_error("Invalid HTTP status code");
    }
    this->reason = parts[2];
}

void Response::parseHeader(Request& req) {
    if (this->headerParsed) return;
    this->headerParsed = true;
    auto line = this->readLine();
    while (!line.empty()) {
        auto kv = str_util::str_splitv(line, ": ", 2);
        if (kv.size() < 2) {
            throw std::runtime_error("Invalid HTTP header");
        }
        this->headers[kv[0]] = kv[1];
        if (!cstr_stricmp(kv[0].c_str(), "transfer-encoding")) {
            auto list = str_util::str_splitv(kv[1], ",");
            for (auto& item : list) {
                auto it = str_util::str_trim(item);
                if (!cstr_stricmp(it.c_str(), "chunked")) {
                    this->chunked = true;
                    break;
#if HAVE_ZLIB
                } else if (!cstr_stricmp(it.c_str(), "gzip")) {
                    this->gzip = true;
                    inflateInit2(&this->zstream, 16 + MAX_WBITS);
                    break;
                } else if (!cstr_stricmp(it.c_str(), "deflate")) {
                    this->deflate = true;
                    inflateInit2(&this->zstream, -MAX_WBITS);
                    break;
#endif
                } else {
                    throw std::runtime_error("Unspported transfer-encoding");
                }
            }
        } else if (!cstr_stricmp(kv[0].c_str(), "content-encoding")) {
#if HAVE_ZLIB
            if (kv[1] == "gzip") {
                this->gzip = true;
                inflateInit2(&this->zstream, 16 + MAX_WBITS);

            } else if (kv[1] == "deflate") {
                this->deflate = true;
                inflateInit2(&this->zstream, -MAX_WBITS);
            } else {
                throw std::runtime_error("Unspported content-encoding");
            }
#else
            throw std::runtime_error("Unspported content-encoding");
#endif
        } else if (!cstr_stricmp(kv[0].c_str(), "set-cookie")) {
            if (req.cookies) {
                req.cookies->handleSetCookie(req, kv[1]);
            }
        }
        line = this->readLine();
    }
}

std::string Response::readLine() {
    std::string line;
    while (true) {
        if (this->pullData()) break;
        auto pos = this->buff.find("\r\n");
        if (pos == std::string::npos) {
            line += this->buff;
            this->buff.clear();
        } else {
            line += this->buff.substr(0, pos);
            this->buff = this->buff.substr(pos + 2);
            break;
        }
    }
    return line;
}

std::string Response::read() {
    if (this->chunked) {
        std::string data;
        size_t size = -1;
        auto line = this->readLine();
        if (this->eof) return "";
        if (sscanf(line.c_str(), "%zx", &size) != 1) {
            throw std::runtime_error("Invalid chunk size");
        }
        size_t osize = size;
        while (size > 0) {
            if (this->pullData()) break;
            auto len = std::min(this->buff.length(), size);
            data += this->buff.substr(0, len);
            this->buff = this->buff.substr(len);
            size -= len;
        }
        if (osize != data.length()) {
            throw std::runtime_error("Chunk size != data length");
        }
        this->readLine();
        if (!osize) {
            this->eof = true;
        }
#if HAVE_ZLIB
        if (this->gzip || this->deflate) {
            return this->inflate(data);
        }
#endif
        return data;
    } else {
        if (this->pullData()) return "";
        auto data = this->buff;
        this->buff.clear();
#if HAVE_ZLIB
        if (this->gzip || this->deflate) {
            return this->inflate(data);
        }
#endif
        return data;
    }
}

std::string Response::readAll() {
    std::string data;
    auto d = this->read();
    while (!this->eof) {
        data += d;
        d = this->read();
    }
    return data;
}

Response::~Response() {
#if HAVE_ZLIB
    if (this->gzip || this->deflate) {
        inflateEnd(&this->zstream);
    }
#endif
}

#if HAVE_ZLIB
std::string Response::inflate(std::string data) {
    this->zstream.next_in = (Bytef*)data.c_str();
    this->zstream.avail_in = data.length();
    std::string re;
    while (true) {
        char buf[10240];
        this->zstream.next_out = (Bytef*)buf;
        this->zstream.avail_out = 10240;
        int res = ::inflate(&this->zstream, Z_NO_FLUSH);
        if (res == Z_STREAM_END) {
            re += std::string(buf, 10240 - this->zstream.avail_out);
            break;
        } else if (res == Z_STREAM_ERROR || res == Z_NEED_DICT || res == Z_DATA_ERROR || res == Z_MEM_ERROR) {
            throw std::runtime_error("inflate failed");
        } else {
            re += std::string(buf, 10240 - this->zstream.avail_out);
        }
        if (this->zstream.avail_out != 0) break;
    }
    return re;
}
#endif

Request::Request(std::string url, std::string method, HttpClientOptions options, HeaderMap headers) {
    auto u = urlparse(url.c_str(), nullptr, 1);
    if (!u) {
        throw std::runtime_error("Invalid URL");
    }
    this->path = u->path;
    std::string params = u->params, query = u->query, scheme = u->scheme, netloc = u->netloc;
    free_url_parse_result(u);
    if (!params.empty()) {
        this->path += ";" + params;
    }
    if (!query.empty()) {
        this->path += "?" + query;
    }
    bool is_number_port = false;
    if (!scheme.empty()) {
        if (!cstr_stricmp(scheme.c_str(), "http")) {
            this->https = false;
        } else if (!cstr_stricmp(scheme.c_str(), "https")) {
            this->https = true;
        } else {
            throw std::runtime_error("Unspported protocol");
        }
    }
    auto pos = netloc.find(":");
    if (pos != std::string::npos) {
        this->port = netloc.substr(pos + 1);
        this->host = netloc.substr(0, pos);
        is_number_port = true;
    } else {
        this->host = netloc;
        this->port = this->https ? "https" : "http";
    }
    this->method = method;
    this->options = options;
    if (is_number_port) {
        this->headers["Host"] = this->host + ":" + this->port;
    } else {
        this->headers["Host"] = this->host;
    }
    this->headers["User-Agent"] = "simple-http-client";
    this->headers["Accept"] = "*/*";
    this->headers["Accept-Encoding"] = getDefaultAcceptEncoding();
    for (auto& header : headers) {
        this->headers[header.first] = header.second;
    }
    make_sure_http_client_inited();
}

HttpBody* Request::getBody() {
    return this->body;
}

bool Response::isEof() {
    return this->eof;
}

Cookie::Cookie(std::string name, std::string value, std::string domain, std::string path, bool secure, bool httpOnly, int64_t expires) {
    this->name = name;
    this->value = value;
    this->domain = domain;
    this->path = path;
    this->secure = secure;
    this->httpOnly = httpOnly;
    this->expires = expires;
}

std::string Cookies::getCookieHeader(std::string host, std::string path, bool https) {
    std::string re;
    if (this->cookies.find(host) != this->cookies.end()) {
        for (auto& cookie : this->cookies[host]) {
            if (cookie.expires != 0 && cookie.expires < time(nullptr)) {
                continue;
            }
            if (cookie.secure && !https) {
                continue;
            }
            if (!cookie.path.empty() && path.find(cookie.path) != 0) {
                continue;
            }
            if (!re.empty()) {
                re += "; ";
            }
            re += cookie.name + "=" + cookie.value;
        }
    }
    std::string host2 = "." + host;
    if (this->cookies.find(host2) != this->cookies.end()) {
        for (auto& cookie : this->cookies[host2]) {
            if (cookie.expires != 0 && cookie.expires < time(nullptr)) {
                continue;
            }
            if (cookie.secure && !https) {
                continue;
            }
            if (!cookie.path.empty() && path.find(cookie.path) != 0) {
                continue;
            }
            if (!re.empty()) {
                re += "; ";
            }
            re += cookie.name + "=" + cookie.value;
        }
    }
    auto pos = host.find(".");
    if (pos != std::string::npos) {
        host = host.substr(pos);
        if (this->cookies.find(host) != this->cookies.end()) {
            for (auto& cookie : this->cookies[host]) {
                if (cookie.expires != 0 && cookie.expires < time(nullptr)) {
                    continue;
                }
                if (cookie.secure && !https) {
                    continue;
                }
                if (!cookie.path.empty() && path.find(cookie.path) != 0) {
                    continue;
                }
                if (!re.empty()) {
                    re += "; ";
                }
                re += cookie.name + "=" + cookie.value;
            }
        }
    }
    return re;
}

void Cookies::handleSetCookie(Request& req, std::string set_cookie) {
    auto list = str_util::str_split(set_cookie, ";");
    std::string name, value, domain, path;
    bool secure = false, httpOnly = false;
    int64_t expires = 0;
    bool first = true;
    for (auto& item: list) {
        auto it = str_util::str_trim(item);
        auto kv = str_util::str_splitv(it, "=", 2);
        if (first) {
            if (kv.size() < 2) throw std::runtime_error("Invalid Set-Cookie: No value");
            name = kv[0];
            value = kv[1];
            first = false;
        }
        if (kv.size() < 2) {
            if (!cstr_stricmp(it.c_str(), "HttpOnly")) {
                httpOnly = true;
            } else if (!cstr_stricmp(it.c_str(), "Secure")) {
                secure = true;
            }
        } else {
            if (!cstr_stricmp(kv[0].c_str(), "Domain")) {
                domain = kv[1];
            } else if (!cstr_stricmp(kv[0].c_str(), "Path")) {
                path = kv[1];
            } else if (!cstr_stricmp(kv[0].c_str(), "Expires")) {
                struct tm tm;
                if (!time_util::strptime(kv[1].c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm)) {
                    throw std::runtime_error("Invalid Set-Cookie: Invalid Expires");
                }
                expires = time_util::timegm(&tm);
            } else if (!cstr_stricmp(kv[0].c_str(), "Max-Age")) {
                int64_t max_age;
                if (sscanf(kv[1].c_str(), "%" SCNd64, &max_age) != 1) {
                    throw std::runtime_error("Invalid Set-Cookie: Invalid Max-Age");
                }
                expires = time(nullptr) + max_age;
            }
        }
    }
    if (domain.empty()) {
        domain = req.host;
    }
    if (path.empty()) {
        path = req.path;
    }
    if (this->cookies.find(domain) == this->cookies.end()) {
        this->cookies[domain] = std::list<Cookie>();
        this->cookies[domain].push_back(Cookie(name, value, domain, path, secure, httpOnly, expires));
    } else {
        bool found = false;
        for (auto it = this->cookies[domain].begin(); it != this->cookies[domain].end(); it++) {
            if (it->name == name && it->path == path) {
                it->value = value;
                it->domain = domain;
                it->secure = secure;
                it->httpOnly = httpOnly;
                it->expires = expires;
                found = true;
                break;
            }
        }
        if (!found) {
            this->cookies[domain].push_back(Cookie(name, value, domain, path, secure, httpOnly, expires));
        }
    }
}

NetscapeCookies::NetscapeCookies() {}

NetscapeCookies::NetscapeCookies(std::string path) {
    this->path = path;
    this->load();
}

NetscapeCookies::~NetscapeCookies() {
    if (this->save_when_disposed) this->save();
}

bool NetscapeCookies::load() {
    if (this->path.empty()) return true;
    if (!fileop::exists(this->path)) return true;
    int fd;
    int err = fileop::open(this->path, fd, O_RDONLY | _O_BINARY, _SH_DENYWR, _S_IWRITE | _S_IREAD);
    if (err < 0) {
        return false;
    }
    FILE* f = fileop::fdopen(fd, "rb");
    if (!f) {
        fileop::close(fd);
        return false;
    }
    auto reader = create_file_reader(f, 0);
    char* line = nullptr;
    size_t line_size = 0;
    std::string l;
    bool re = true;
    if (file_reader_read_line(reader, &line, &line_size)) {
        re = false;
        goto end;
    }
    l = std::string(line, line_size);
    free(line);
    line = nullptr;
    while (!l.empty()) {
        std::string name, value, domain, path;
        bool secure = false, httpOnly = false;
        int64_t expires = 0;
        if (!cstr_strnicmp(l.c_str(), "#HttpOnly_", 10)) {
            l = l.substr(10);
            httpOnly = true;
        }
        if (l.find("#") == 0) {
            if (file_reader_read_line(reader, &line, &line_size)) {
                break;
            }
            l = std::string(line, line_size);
            free(line);
            line = nullptr;
            continue;
        }
        auto list = str_util::str_splitv(l, "\t", 7);
        if (list.size() < 7) {
            re = false;
            goto end;
        }
        domain = list[0];
        path = list[2];
        secure = !cstr_stricmp(list[3].c_str(), "TRUE");
        if (sscanf(list[4].c_str(), "%" SCNd64, &expires) != 1) {
            re = false;
            goto end;
        }
        name = list[5];
        value = list[6];
        if (this->cookies.find(domain) == this->cookies.end()) {
            this->cookies[domain] = std::list<Cookie>();
            this->cookies[domain].push_back(Cookie(name, value, domain, path, secure, httpOnly, expires));
        } else {
            this->cookies[domain].push_back(Cookie(name, value, domain, path, secure, httpOnly, expires));
        }
        if (file_reader_read_line(reader, &line, &line_size)) {
            break;
        }
        l = std::string(line, line_size);
        free(line);
        line = nullptr;
    }
end:
    if (reader) free_file_reader(reader);
    if (f) fileop::fclose(f);
    if (line) free(line);
    return re;
}

bool NetscapeCookies::save() {
    if (this->path.empty()) return true;
    int fd;
    int err = fileop::open(this->path, fd, O_WRONLY | O_CREAT | _O_BINARY, 16, _S_IWRITE | _S_IREAD);
    if (err < 0) {
        return false;
    }
    FILE* f = fileop::fdopen(fd, "wb");
    if (!f) {
        fileop::close(fd);
        return false;
    }
    fprintf(f, "# Netscape HTTP Cookie File\n\
# http://curl.haxx.se/rfc/cookie_spec.html\n");
    for (auto& domain : this->cookies) {
        for (auto& cookie : domain.second) {
            if (cookie.expires == 0 && cookie.expires < time(nullptr)) {
                continue;
            }
            if (cookie.secure) {
                fprintf(f, "#HttpOnly_");
            }
            fprintf(f, "%s\t%s\t%s\t%s\t%s\t%s\t%s\n", cookie.domain.c_str(), cookie.domain.find(".") == 0 ? "TRUE" : "FALSE", cookie.path.c_str(), cookie.secure ? "TRUE" : "FALSE", std::to_string(cookie.expires).c_str(), cookie.name.c_str(), cookie.value.c_str());
        }
    }
    fileop::fclose(f);
    return true;
}

std::map<std::string, std::string> parseCookie(std::string cookie) {
    std::map<std::string, std::string> re;
    auto list = str_util::str_split(cookie, ";");
    for (auto& item : list) {
        auto kv = str_util::str_splitv(item, "=", 2);
        if (kv.size() >= 2) {
            re[str_util::str_trim(kv[0])] = str_util::str_trim(kv[1]);
        }
    }
    return re;
}

std::string dumpCookie(std::map<std::string, std::string> cookie) {
    std::string re;
    for (auto& item : cookie) {
        if (!re.empty()) {
            re += "; ";
        }
        re += item.first + "=" + item.second;
    }
    return re;
}

std::string Request::toUri() {
    std::string re;
    if (this->https) {
        re += "https://";
    } else {
        re += "http://";
    }
    re += this->host;
    if (this->port != (this->https ? "https" : "http")) {
        re += ":" + this->port;
    }
    re += this->path;
    return re;
}
