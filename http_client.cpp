#include "http_client.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include "err.h"
#include "str_util.h"

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

Socket::Socket(std::string host, std::string protocol) {
    this->host = host;
    this->protocol = protocol;
#if HAVE_OPENSSL
    if (protocol == "https") {
        return;
    }
#endif
    addrinfo hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int re = getaddrinfo(host.c_str(), protocol.c_str(), &hints, &this->addr);
    if (re) {
        throw AIException(re);
    }
}

Socket::~Socket() {
    if (this->moved) return;
#if HAVE_OPENSSL
    if (this->ssl_ctx) {
        SSL_CTX_free(this->ssl_ctx);
        this->ssl_ctx = nullptr;
    }
    if (this->web) {
        BIO_free_all(this->web);
        this->web = nullptr;
    }
#endif
    this->close();
    if (this->addr) {
        freeaddrinfo(this->addr);
        this->addr = nullptr;
    }
}

void Socket::connect() {
#if HAVE_OPENSSL
    if (this->protocol == "https") {
        const SSL_METHOD* method = SSLv23_client_method();
        if (!method) {
            throw std::runtime_error("SSLv23_client_method failed");
        }
        this->ssl_ctx = SSL_CTX_new(method);
        if (!this->ssl_ctx) {
            throw std::runtime_error("SSL_CTX_new failed");
        }
        const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
        SSL_CTX_set_options(this->ssl_ctx, flags);
        this->web = BIO_new_ssl_connect(this->ssl_ctx);
        if (!this->web) {
            throw std::runtime_error("BIO_new_ssl_connect failed");
        }
        int res =BIO_set_conn_hostname(this->web, this->host.c_str());
        if (res == -1) {
            throw std::runtime_error("BIO_set_conn_hostname failed");
        }
        BIO_get_ssl(this->web, &this->ssl);
        if (!this->ssl) {
            throw std::runtime_error("BIO_get_ssl failed");
        }
        res = SSL_set_tlsext_host_name(this->ssl, this->host.c_str());
        if (res == -1) {
            throw std::runtime_error("SSL_set_tlsext_host_name failed");
        }
        res = BIO_do_connect(this->web);
        if (res != 1) {
            throw std::runtime_error("BIO_do_connect failed");
        }
        res = BIO_do_handshake(this->web);
        if (res != 1) {
            throw std::runtime_error("BIO_do_handshake failed");
        }
        return;
    }
#endif
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
}

size_t Socket::send(const char* data, size_t len, int flags) {
#if HAVE_OPENSSL
    if (this->protocol == "https") {
        int sented = BIO_write(this->web, data, (int)len);
        if (sented <= 0) {
            throw std::runtime_error("BIO_write failed");
        }
        return sented;
    }
#endif
    if (this->socket == -1) {
        throw std::runtime_error("Socket not connected");
    }
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
#if HAVE_OPENSSL
    if (this->protocol == "https") {
        int recved = BIO_read(this->web, data, (int)len);
        if (recved <= 0) {
            throw std::runtime_error("BIO_read failed");
        }
        return recved;
    }
#endif
    if (this->socket == -1) {
        throw std::runtime_error("Socket not connected");
    }
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
#if _WIN32
        closesocket(this->socket);
#else
        ::close(this->socket);
#endif
        closed = true;
    }
}

Request::Request(std::string host, std::string path, std::string method, HeaderMap headers, HttpClientOptions options) {
    this->host = host;
    this->path = path;
    this->method = method;
    this->headers = headers;
    this->options = options;
}

Response Request::send() {
    std::string data;
    data += this->method + " " + this->path + " HTTP/1.1\r\n";
    auto hasBody = !this->body || !this->body->isFinished();
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
    Socket socket(this->host, this->options.https ? "https" : "http");
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
    this->host = host;
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
    this->headers["Host"] = host;
    this->headers["User-Agent"] = "simple-http-client";
    this->headers["Accept"] = "*/*";
}

Request HttpClient::request(std::string path, std::string method) {
    return Request(this->host, path, method, this->headers, this->options);
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
    parseStatus();
    parseHeader();
}

bool Response::pullData() {
    if (!this->buff.empty()) return false;
    this->buff = this->socket.recv(1024);
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
    if (sscanf(parts[1].c_str(), "%" SCNu8, &this->code) != 1) {
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
                } else {
                    throw std::runtime_error("Unspported transfer-encoding");
                }
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
    if (!this->code) this->parseStatus();
    if (!this->headerParsed) this->parseHeader();
    if (this->chunked) {
        std::string data;
        size_t size = -1;
        auto line = this->readLine();
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
        return data;
    } else {
        if (this->pullData()) return "";
        auto data = this->buff;
        this->buff.clear();
        return data;
    }
}

std::string Response::readAll() {
    std::string data;
    auto d = this->read();
    while (!d.empty()) {
        data += d;
        d = this->read();
    }
    return data;
}
