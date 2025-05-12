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
    class SHA384: public SHA512 {
    public:
        SHA384();
        int digestLength() override;
    protected:
        void _initState() override;
    };
    class SHA256: public Hash {
    public:
        SHA256();
        virtual int digestLength() override;
        int blockSize() override;
        Hash* update(const uint8_t* data, size_t len) override;
        using Hash::update;
        Hash* reset() override;
        Hash* finish(uint8_t* data, size_t len) override;
        using Hash::finish;
        void clean() override;
    protected:
        uint32_t state[8];
        virtual void _initState();
    private:
        uint32_t _temp[64];
        uint8_t _buffer[128];
        size_t _bufferLength = 0;
        size_t _bytesHashed = 0;
        bool _finished = false;
        size_t hashBlocks(const uint8_t* m, size_t pos, size_t len);
    };
    class SHA224: public SHA256 {
    public:
        SHA224();
        int digestLength() override;
    protected:
        void _initState() override;
    };
    class SHA1: public Hash {
    public:
        SHA1();
        virtual int digestLength() override;
        int blockSize() override;
        Hash* update(const uint8_t* data, size_t len) override;
        using Hash::update;
        Hash* reset() override;
        Hash* finish(uint8_t* data, size_t len) override;
        using Hash::finish;
        void clean() override;
    protected:
        uint32_t state[5];
        virtual void _initState();
    private:
        uint32_t _temp[80];
        uint8_t _buffer[128];
        size_t _bufferLength = 0;
        size_t _bytesHashed = 0;
        bool _finished = false;
        size_t hashBlocks(const uint8_t* m, size_t pos, size_t len);
    };
    template<class H>
    class HMAC: public Hash {
    public:
        HMAC(const uint8_t* key, size_t len) {
            int blockSize = _outer.blockSize();
            std::vector<uint8_t> pad(blockSize);
            if (len > blockSize) {
                _inner.update(key, len)->finish(pad)->clean();
            } else {
                memcpy(pad.data(), key, len);
            }
            _pad = std::move(pad);
            reset();
        }
        HMAC(const std::string& key) : HMAC((const uint8_t*)key.c_str(), key.size()) {}
        HMAC(const std::vector<uint8_t>& key) : HMAC(key.data(), key.size()) {}
        template <size_t T>
        HMAC(const uint8_t (&key)[T]) : HMAC(key, T) {}
        Hash* reset() override {
            _inner.clean();
            _outer.clean();
            int blockSize = _outer.blockSize();
            auto pad(_pad);
            for (int i = 0; i < blockSize; i++) {
                pad[i] ^= 0x36;
            }
            _inner.update(pad);
            for (int i = 0; i < blockSize; i++) {
                pad[i] ^= 0x36 ^ 0x5c;
            }
            _outer.update(pad);
            _finished = false;
            return this;
        }
        int digestLength() override {
            return _outer.digestLength();
        }
        int blockSize() override {
            return _outer.blockSize();
        }
        Hash* update(const uint8_t* data, size_t len) override {
            _inner.update(data, len);
            return this;
        }
        using Hash::update;
        Hash* finish(uint8_t* data, size_t len) override {
            if (_finished) {
                _outer.finish(data, len);
                return this;
            }
            _outer.update(_inner.digest())->finish(data, len);
            _finished = true;
            return this;
        }
        using Hash::finish;
        void clean() override {
            reset();
        }
    private:
        H _inner;
        H _outer;
        std::vector<uint8_t> _pad;
        bool _finished = false; 
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
