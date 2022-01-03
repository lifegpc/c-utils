#ifndef _UTIL_LINKED_LIST_H
#define _UTIL_LINKED_LIST_H
#include <list>
#include <malloc.h>
#include <stddef.h>
#include <string.h>
template <typename T>
struct LinkedList {
    T d;
    struct LinkedList* prev;
    struct LinkedList* next;
};

template <typename T>
bool linked_list_append(struct LinkedList<T>*& list, T* data = nullptr, struct LinkedList<T>** tail = nullptr) {
    bool have_list = list;
    struct LinkedList<T>* tmp = (struct LinkedList<T>*)malloc(sizeof(LinkedList<T>));
    if (!tmp) return false;
    memset(tmp, 0, sizeof(LinkedList<T>));
    if (data) tmp->d = T(*data); else tmp->d = T();
    if (!have_list) {
        list = tmp;
    } else {
        struct LinkedList<T>* t = list;
        while (t->next) t = t->next;
        t->next = tmp;
        tmp->prev = t;
    }
    if (tail) *tail = tmp;
    return true;
}

template <typename T>
bool linked_list_append(struct LinkedList<T>*& list, struct LinkedList<T>** tail) {
    return linked_list_append(list, (T*)nullptr, tail);
}

template <typename T>
bool linked_list_append_head(struct LinkedList<T>*& list, T* data = nullptr) {
    bool have_list = list;
    struct LinkedList<T>* tmp = (struct LinkedList<T>*)malloc(sizeof(LinkedList<T>));
    if (!tmp) return false;
    memset(tmp, 0, sizeof(LinkedList<T>));
    if (data) tmp->d = T(*data); else tmp->d = T();
    if (have_list) {
        tmp->next = list;
        list->prev = tmp;
    }
    list = tmp;
    return true;
}

template <typename T, typename ... Args>
bool linked_list_append_list(struct LinkedList<T>*& list, struct LinkedList<T>* list2, bool(*compare_func)(T, T, Args...) = nullptr, Args... args) {
    if (!list2) return true;
    struct LinkedList<T>* t = list2;
    struct LinkedList<T>* tail = linked_list_tail(list);
    if (!compare_func || !linked_list_have(list, t->d, compare_func, args...)) {
        if (!linked_list_append<T>(list, &t->d)) return false;
    }
    while (t->next) {
        t = t->next;
        if (!compare_func || !linked_list_have(list, t->d, compare_func, args...)) {
            if (!linked_list_append<T>(list, &t->d)) {
                if (tail == nullptr) {
                    linked_list_clear(list);
                } else {
                    if (tail->next) linked_list_clear(tail->next);
                }
                return false;
            }
        }
    }
    return true;
}

template <typename T>
void linked_list_clear(struct LinkedList<T>*& list, void(*free_func)(T) = nullptr) {
    if (!list) return;
    struct LinkedList<T>* t = list;
    struct LinkedList<T>* tmp = nullptr;
    if (free_func) free_func(t->d);
    while (t->next) {
        tmp = t;
        t = t->next;
        free(tmp);
        if (free_func) free_func(t->d);
    }
    free(t);
    list = nullptr;
}

template <typename T>
size_t linked_list_count(struct LinkedList<T>* list) {
    if (!list) return 0;
    struct LinkedList<T>* t = list;
    size_t c = 1;
    while (t->next) {
        t = t->next;
        c += 1;
    }
    if (list->prev) {
        t = t->prev;
        c += 1;
        while (t->prev) {
            t = t->prev;
            c += 1;
        }
    }
    return c;
}

template <typename T>
void linked_list_free_tail(struct LinkedList<T>*& list, void(*free_func)(T) = nullptr) {
    if (!list) return;
    struct LinkedList<T>* t = linked_list_tail(list);
    if (t == list) {
        linked_list_clear(list, free_func);
    } else {
        if (free_func) free_func(t->d);
        if (t->prev) {
            t->prev->next = nullptr;
        }
        free(t);
    }
}

template <typename T, typename ... Args>
bool linked_list_have(struct LinkedList<T>* list, T data, bool(*compare_func)(T, T, Args...), Args... args) {
    if (!list || !compare_func) return false;
    struct LinkedList<T>* t = list;
    if (compare_func(t->d, data, args...)) return true;
    while (t->next) {
        t = t->next;
        if (compare_func(t->d, data, args...)) return true;
    }
    return false;
}

template <typename T>
void linked_list_remove(struct LinkedList<T>*& node, void(*free_func)(T) = nullptr) {
    if (!node) return;
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (free_func) free_func(node->d);
    free(node);
}

template <typename T>
struct LinkedList<T>* linked_list_tail(struct LinkedList<T>* list) {
    if (!list) return nullptr;
    struct LinkedList<T>* t = list;
    while (t->next) {
        t = t->next;
    }
    return t;
}
#endif
