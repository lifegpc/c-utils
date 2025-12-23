#ifndef _UTILS_STREAM_H
#define _UTILS_STREAM_H
#include <stddef.h>
#include <stdint.h>
#include "fileop.h"
#include <vector>
#include <string.h>
#include "cstr_util.h"
#if _WIN32
#include <fcntl.h>
#ifndef _SH_DENYWR
#define _SH_DENYWR 0x20
#endif
#ifndef _O_BINARY
#define _O_BINARY 0x8000
#endif
#ifndef _O_RDONLY
#define _O_RDONLY 0x0000
#endif
#endif

class ReadStream {
public:
    virtual ~ReadStream() {}
    virtual size_t read(uint8_t* buf, size_t size) = 0;
    virtual bool seek(int64_t offset, int whence) {
        return false;
    }
    virtual int64_t tell() {
        return -1;
    }
    virtual bool seekable() = 0;
    virtual bool eof() = 0;
    virtual bool error() = 0;
    virtual bool close() = 0;
    bool readall(const uint8_t* buf, size_t size) {
        size_t total_readed = 0;
        while (total_readed < size) {
            size_t r = read((uint8_t*)buf + total_readed, size - total_readed);
            if (r == 0) break;
            total_readed += r;
        }
        return total_readed == size && !error();
    }
    bool readall(std::vector<uint8_t>& buf) {
        return readall(buf.data(), buf.size());
    }
    template<size_t T>
    bool readall(uint8_t(&data)[T]) {
        return readall(data, T);
    }
    bool readu8(uint8_t& value) {
        uint8_t buf[1];
        if (!readall(buf)) return false;
        value = buf[0];
        return true;
    }
    bool readu16(uint16_t& value) {
        uint8_t buf[2];
        if (!readall(buf)) return false;
        value = cstr_read_uint16(buf, 0);
        return true;
    }
    bool readu32(uint32_t& value) {
        uint8_t buf[4];
        if (!readall(buf)) return false;
        value = cstr_read_uint32(buf, 0);
        return true;
    }
    bool readu64(uint64_t& value) {
        uint8_t buf[8];
        if (!readall(buf)) return false;
        value = cstr_read_uint64(buf, 0);
        return true;
    }
    bool skip(uint64_t bytes) {
        if (seekable()) {
            if (!seek(bytes, SEEK_CUR)) {
                return false;
            }
        } else {
            uint64_t skipped = 0;
            uint8_t buffer[4096];
            while (skipped < bytes) {
                size_t to_read = (bytes - skipped) < sizeof(buffer) ? (bytes - skipped) : sizeof(buffer);
                size_t r = read(buffer, to_read);
                if (r == 0) break;
                skipped += r;
            }
        }
        return !error();
    }
};

class FileReadStream : public ReadStream {
public:
    virtual ~FileReadStream() {
        if (fp) {
            fileop::fclose(fp);
            fp = nullptr;
        }
    }
    /**
     * @brief Open a file for reading
     * @param filename File name (on Windows, UTF-8 encoding is supported)
    */
    FileReadStream(const char* filename) {
#if _WIN32
        int fd = 0;
        int re = fileop::open(filename, fd, _O_RDONLY | _O_BINARY, _SH_DENYWR);
        if (re != 0) {
            errored = true;
            return;
        }
        fp = fileop::fdopen(fd, "rb");
        if (!fp) {
            errored = true;
            fileop::close(fd);
        }
#else
        fp = fileop::fopen(filename, "rb");
        if (!fp) {
            errored = true;
        }
#endif
    }
    virtual size_t read(uint8_t* buf, size_t size) {
        if (!fp) return 0;
        size_t readed = fread(buf, 1, size, fp);
        if (readed != size && ferror(fp)) {
            errored = true;
        }
        return readed;
    }
    virtual bool seek(int64_t offset, int whence) {
        if (!fp) return false;
        if (fileop::fseek(fp, offset, whence) != 0) {
            return false;
        }
        return true;
    }
    virtual int64_t tell() {
        if (!fp) return -1;
        return fileop::ftell(fp);
    }
    virtual bool seekable() {
        return fp != nullptr;
    }
    virtual bool eof() {
        if (!fp) return true;
        return feof(fp) != 0;
    }
    virtual bool error() {
        return errored;
    }
    virtual bool close() {
        if (!fp) return true;
        bool res = fileop::fclose(fp);
        fp = nullptr;
        return res;
    }
private:
    FILE* fp = nullptr;
    bool errored = false;
};

