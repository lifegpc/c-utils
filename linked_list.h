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
    if (data) tmp->d = *data; else tmp->d = T();
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
    if (data) tmp->d = *data; else tmp->d = T();
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
bool linked_list_delete(struct LinkedList<T>*& list, T data) {
    if (!list) return false;
    struct LinkedList<T>* t = list;
    while (t->prev) {
        t = t->prev;
    }
    if (t->d == data) {
        if (t->prev) t->prev->next = t->next;
        if (t->next) t->next->prev = t->prev;
        if (t == list) {
            if (t->prev) {
                list = t->prev;
            } else if (t->next) {
                list = t->next;
            } else {
                list = nullptr;
            }
        }
        free(t);
        return true;
    }
    while (t->next) {
        t = t->next;
        if (t->d == data) {
            if (t->prev) t->prev->next = t->next;
            if (t->next) t->next->prev = t->prev;
            if (t == list) {
                if (t->prev) {
                    list = t->prev;
                } else if (t->next) {
                    list = t->next;
                } else {
                    list = nullptr;
                }
            }
            free(t);
            return true;
        }
    }
    return false;
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

template <typename T, typename D, typename ... Args>
struct LinkedList<T>* linked_list_get(struct LinkedList<T>* list, D data, bool(*compare_func)(T, D, Args...), Args ... args) {
    if (!list || !compare_func) return nullptr;
    struct LinkedList<T>* t = list;
    while (t->prev) {
        t = t->prev;
    }
    if (compare_func(t->d, data, args...)) return t;
    while (t->next) {
        t = t->next;
        if (compare_func(t->d, data, args...)) return t;
    }
    return nullptr;
}

template <typename T>
struct LinkedList<T>* linked_list_get(struct LinkedList<T>* list, size_t index) {
    if (!list) return nullptr;
    struct LinkedList<T>* t = list;
    while (t->prev) {
        t = t->prev;
    }
    size_t n = 0;
    if (index == n) return t;
    while (t->next) {
        t = t->next;
        n++;
        if (index == n) return t;
    }
    return nullptr;
}

template <typename T>
bool linked_list_have(struct LinkedList<T>* list, T data) {
    if (!list) return false;
    struct LinkedList<T>* t = list;
    if (t->d == data) return true;
    while (t->next) {
        t = t->next;
        if (t->d == data) return true;
    }
    return false;
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

template <typename T, typename D, typename ... Args>
bool linked_list_have(struct LinkedList<T>* list, D data, bool(*compare_func)(T, D, Args...), Args... args) {
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
struct LinkedList<T>* linked_list_head(struct LinkedList<T>* list) {
    if (!list) return nullptr;
    struct LinkedList<T>* t = list;
    while (t->prev) {
        t = t->prev;
    }
    return t;
}

template <typename T, typename ... Args>
void linked_list_iter(struct LinkedList<T>* list, void(*callback)(size_t index, T data, Args... args), Args... args) {
    if (!list || !callback) return;
    size_t i = 0;
    struct LinkedList<T>* t = list;
    callback(i, t->d, args...);
    while (t->next) {
        t = t->next;
        i++;
        callback(i, t->d, args...);
    }
}

template <typename T, typename ... Args>
void linked_list_iter(struct LinkedList<T>* list, void(*callback)(T data, Args... args), Args... args) {
    if (!list || !callback) return;
    struct LinkedList<T>* t = list;
    callback(t->d, args...);
    while (t->next) {
        t = t->next;
        callback(t->d, args...);
    }
}

template <typename T, typename D, typename ... Args>
D linked_list_iter(struct LinkedList<T>* list, D(*callback)(T data, Args... args), D failed, Args... args) {
    if (!list || !callback) return failed;
    struct LinkedList<T>* t = list;
    D re = callback(t->d, args...);
    if (re == failed) return re;
    while (t->next) {
        t = t->next;
        re = callback(t->d, args...);
        if (re == failed) return re;
    }
    return re;
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
