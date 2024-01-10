#include "http_client.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include "err.h"
#include "str_util.h"
#include "urlparse.h"

#include <stdexcept>
#include <string>
#include "inttypes.h"

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
    return Response(socket);
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
    return Request(this->host, this->port, this->https, path, method, this->headers, this->options);
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

Response::Response(Socket socket): socket(socket) {
#if HAVE_ZLIB
    memset(&this->zstream, 0, sizeof(z_stream));
#endif
    parseStatus();
    parseHeader();
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

void Response::parseHeader() {
    if (!this->code) parseStatus();
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
    if (!this->code) this->parseStatus();
    if (!this->headerParsed) this->parseHeader();
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
