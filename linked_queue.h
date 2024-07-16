#ifndef _UTIL_LINKED_QUEUE_H
#define _UTIL_LINKED_QUEUE_H
#include <functional>
#include <stddef.h>
#include <malloc.h>

template <typename T>
struct LinkedQueueNode {
    T d;
    struct LinkedQueueNode* next;
};

template <typename T>
struct LinkedQueue {
    struct LinkedQueueNode<T>* front;
    struct LinkedQueueNode<T>* rear;
};

template <typename T>
void linked_queue_init(struct LinkedQueue<T>& queue) {
    queue.front = nullptr;
    queue.rear = nullptr;
}

template <typename T>
size_t linked_queue_length(struct LinkedQueue<T>& queue) {
    if (!queue.front) return 0;
    struct LinkedQueueNode<T>* node = queue.front;
    size_t i = 1;
    while (node->next) {
        node = node->next;
        i++;
    }
    return i;
}

template <typename T>
void free_linked_queue(struct LinkedQueue<T>& queue, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!queue.front) return;
    struct LinkedQueueNode<T>* node = queue.front, *tmp = queue.front;
    do {
        tmp = node;
        node = node->next;
        if (free_func) {
            free_func(tmp->d);
        }
        free(tmp);
    } while (node);
    queue.front = nullptr;
    queue.rear = nullptr;
}

template <typename T, class F>
inline void free_linked_queue(struct LinkedQueue<T>& queue, F free_func) {
    free_linked_queue(queue, std::function<void(T)>(free_func));
}

template <typename T>
bool linked_queue_push(struct LinkedQueue<T>& queue, T data) {
    struct LinkedQueueNode<T>* node = (struct LinkedQueueNode<T>*)malloc(sizeof(struct LinkedQueueNode<T>));
    if (!node) {
        return false;
    }
    node->d = data;
    node->next = nullptr;
    if (queue.rear) {
        queue.rear->next = node;
    } else {
        queue.front = node;
    }
    queue.rear = node;
    return true;
}

template <typename T>
bool linked_queue_pop(struct LinkedQueue<T>& queue, T& data) {
    if (!queue.front) return false;
    data = queue.front->d;
    struct LinkedQueueNode<T>* tmp = queue.front;
    if (!tmp->next) {
        queue.rear = nullptr;
    }
    queue.front = tmp->next;
    free(tmp);
    return true;
}

template <typename T, typename ...Args>
void linked_queue_iter(struct LinkedQueue<T>& queue, std::function<void(T, Args...)> callback, Args... args) {
    if (!queue.front || !callback) return;
    struct LinkedQueueNode<T>* node = queue.front;
    do {
        callback(node->d, args...);
        node = node->next;
    } while (node);
}

template <typename T, class F, typename ...Args>
inline void linked_queue_iter(struct LinkedQueue<T>& queue, F callback, Args... args) {
    linked_queue_iter(queue, std::function<void(T, Args...)>(callback), args...);
}

#endif
