#include "hash_lib.h"
#include <string.h>
#include "cstr_util.h"

#define SHA512_DIGEST_LENGTH 64
#define SHA512_BLOCK_SIZE 128
#define SHA512_256_DIGEST_LENGTH 32

using namespace hash_lib;

template<size_t T, class Type>
void cleanBuffer(Type(&buffer)[T]) {
    memset(buffer, 0, sizeof(Type) * T);
}

Hash* Hash::update(const std::string& data) {
    return this->update((const uint8_t*)data.c_str(), data.size());
}

Hash* Hash::update(const std::vector<uint8_t>& data) {
    return this->update(data.data(), data.size());
}

Hash* Hash::update(FILE* f) {
    if (!f) return this;
    uint8_t buffer[1024];
    size_t read;
    while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0) {\
        this->update(buffer, read);
    }
    return this;
}

Hash* Hash::finish(std::string& data) {
    return this->finish((uint8_t*)data.c_str(), data.size());
}

Hash* Hash::finish(std::vector<uint8_t>& data) {
    return this->finish(data.data(), data.size());
}

std::vector<uint8_t> Hash::digest() {
    auto vect = std::vector<uint8_t>(this->digestLength());
    this->finish(vect);
    return vect;
}

std::string Hash::hexDigest() {
    std::string hex;
    auto d = this->digest();
    for (auto i : d) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", i);
        hex += buf;
    }
    return hex;
}

SHA512::SHA512() {
    this->reset();
}

int SHA512::digestLength() {
    return SHA512_DIGEST_LENGTH; // SHA-512 produces a 512-bit hash value (64 bytes)
}

int SHA512::blockSize() {
    return SHA512_BLOCK_SIZE; // SHA-512 processes data in 1024-bit blocks (128 bytes)
}

Hash* SHA512::reset() {
    _initState();
    _bufferLength = 0;
    _bytesHashed = 0;
    _finished = false;
    return this;
}

void SHA512::clean() {
    cleanBuffer(_buffer);
    cleanBuffer(_tempHi);
    cleanBuffer(_tempLo);
    _initState();
}

void SHA512::_initState() {
    stateHi[0] = 0x6a09e667;
    stateHi[1] = 0xbb67ae85;
    stateHi[2] = 0x3c6ef372;
    stateHi[3] = 0xa54ff53a;
    stateHi[4] = 0x510e527f;
    stateHi[5] = 0x9b05688c;
    stateHi[6] = 0x1f83d9ab;
    stateHi[7] = 0x5be0cd19;
    stateLo[0] = 0xf3bcc908;
    stateLo[1] = 0x84caa73b;
    stateLo[2] = 0xfe94f82b;
    stateLo[3] = 0x5f1d36f1;
    stateLo[4] = 0xade682d1;
    stateLo[5] = 0x2b3e6c1f;
    stateLo[6] = 0xfb41bd6b;
    stateLo[7] = 0x137e2179;
}

