#ifndef _UTIL_HASH_MAP_H
#define _UTIL_HASH_MAP_H
#include <functional>
#include <random>
#include <stdint.h>
#include <string.h>
#include "utils_static.h"

template <typename K, typename V>
struct hash_map_entry {
    K key;
    V value;
};

size_t hash_map_get_next_cap(size_t cap);
std::function<size_t(size_t)> hash_map_linear_probing(size_t interval);
size_t hash_map_quadratic_probing(size_t i);
size_t hash_map_quadratic_probing_alter(size_t i);

class HashMapRandomProbingGeneator: public std::function<size_t(size_t)> {
    std::mt19937_64 gen;
    std::vector<size_t> cache;
public:
    HashMapRandomProbingGeneator(uint64_t seed): std::function<size_t(size_t)>([&](size_t i) {
        while (cache.size() < i) {
            cache.push_back(gen());
        }
        return cache[i - 1];
    }) {
        gen = std::mt19937_64(seed);
    }
};

std::function<size_t(size_t)> hash_map_random_probing(uint64_t seed);

template <typename K, typename V>
struct hash_map {
    std::function<void*(size_t)> malloc;
    std::function<void*(void *, size_t)> realloc;
    std::function<void(void *)> free;
    size_t cap;
    size_t count;
    size_t growat;
    uint8_t loadfactor;
    struct hash_map_entry<K, V>** map;
    std::function<size_t(size_t)> probing;
    std::hash<K> hash;
    std::function<void(K)> free_key;
    std::function<void(V)> free_value;
};

template <typename K, typename V, class H = std::hash<K>>
struct hash_map<K, V>* hash_map_new(
    size_t cap = hashmap_primes[0],
    uint8_t loadfactor = 60,
    H hash = H(),
    std::function<size_t(size_t)> probing = std::function<size_t(size_t)>(hash_map_quadratic_probing_alter),
    std::function<void*(size_t)> malloc = std::function<void*(size_t)>(malloc),
    std::function<void*(void *, size_t)> realloc = std::function<void*(void *, size_t)>(realloc),
    std::function<void(void *)> free = std::function<void(void *)>(free),
    std::function<void(K)> free_key = std::function<void(K)>(),
    std::function<void(V)> free_value = std::function<void(V)>()
) {
    if (!cap) cap = hash_map_get_next_cap(cap);
    struct hash_map<K, V>* map = new (struct hash_map<K, V>)();
    if (!map) return nullptr;
    map->malloc = malloc;
    map->realloc = realloc;
    map->free = free;
    map->cap = cap;
    map->count = 0;
    hash_map_set_loadfactor(map, loadfactor);
    map->probing = probing;
    map->hash = hash;
    map->free_key = free_key;
    map->free_value = free_value;
    size_t mapsize = sizeof(void*) * map->cap;
    map->map = (struct hash_map_entry<K, V>**)map->malloc(mapsize);
    if (!map->map) {
        delete map;
        return nullptr;
    }
    memset(map->map, 0, mapsize);
    return map;
}

template <typename K, typename V>
void free_hash_map(struct hash_map<K, V>*& map) {
    hash_map_clear(map, false);
    map->free(map->map);
    delete map;
    map = nullptr;
}

template <typename K, typename V>
void hash_map_clear(struct hash_map<K, V>* map, bool shrink = true) {
    if (!map) return;
    for (size_t i = 0; i < map->cap; i++) {
        auto m = map->map[i];
        if (m) {
            if (map->free_key) map->free_key(m->key);
            if (map->free_value) map->free_value(m->value);
            delete map->map[i];
            map->map[i] = nullptr;
        }
    }
    map->count = 0;
    if (shrink && map->cap > hashmap_primes[0]) {
        hash_map_set_cap(map, hashmap_primes[0]);
    }
}

template <typename K, typename V>
struct hash_map_entry<K, V>* hash_map_get_entry(struct hash_map<K, V>* map, K key) {
    if (!map) return nullptr;
    size_t h = map->hash(key);
    size_t loc = h % map->cap;
    size_t i = 1;
    while (map->map[loc] && map->map[loc]->key != key) {
        loc = (h + map->probing(i++)) % map->cap;
    }
    return map->map[loc];
}

