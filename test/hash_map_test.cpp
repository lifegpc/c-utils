#include "gtest/gtest.h"
#include "hash_map.h"
#include <string>

TEST(HashMapTest, NextCapTest) {
    GTEST_ASSERT_EQ(hash_map_get_next_cap(0), 53);
    GTEST_ASSERT_EQ(hash_map_get_next_cap(53), 97);
    GTEST_ASSERT_EQ(hash_map_get_next_cap(1610612741), 3221225482);
}

TEST(HashMapTest, ProbingTest) {
    auto i = hash_map_linear_probing(1);
    GTEST_ASSERT_EQ(i(1), 1);
    GTEST_ASSERT_EQ(i(3), 3);
    i = hash_map_linear_probing(33);
    GTEST_ASSERT_EQ(i(3), 99);
    GTEST_ASSERT_EQ(hash_map_quadratic_probing(4), 16);
    GTEST_ASSERT_EQ(hash_map_quadratic_probing_alter(3), 4);
    GTEST_ASSERT_EQ(hash_map_quadratic_probing_alter(4), -4);
    i = hash_map_random_probing(0);
    GTEST_ASSERT_EQ(i(1), 16539830640600551411llu);
    GTEST_ASSERT_EQ(i(2), 9045840598434793555llu);
}

TEST(HashMapTest, HashMap) {
    auto map = hash_map_new<std::string, int>();
    GTEST_ASSERT_TRUE(map);
    GTEST_ASSERT_TRUE(hash_map_insert(map, "123", 123));
    GTEST_ASSERT_TRUE(hash_map_resize(map, hashmap_primes[1]));
    GTEST_ASSERT_TRUE(hash_map_insert(map, "234", 234));
    GTEST_ASSERT_EQ(hash_map_get_entry(map, "123")->value, 123);
    int v = 0;
    GTEST_ASSERT_TRUE(hash_map_get(map, "234", v));
    GTEST_ASSERT_FALSE(hash_map_get(map, "333", v));
    GTEST_ASSERT_FALSE(hash_map_get_entry(map, "333"));
    GTEST_ASSERT_EQ(v, 234);
    GTEST_ASSERT_TRUE(hash_map_insert(map, "2222", 2222));
    GTEST_ASSERT_TRUE(hash_map_get_entry(map, "2222"));
    GTEST_ASSERT_TRUE(hash_map_delete(map, "2222", &v));
    GTEST_ASSERT_EQ(v, 2222);
    GTEST_ASSERT_FALSE(hash_map_get_entry(map, "2222"));
    free_hash_map(map);
}

class IntHash: public std::hash<int> {
    size_t operator()(const int& s) const noexcept {
        return (size_t)s;
    }
};

TEST(HashMapTest, HashMapConf) {
    std::hash<int> h = IntHash();
    auto l = hash_map_linear_probing(1);
    auto map = hash_map_new<int, int>(10, 60, h, l);
    GTEST_ASSERT_TRUE(map);
    GTEST_ASSERT_TRUE(hash_map_insert(map, 1, 123));
    GTEST_ASSERT_TRUE(hash_map_insert(map, 11, 234));
    GTEST_ASSERT_EQ(map->map[2]->key, 11);
    GTEST_ASSERT_EQ(hash_map_get_entry(map, 11)->value, 234);
    free_hash_map(map);
    GTEST_ASSERT_FALSE(map);
    map = hash_map_new<int, int>(10, 60, h);
    GTEST_ASSERT_TRUE(map);
    GTEST_ASSERT_TRUE(hash_map_insert(map, 1, 123));
    GTEST_ASSERT_TRUE(hash_map_insert(map, 11, 234));
    GTEST_ASSERT_TRUE(hash_map_insert(map, 41, 255));
    GTEST_ASSERT_TRUE(hash_map_insert(map, 51, 188));
    GTEST_ASSERT_TRUE(hash_map_insert(map, 61, 133));
    GTEST_ASSERT_EQ(map->map[1]->key, 1);
    GTEST_ASSERT_EQ(map->map[2]->key, 11);
    GTEST_ASSERT_EQ(map->map[0]->key, 41);
    GTEST_ASSERT_EQ(map->map[5]->key, 51);
    GTEST_ASSERT_EQ(map->map[7]->key, 61);
    GTEST_ASSERT_EQ(hash_map_get_entry(map, 51)->value, 188);
    free_hash_map(map);
}

TEST(HashMapTest, HashMap2) {
    auto map = hash_map_new<int, int>();
    GTEST_ASSERT_TRUE(map);
    for (int i = 0; i < 100; i++) {
        hash_map_insert(map, i, i * i);
    }
    GTEST_ASSERT_EQ(map->count, 100);
    GTEST_ASSERT_EQ(map->cap, hashmap_primes[2]);
    GTEST_ASSERT_EQ(hash_map_get_entry(map, 15)->value, 225);
    free_hash_map(map);
}