const uint32_t SHA512_K[] = {
    0x428a2f98, 0xd728ae22, 0x71374491, 0x23ef65cd,
    0xb5c0fbcf, 0xec4d3b2f, 0xe9b5dba5, 0x8189dbbc,
    0x3956c25b, 0xf348b538, 0x59f111f1, 0xb605d019,
    0x923f82a4, 0xaf194f9b, 0xab1c5ed5, 0xda6d8118,
    0xd807aa98, 0xa3030242, 0x12835b01, 0x45706fbe,
    0x243185be, 0x4ee4b28c, 0x550c7dc3, 0xd5ffb4e2,
    0x72be5d74, 0xf27b896f, 0x80deb1fe, 0x3b1696b1,
    0x9bdc06a7, 0x25c71235, 0xc19bf174, 0xcf692694,
    0xe49b69c1, 0x9ef14ad2, 0xefbe4786, 0x384f25e3,
    0x0fc19dc6, 0x8b8cd5b5, 0x240ca1cc, 0x77ac9c65,
    0x2de92c6f, 0x592b0275, 0x4a7484aa, 0x6ea6e483,
    0x5cb0a9dc, 0xbd41fbd4, 0x76f988da, 0x831153b5,
    0x983e5152, 0xee66dfab, 0xa831c66d, 0x2db43210,
    0xb00327c8, 0x98fb213f, 0xbf597fc7, 0xbeef0ee4,
    0xc6e00bf3, 0x3da88fc2, 0xd5a79147, 0x930aa725,
    0x06ca6351, 0xe003826f, 0x14292967, 0x0a0e6e70,
    0x27b70a85, 0x46d22ffc, 0x2e1b2138, 0x5c26c926,
    0x4d2c6dfc, 0x5ac42aed, 0x53380d13, 0x9d95b3df,
    0x650a7354, 0x8baf63de, 0x766a0abb, 0x3c77b2a8,
    0x81c2c92e, 0x47edaee6, 0x92722c85, 0x1482353b,
    0xa2bfe8a1, 0x4cf10364, 0xa81a664b, 0xbc423001,
    0xc24b8b70, 0xd0f89791, 0xc76c51a3, 0x0654be30,
    0xd192e819, 0xd6ef5218, 0xd6990624, 0x5565a910,
    0xf40e3585, 0x5771202a, 0x106aa070, 0x32bbd1b8,
    0x19a4c116, 0xb8d2d0c8, 0x1e376c08, 0x5141ab53,
    0x2748774c, 0xdf8eeb99, 0x34b0bcb5, 0xe19b48a8,
    0x391c0cb3, 0xc5c95a63, 0x4ed8aa4a, 0xe3418acb,
    0x5b9cca4f, 0x7763e373, 0x682e6ff3, 0xd6b2b8a3,
    0x748f82ee, 0x5defb2fc, 0x78a5636f, 0x43172f60,
    0x84c87814, 0xa1f0ab72, 0x8cc70208, 0x1a6439ec,
    0x90befffa, 0x23631e28, 0xa4506ceb, 0xde82bde9,
    0xbef9a3f7, 0xb2c67915, 0xc67178f2, 0xe372532b,
    0xca273ece, 0xea26619c, 0xd186b8c7, 0x21c0c207,
    0xeada7dd6, 0xcde0eb1e, 0xf57d4f7f, 0xee6ed178,
    0x06f067aa, 0x72176fba, 0x0a637dc5, 0xa2c898a6,
    0x113f9804, 0xbef90dae, 0x1b710b35, 0x131c471b,
    0x28db77f5, 0x23047d84, 0x32caab7b, 0x40c72493,
    0x3c9ebe0a, 0x15c9bebc, 0x431d67c4, 0x9c100d4c,
    0x4cc5d4be, 0xcb3e42b6, 0x597f299c, 0xfc657e2a,
    0x5fcb6fab, 0x3ad6faec, 0x6c44198c, 0x4a475817
};

#define SHA512_MACRO(num) \
        h = ah##num; \
        l = al##num; \
        a = l & 0xffff; \
        b = l >> 16; \
        c = h & 0xffff; \
        d = h >> 16; \
        h = stateHi[num]; \
        l = stateLo[num]; \
        a += l & 0xffff; \
        b += l >> 16; \
        c += h & 0xffff; \
        d += h >> 16; \
        b += a >> 16; \
        c += b >> 16; \
        d += c >> 16; \
        stateHi[num] = ah##num = (c & 0xffff) | (d << 16); \
        stateLo[num] = al##num = (a & 0xffff) | (b << 16);

