#include "hash_map.h"

size_t hash_map_get_next_cap(size_t cap) {
    if (cap >= hashmap_primes[25]) return cap * 2;
    for (auto i = 0; i < 26; i++) {
        if (hashmap_primes[i] > cap) return hashmap_primes[i];
    }
    return cap * 2;
}

std::function<size_t(size_t)> hash_map_linear_probing(size_t interval) {
    return std::function([interval](size_t i) {
        return i * interval;
    });
}

size_t hash_map_quadratic_probing(size_t i) {
    return i * i;
}

size_t hash_map_quadratic_probing_alter(size_t i) {
    size_t t = (i + 1) / 2;
    t = t * t;
    return i % 2 == 0 ? -t : t;
}

std::function<size_t(size_t)> hash_map_random_probing(uint64_t seed) {
    return HashMapRandomProbingGeneator(seed);
}
