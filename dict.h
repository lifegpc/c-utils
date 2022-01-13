#ifndef _UTIL_DICT_H
#define _UTIL_DICT_H
#include "linked_list.h"
template <typename T, typename V>
struct dict_entry {
    T key;
    V value;
};
template <typename T, typename V>
struct Dict {
    struct dict_entry<T, V> d;
    struct Dict* prev;
    struct Dict* next;
};
template <typename T, typename V>
size_t dict_count(struct Dict<T, V>* d) {
    return linked_list_count((struct LinkedList<struct dict_entry<T, V>>*)d);
}
template <typename T, typename V>
void dict_free(struct Dict<T, V>*& d, void(*free_func)(struct dict_entry<T, V>) = nullptr) {
    return linked_list_clear((struct LinkedList<struct dict_entry<T, V>>*&)d, free_func);
}
template <typename K, typename V>
bool dict_get_internal(struct dict_entry<K, V> o, K key) {
    return o.key == key;
}
template <typename K, typename V>
struct dict_entry<K, V>* dict_get(struct Dict<K, V>* d, K key) {
    struct Dict<K, V>* re = (struct Dict<K, V>*)linked_list_get((struct LinkedList<struct dict_entry<K, V>>*)d, key, &dict_get_internal);
    if (!re) return nullptr;
    return &re->d;
}
template <typename K, typename V>
V dict_get_value(struct Dict<K, V>* d, K key) {
    struct dict_entry<K, V>* re = dict_get(d, key);
    if (!re) return nullptr;
    return re->value;
}
template <typename K, typename V>
bool dict_heve_key_internal(struct dict_entry<K, V> origin, K key) {
    return key == origin.key;
}
template <typename K, typename V>
bool dict_have_key(struct Dict<K, V>* d, K key) {
    return linked_list_have((struct LinkedList<struct dict_entry<K, V>>*)d, key, &dict_heve_key_internal);
}
template <typename K, typename V, typename ... Args>
void dict_iter(struct Dict<K, V>* d, void(*callback)(K key, V value, Args... args), Args... args) {
    if (!d || !callback) return;
    struct Dict<K, V>* t = d;
    callback(t->d.key, t->d.value, args...);
    while (t->next) {
        t = t->next;
        callback(t->d.key, t->d.value, args...);
    }
}
template <typename K, typename V>
bool dict_set(struct Dict<K, V>*& d, K key, V value, void(*free_func)(V) = nullptr) {
    if (!d) {
        struct dict_entry<K, V> v = { key, value };
        return linked_list_append((struct LinkedList<struct dict_entry<K, V>>*&)d, &v);
    }
    struct Dict<K, V>* t = d;
    if (t->d.key == key) {
        if (free_func) free_func(t->d.value);
        t->d.value = value;
        return true;
    }
    while (t->next) {
        t = t->next;
        if (t->d.key == key) {
            if (free_func) free_func(t->d.value);
            t->d.value = value;
            return true;
        }
    }
    struct dict_entry<K, V> v = { key, value };
    return linked_list_append((struct LinkedList<struct dict_entry<K, V>>*&)d, &v);
}
template <typename K, typename V>
bool dict_set(struct Dict<K, V>*& d, K key, V value, void(*value_copy_func)(V, V), void(*free_func)(V) = nullptr) {
    if (!value_copy_func) return false;
    if (!d) {
        struct dict_entry<K, V> v;
        v.key = key;
        value_copy_func(v.value, value);
        return linked_list_append((struct LinkedList<struct dict_entry<K, V>>*&)d, &v);
    }
    struct Dict<K, V>* t = d;
    if (t->d.key == key) {
        if (free_func) free_func(t->d.value);
        value_copy_func(t->d.value, value);
        return true;
    }
    while (t->next) {
        t = t->next;
        if (t->d.key == key) {
            if (free_func) free_func(t->d.value);
            value_copy_func(t->d.value, value);
            return true;
        }
    }
    struct dict_entry<K, V> v;
    v.key = key;
    value_copy_func(v.value, value);
    return linked_list_append((struct LinkedList<struct dict_entry<K, V>>*&)d, &v);
}
#endif