size_t  SHA512::hashBlocks(const uint8_t* m, size_t pos, size_t len) {
    uint32_t ah0 = stateHi[0], ah1 = stateHi[1], ah2 = stateHi[2], ah3 = stateHi[3],
             ah4 = stateHi[4], ah5 = stateHi[5], ah6 = stateHi[6], ah7 = stateHi[7],
             al0 = stateLo[0], al1 = stateLo[1], al2 = stateLo[2], al3 = stateLo[3],
             al4 = stateLo[4], al5 = stateLo[5], al6 = stateLo[6], al7 = stateLo[7];
    uint32_t h, l;
    uint32_t th, tl;
    uint32_t a, b, c, d;
    while (len >= 128) {
        for (int i = 0; i < 16; i++) {
            size_t j = i * 8 + pos;
            _tempHi[i] = cstr_read_uint32(m + j, 1);
            _tempLo[i] = cstr_read_uint32(m + j + 4, 1);
        }
        for (int i = 0; i < 80; i++) {
            uint32_t bh0 = ah0, bh1 = ah1, bh2 = ah2, bh3 = ah3,
                     bh4 = ah4, bh5 = ah5, bh6 = ah6, bh7 = ah7,
                     bl0 = al0, bl1 = al1, bl2 = al2, bl3 = al3,
                     bl4 = al4, bl5 = al5, bl6 = al6, bl7 = al7;
            // add
            h = ah7;
            l = al7;
            a = l & 0xffff;
            b = l >> 16;
            c = h & 0xffff;
            d = h >> 16;
            // Sigma1
            h = ((ah4 >> 14) | (al4 << (32 - 14))) ^ ((ah4 >> 18) | (al4 << (32 - 18))) ^ ((al4 >> (41 - 32)) | (ah4 << (32 - (41 - 32))));
            l = ((al4 >> 14) | (ah4 << (32 - 14))) ^ ((al4 >> 18) | (ah4 << (32 - 18))) ^ ((ah4 >> (41 - 32)) | (al4 << (32 - (41 - 32))));
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            // Ch
            h = (ah4 & ah5) ^ (~ah4 & ah6);
            l = (al4 & al5) ^ (~al4 & al6);
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            // K
            h = SHA512_K[i * 2];
            l = SHA512_K[i * 2 + 1];
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            // w
            h = _tempHi[i % 16];
            l = _tempLo[i % 16];
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            b += a >> 16;
            c += b >> 16;
            d += c >> 16;
            th = c & 0xffff | (d << 16);
            tl = a & 0xffff | (b << 16);
            // add
            h = th;
            l = tl;
            a = l & 0xffff;
            b = l >> 16;
            c = h & 0xffff;
            d = h >> 16;
            // Sigma0
            h = ((ah0 >> 28) | (al0 << (32 - 28))) ^ ((al0 >> (34 - 32)) | (ah0 << (32 - (34 - 32)))) ^ ((al0 >> (39 - 32)) | (ah0 << (32 - (39 - 32))));
            l = ((al0 >> 28) | (ah0 << (32 - 28))) ^ ((ah0 >> (34 - 32)) | (al0 << (32 - (34 - 32)))) ^ ((ah0 >> (39 - 32)) | (al0 << (32 - (39 - 32))));
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            // Maj
            h = (ah0 & ah1) ^ (ah0 & ah2) ^ (ah1 & ah2);
            l = (al0 & al1) ^ (al0 & al2) ^ (al1 & al2);
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            b += a >> 16;
            c += b >> 16;
            d += c >> 16;
            bh7 = (c & 0xffff) | (d << 16);
            bl7 = (a & 0xffff) | (b << 16);
            // add
            h = bh3;
            l = bl3;
            a = l & 0xffff;
            b = l >> 16;
            c = h & 0xffff;
            d = h >> 16;
            h = th;
            l = tl;
            a += l & 0xffff;
            b += l >> 16;
            c += h & 0xffff;
            d += h >> 16;
            b += a >> 16;
            c += b >> 16;
            d += c >> 16;
            bh3 = (c & 0xffff) | (d << 16);
            bl3 = (a & 0xffff) | (b << 16);
            ah1 = bh0;
            ah2 = bh1;
            ah3 = bh2;
            ah4 = bh3;
            ah5 = bh4;
            ah6 = bh5;
            ah7 = bh6;
            ah0 = bh7;
            al1 = bl0;
            al2 = bl1;
            al3 = bl2;
            al4 = bl3;
            al5 = bl4;
            al6 = bl5;
            al7 = bl6;
            al0 = bl7;
            if (i % 16 == 15) {
                for (int j = 0; j < 16; j++) {
                    // add
                    h = _tempHi[j];
                    l = _tempLo[j];
                    a = l & 0xffff;
                    b = l >> 16;
                    c = h & 0xffff;
                    d = h >> 16;
                    h = _tempHi[(j + 9) % 16];
                    l = _tempLo[(j + 9) % 16];
                    a += l & 0xffff;
                    b += l >> 16;
                    c += h & 0xffff;
                    d += h >> 16;
                    // sigma0
                    th = _tempHi[(j + 1) % 16];
                    tl = _tempLo[(j + 1) % 16];
                    h = ((th >> 1) | (tl << (32 - 1))) ^ ((th >> 8) | (tl << (32 - 8))) ^ (th >> 7);
                    l = ((tl >> 1) | (th << (32 - 1))) ^ ((tl >> 8) | (th << (32 - 8))) ^ ((tl >> 7) | (th << (32 - 7)));
                    a += l & 0xffff;
                    b += l >> 16;
                    c += h & 0xffff;
                    d += h >> 16;
                    // sigma1
                    th = _tempHi[(j + 14) % 16];
                    tl = _tempLo[(j + 14) % 16];
                    h = ((th >> 19) | (tl << (32 - 19))) ^ ((tl >> (61 - 32)) | (th << (32 - (61 - 32)))) ^ (th >> 6);
                    l = ((tl >> 19) | (th << (32 - 19))) ^ ((th >> (61 - 32)) | (tl << (32 - (61 - 32)))) ^ ((tl >> 6) | (th << (32 - 6)));
                    a += l & 0xffff;
                    b += l >> 16;
                    c += h & 0xffff;
                    d += h >> 16;
                    b += a >> 16;
                    c += b >> 16;
                    d += c >> 16;
                    _tempHi[j] = (c & 0xffff) | (d << 16);
                    _tempLo[j] = (a & 0xffff) | (b << 16);
                }
            }
        }
        // add
        SHA512_MACRO(0);
        SHA512_MACRO(1);
        SHA512_MACRO(2);
        SHA512_MACRO(3);
        SHA512_MACRO(4);
        SHA512_MACRO(5);
        SHA512_MACRO(6);
        SHA512_MACRO(7);
        pos += 128;
        len -= 128;
    }
    return pos;
}