template <typename K, typename V, class X>
inline struct hash_map_entry<K, V>* hash_map_get_entry(struct hash_map<K, V>* map, X key) {
    return hash_map_get_entry(map, K(key));
}

template <typename K, typename V>
bool hash_map_get(struct hash_map<K, V>* map, K key, V& value) {
    if (!map) return false;
    size_t h = map->hash(key);
    size_t loc = h % map->cap;
    size_t i = 1;
    while (map->map[loc] && map->map[loc]->key != key) {
        loc = (h + map->probing(i++)) % map->cap;
    }
    value = map->map[loc]->value;
    return true;
}

template <typename K, typename V, class X>
inline bool hash_map_get(struct hash_map<K, V>* map, X key, V& value) {
    return hash_map_get(map, K(key), value);
}

template <typename K, typename V>
struct hash_map_entry<K, V>* hash_map_insert_entry(struct hash_map<K, V>* map, struct hash_map_entry<K, V>* entry) {
    if (!map || !entry) return nullptr;
    size_t h = map->hash(entry->key);
    size_t loc = h % map->cap;
    size_t i = 1;
    while (map->map[loc] && map->map[loc]->key != entry->key) {
        loc = (h + map->probing(i++)) % map->cap;
    }
    auto t = map->map[loc];
    map->map[loc] = entry;
    map->count += t ? 0 : 1;
    return t;
}

template <typename K, typename V>
struct hash_map_entry<K, V>* hash_map_insert(struct hash_map<K, V>* map, K key, V value) {
    if (!map) return nullptr;
    if (map->count >= map->growat) {
        if (!hash_map_resize(map, hash_map_get_next_cap(map->cap))) {
            return nullptr;
        }
    }
    size_t h = map->hash(key);
    size_t loc = h % map->cap;
    size_t i = 1;
    while (map->map[loc] && map->map[loc]->key != key) {
        loc = (h + map->probing(i++)) % map->cap;
    }
    if (map->map[loc]) {
        if (map->free_value) map->free_value(map->map[loc]->value);
        if (map->free_key) map->free_key(key);
        map->map[loc]->value = value;
        return map->map[loc];
    }
    struct hash_map_entry<K, V>* ent = new (struct hash_map_entry<K, V>)({key, value});
    if (!ent) return nullptr;
    map->map[loc] = ent;
    map->count += 1;
    return map->map[loc];
}

template <typename K, typename V, class X, class Y>
inline struct hash_map_entry<K, V>* hash_map_insert(struct hash_map<K, V>* map, X key, Y value) {
    return hash_map_insert(map, K(key), V(value));
}

template <typename K, typename V>
bool hash_map_resize(struct hash_map<K, V>* map, size_t newcap) {
    if (!map && !newcap) return false;
    if (map->count > newcap) return false;
    if (map->cap == newcap) return true;
    size_t mapsize = sizeof(void *) * newcap;
    auto t = (struct hash_map_entry<K, V>**)map->malloc(mapsize);
    if (!t) return false;
    memset(t, 0, mapsize);
    auto ori = map->map;
    auto oricap = map->cap;
    map->cap = newcap;
    map->growat = map->cap * map->loadfactor / 100;
    map->map = t;
    map->count = 0;
    for (size_t i = 0; i < oricap; i++) {
        if (ori[i]) {
            hash_map_insert_entry(map, ori[i]);
        }
    }
    map->free(ori);
    return true;
}

template <typename K, typename V>
bool hash_map_set_cap(struct hash_map<K, V>* map, size_t newcap) {
    if (!map || !newcap) return false;
    auto t = (struct hash_map_entry<K, V>**)map->realloc(map->map, sizeof(void *) * newcap);
    if (t) {
        map->map = t;
        map->cap = newcap;
        map->growat = map->cap * map->loadfactor / 100;
        return true;
    }
    return false;
}

template <typename K, typename V>
void hash_map_set_loadfactor(struct hash_map<K, V>* map, uint8_t loadfactor) {
    if (!map) return;
    if (loadfactor > 100) loadfactor = 100;
    map->loadfactor = loadfactor;
    map->growat = map->cap * loadfactor / 100;
}

#endif
