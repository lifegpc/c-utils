#ifndef _UTIL_HASH_LIB_H
#define _UTIL_HASH_LIB_H
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
#include <string>
#include <vector>
#include "fileop.h"
namespace hash_lib {
    class Hash {
    public:
        /**
         * The byte length of the hash
         */
        virtual int digestLength() = 0;
        /**
         * The byte length of the block
         */
        virtual int blockSize() = 0;
        virtual Hash* update(const uint8_t* data, size_t len) = 0;
        template<size_t T>
        Hash* update(const uint8_t(&data)[T]) {
            return this->update(data, T);
        }
        Hash* update(const std::string& data);
        Hash* update(const std::vector<uint8_t>& data);
        Hash* update(FILE* f);
        virtual Hash* reset() = 0;
        virtual Hash* finish(uint8_t* data, size_t len) = 0;
        template<size_t T>
        Hash* finish(uint8_t(&data)[T]) {
            return this->finish(data, T);
        }
        Hash* finish(std::string& data);
        Hash* finish(std::vector<uint8_t>& data);
        std::vector<uint8_t> digest();
        virtual void clean() = 0;
        std::string hexDigest();
    };
    class SHA512: public Hash {
    public:
        SHA512();
        virtual int digestLength() override;
        int blockSize() override;
        Hash* update(const uint8_t* data, size_t len) override;
        using Hash::update;
        Hash* reset() override;
        Hash* finish(uint8_t* data, size_t len) override;
        using Hash::finish;
        void clean() override;
    protected:
        uint32_t stateHi[8];
        uint32_t stateLo[8];
        virtual void _initState();
    private:
        uint32_t _tempHi[16];
        uint32_t _tempLo[16];
        uint8_t _buffer[256];
        size_t _bufferLength = 0;
        size_t _bytesHashed = 0;
        bool _finished = false;
        size_t hashBlocks(const uint8_t* m, size_t pos, size_t len);
    };
    class SHA512_256: public SHA512 {
    public:
        SHA512_256();
        int digestLength() override;
    protected:
        void _initState() override;
    };
    template<class H>
    std::vector<uint8_t> hash(const uint8_t* data, size_t len) {
        H h;
        h.update(data, len);
        return h.digest();
    }
    template<class H>
    std::vector<uint8_t> hash(const std::string& data) {
        H h;
        h.update(data);
        return h.digest();
    }
    template<class H>
    std::vector<uint8_t> hash(const std::vector<uint8_t>& data) {
        H h;
        h.update(data);
        return h.digest();
    }
    template<class H, size_t T>
    std::vector<uint8_t> hash(const uint8_t(&data)[T]) {
        H h;
        h.update(data);
        return h.digest();
    }
    template<class H>
    std::string hashHex(const uint8_t* data, size_t len) {
        H h;
        h.update(data, len);
        return h.hexDigest();
    }
    template<class H>
    std::string hashHex(const std::string& data) {
        H h;
        h.update(data);
        return h.hexDigest();
    }
    template<class H>
    std::string hashHex(const std::vector<uint8_t>& data) {
        H h;
        h.update(data);
        return h.hexDigest();
    }
    template<class H, size_t T>
    std::string hashHex(const uint8_t(&data)[T]) {
        H h;
        h.update(data);
        return h.hexDigest();
    }
    template<class H>
    std::vector<uint8_t> hashFile(const std::string& filePath) {
        FILE* f = fileop::fopen(filePath.c_str(), "rb");
        if (!f) {
            return {};
        }
        H h;
        h.update(f);
        if (ferror(f)) {
            fileop::fclose(f);
            return {};
        }
        fileop::fclose(f);
        return h.digest();
    }
    template<class H>
    std::string hashHexFile(const std::string& filePath) {
        FILE* f = fileop::fopen(filePath.c_str(), "rb");
        if (!f) {
            return "";
        }
        H h;
        h.update(f);
        if (ferror(f)) {
            fileop::fclose(f);
            return "";
        }
        fileop::fclose(f);
        return h.hexDigest();
    }
}
#endif
#endif
