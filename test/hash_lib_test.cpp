#include "gtest/gtest.h"
#include "hash_lib.h"

using namespace hash_lib;

TEST(HashLibTest, SHA512ClassTest) {
    SHA512 sha512;
    sha512.update("Hello, World!");
    GTEST_ASSERT_EQ(sha512.hexDigest(), "374d794a95cdcfd8b35993185fef9ba368f160d8daf432d08ba9f1ed1e5abe6cc69291e0fa2fe0006a52570ef18c19def4e617c33ce52ef0a6e5fbe318cb0387");
    auto re = hash<SHA512>("Hello, World!");
    GTEST_ASSERT_EQ(re, sha512.digest());
    auto hexRe = hashHex<SHA512>("Hello, World!");
    GTEST_ASSERT_EQ(hexRe, sha512.hexDigest());
}

TEST(HashLibTest, SHA512Test) {
    GTEST_ASSERT_EQ(hashHex<SHA512>(""), "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
    GTEST_ASSERT_EQ(hashHex<SHA512>("中文测试"), "1fea9aee07bd0ab66604ef4f079d6b109a0e625c3bc38fe8f850111a9ee6b4a689f3cb454dfd8a16cbd35963382f4ca5d91cdcff2dd473028e6cfee256812eec");
    GTEST_ASSERT_EQ(hashHex<SHA512>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "216b232fc6db1bbdcac1ba59bda0732157d1005c6c5f3ac4cdff555ee013cf48b181a580d6ae3eda8dfd875448e06b1613494fd1bae20dbd8e2f2326634a147c");
}

TEST(HashLibTest, SHA512_256Test) {
    GTEST_ASSERT_EQ(hashHex<SHA512_256>("Hello, World!"), "0686f0a605973dc1bf035d1e2b9bad1985a0bff712ddd88abd8d2593e5f99030");
    GTEST_ASSERT_EQ(hashHex<SHA512_256>(""), "c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a");
    GTEST_ASSERT_EQ(hashHex<SHA512_256>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "4b4c7d22f53f567031db0d4b5751f193a305a765aa16257ef568644891016b7e");
}

TEST(HashLibTest, SHA384Test) {
    GTEST_ASSERT_EQ(hashHex<SHA384>(""), "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b");
    GTEST_ASSERT_EQ(hashHex<SHA384>("Hello, World!"), "5485cc9b3365b4305dfb4e8337e0a598a574f8242bf17289e0dd6c20a3cd44a089de16ab4ab308f63e44b1170eb5f515");
    GTEST_ASSERT_EQ(hashHex<SHA384>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "7123f09157c136ca767487c1316aa1234328f4a4c4749228f3b622f2a6a5964a0142cc90bbc868db7b32cb2652cc3263");
}

TEST(HashLibTest, SHA256Test) {
    GTEST_ASSERT_EQ(hashHex<SHA256>(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    GTEST_ASSERT_EQ(hashHex<SHA256>("Hello, World!"), "dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f");
    GTEST_ASSERT_EQ(hashHex<SHA256>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "29388dd3cd53f3921b3b842e1583980ef2e07a9a48262362decc5870b03fbf6d");
}

TEST(HashLibTest, SHA224Test) {
    GTEST_ASSERT_EQ(hashHex<SHA224>(""), "d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f");
    GTEST_ASSERT_EQ(hashHex<SHA224>("Hello, World!"), "72a23dfa411ba6fde01dbfabf3b00a709c93ebf273dc29e2d8b261ff");
    GTEST_ASSERT_EQ(hashHex<SHA224>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "6dc50e486071b01bf45d5d228207e9c0a8254e1cc48c88b989f8527d");
}

TEST(HashLibTest, SHA1Test) {
    GTEST_ASSERT_EQ(hashHex<SHA1>(""), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    GTEST_ASSERT_EQ(hashHex<SHA1>("Hello, World!"), "0a0a9f2a6772942557ab5355d76af442f8f65e01");
    GTEST_ASSERT_EQ(hashHex<SHA1>("随便来一些中文。测试超过一百二十八字节时的状况。用于测试是否存在问题。还是不够长呢。啊啊啊。"), "21c05e3532d593ec382b8e361d43a17e8fb8774a");
}
