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
