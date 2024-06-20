#ifndef _UTIL_LINKED_STACK_H
#define _UTIL_LINKED_STACK_H
#include <functional>
#include <stddef.h>
#include <malloc.h>

template <typename T>
struct LinkedStack {
    T d;
    struct LinkedStack* next;
};

template <typename T>
size_t linked_stack_length(struct LinkedStack<T>* top) {
    if (!top) return 0;
    struct LinkedStack<T>* now = top;
    size_t i = 1;
    while (now->next) {
        now = now->next;
        i += 1;
    }
    return i;
}

template <typename T>
void linked_stack_clear(struct LinkedStack<T>*& top, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!top) return;
    struct LinkedStack<T>* tmp;
    do {
        if (free_func) {
            free_func(top->d);
        }
        tmp = top;
        top = top->next;
        free(tmp);
    } while (top);
}

template <typename T, class F>
inline void linked_stack_clear(struct LinkedStack<T>*& top, F free_func) {
    linked_stack_clear(top, std::function<void(T)>(free_func));
}

template <typename T>
bool linked_stack_push(struct LinkedStack<T>*& top, T ele) {
    struct LinkedStack<T>* t = (struct LinkedStack<T>*)malloc(sizeof(struct LinkedStack<T>));
    if (!t) {
        return false;
    }
    t->d = ele;
    t->next = top;
    top = t;
    return true;
}

template <typename T>
bool linked_stack_pop(struct LinkedStack<T>*& top, T& ele) {
    if (!top) return false;
    ele = top->d;
    struct LinkedStack<T>* tmp = top;
    top = top->next;
    free(tmp);
    return true;
}

template <typename T, typename ... Args>
void linked_stack_iter(struct LinkedStack<T>* top, std::function<void(T, Args...)> callback, Args... args) {
    if (!top || !callback) return;
    struct LinkedStack<T>* now = top;
    do {
        callback(now->d, args...);
        now = now->next;
    } while (now);
}

template <typename T, class F, typename ... Args>
inline void linked_stack_iter(struct LinkedStack<T>* top, F callback, Args... args) {
    linked_stack_iter(top, std::function<void(T, Args...)>(callback), args...);
}

#endif