Hash* SHA512::update(const uint8_t* data, size_t len) {
    if (_finished) {
        return this;
    }
    size_t dataPos = 0;
    _bytesHashed += len;
    if (_bufferLength > 0) {
        while (_bufferLength < SHA512_BLOCK_SIZE && len > 0) {
            _buffer[_bufferLength++] = data[dataPos++];
            len--;
        }
        if (_bufferLength == SHA512_BLOCK_SIZE) {
            hashBlocks(_buffer, 0, SHA512_BLOCK_SIZE);
            _bufferLength = 0;
        }
    }
    if (len >= SHA512_BLOCK_SIZE) {
        dataPos = hashBlocks(data, dataPos, len);
        len %= SHA512_BLOCK_SIZE;
    }
    while (len > 0) {
        _buffer[_bufferLength++] = data[dataPos++];
        len--;
    }
    return this;
}

Hash* SHA512::finish(uint8_t* data, size_t len) {
    if (!_finished) {
        size_t bytesHashed = _bytesHashed;
        size_t left = _bufferLength;
        uint64_t bitLen = bytesHashed << 3;
        size_t padLength = (bytesHashed % SHA512_BLOCK_SIZE) < 112 ? 128 : 256;
        _buffer[left] = 0x80;
        memset(_buffer + left + 1, 0, padLength - left - 9);
        cstr_write_uint64(_buffer + padLength - 8, bitLen, 1);
        hashBlocks(_buffer, 0, padLength);
        _finished = true;
    }
    for (int i = 0; i < this->digestLength() / 8 && i < len / 8; i++) {
        cstr_write_uint32(data + i * 8, stateHi[i], 1);
        cstr_write_uint32(data + i * 8 + 4, stateLo[i], 1);
    }
    return this;
}

SHA512_256::SHA512_256() {
    this->reset();
}

int SHA512_256::digestLength() {
    return SHA512_256_DIGEST_LENGTH; // SHA-512/256 produces a 256-bit hash value (32 bytes)
}

void SHA512_256::_initState() {
    stateHi[0] = 0x22312194;
    stateHi[1] = 0x9f555fa3;
    stateHi[2] = 0x2393b86b;
    stateHi[3] = 0x96387719;
    stateHi[4] = 0x96283ee2;
    stateHi[5] = 0xbe5e1e25;
    stateHi[6] = 0x2b0199fc;
    stateHi[7] = 0x0eb72ddc;
    stateLo[0] = 0xfc2bf72c;
    stateLo[1] = 0xc84c64c2;
    stateLo[2] = 0x6f53b151;
    stateLo[3] = 0x5940eabd;
    stateLo[4] = 0xa88effe3;
    stateLo[5] = 0x53863992;
    stateLo[6] = 0x2c85b8aa;
    stateLo[7] = 0x81c52ca2;
}
