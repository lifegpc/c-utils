#ifndef _UTIL_LINKED_LIST_H
#define _UTIL_LINKED_LIST_H
#include <list>
#include <malloc.h>
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
#endif