class MemReadStream : public ReadStream {
public:
    MemReadStream(const std::vector<uint8_t>& data) : data(data), pos(0) {}
    
    MemReadStream(std::vector<uint8_t>&& data) : data(std::move(data)), pos(0) {}

    virtual size_t read(uint8_t* buf, size_t size) override {
        if (pos >= data.size()) return 0;
        
        size_t remaining = data.size() - pos;
        size_t to_read = remaining < size ? remaining : size;
        
        memcpy(buf, data.data() + pos, to_read);
        pos += to_read;
        
        return to_read;
    }

    virtual bool seek(int64_t offset, int whence) override {
        int64_t new_pos;
        switch (whence) {
            case SEEK_SET:
                new_pos = offset;
                break;
            case SEEK_CUR:
                new_pos = pos + offset;
                break;
            case SEEK_END:
                new_pos = data.size() + offset;
                break;
            default:
                return false;
        }
        
        if (new_pos < 0 || new_pos > (int64_t)data.size()) {
            return false;
        }
        
        pos = (size_t)new_pos;
        return true;
    }

    virtual int64_t tell() override {
        return (int64_t)pos;
    }

    virtual bool seekable() override {
        return true;
    }

    virtual bool eof() override {
        return pos >= data.size();
    }

    virtual bool error() override {
        return false;
    }

    virtual bool close() override {
        return true;
    }

private:
    const std::vector<uint8_t> data;
    size_t pos = 0;
};

class ReadStreamRegion : public ReadStream {
public:
    ReadStreamRegion(ReadStream* source, int64_t start, int64_t end)
        : source(source), start_pos(start), end_pos(end), current_pos(0) {
        if (!source || !source->seekable()) {
            errored = true;
        }
    }

    virtual size_t read(uint8_t* buf, size_t size) override {
        if (errored || !source) return 0;

        // 计算实际可读取的大小
        int64_t remaining = end_pos - start_pos - current_pos;
        if (remaining <= 0) return 0;

        size_t to_read = (size_t)remaining < size ? (size_t)remaining : size;

        // Seek 到当前应该读取的位置
        if (!source->seek(start_pos + current_pos, SEEK_SET)) {
            errored = true;
            return 0;
        }

        // 从源流读取数据
        size_t readed = source->read(buf, to_read);
        current_pos += readed;

        if (source->error()) {
            errored = true;
        }

        return readed;
    }

    virtual bool seek(int64_t offset, int whence) override {
        if (!source) return false;

        int64_t new_pos;
        switch (whence) {
            case SEEK_SET:
                new_pos = offset;
                break;
            case SEEK_CUR:
                new_pos = current_pos + offset;
                break;
            case SEEK_END:
                new_pos = (end_pos - start_pos) + offset;
                break;
            default:
                return false;
        }

        // 限制在有效范围内
        if (new_pos < 0 || new_pos > end_pos - start_pos) {
            return false;
        }

        current_pos = new_pos;
        return true;
    }

    virtual int64_t tell() override {
        return current_pos;
    }

    virtual bool seekable() override {
        return source && source->seekable();
    }

    virtual bool eof() override {
        if (!source) return true;
        return current_pos >= (end_pos - start_pos);
    }

    virtual bool error() override {
        return errored || (source && source->error());
    }

    virtual bool close() override {
        // 不关闭源流，因为它可能被其他地方使用
        return true;
    }

private:
    ReadStream* source;
    int64_t start_pos;
    int64_t end_pos;
    int64_t current_pos;
    bool errored = false;
};
#endif
