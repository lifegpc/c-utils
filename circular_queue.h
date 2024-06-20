#ifndef _UTIL_CIRCULAR_QUEUE_H
#define _UTIL_CIRCULAR_QUEUE_H
#include <functional>
#include <malloc.h>
#include <stddef.h>

template <typename T>
struct CircularQueue {
    T* base;
    size_t front;
    size_t rear;
    size_t size;
};

template <typename T>
bool init_circular_queue(struct CircularQueue<T>& queue, size_t size) {
    if (size < 2) {
        return false;
    }
    queue.base = (T*)malloc(size * sizeof(T));
    if (!queue.base) {
        return false;
    }
    queue.front = 0;
    queue.rear = 0;
    queue.size = size;
    return true;
}

template <typename T>
void free_circular_queue(struct CircularQueue<T>& queue, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!queue.base) return;
    if (queue.front == queue.rear) return;
    if (free_func) {
        circular_queue_iter(queue, free_func);
    }
    free(queue.base);
    queue.base = nullptr;
    queue.front = 0;
    queue.rear = 0;
    queue.size = 0;
}

template <typename T, class F>
inline void free_circular_queue(struct CircularQueue<T>& queue, F free_func) {
    free_circular_queue(queue, std::function<void(T)>(free_func));
}

template <typename T>
void circular_queue_clear(struct CircularQueue<T>& queue, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!queue.base) return;
    if (free_func && queue.front != queue.rear) {
        circular_queue_iter(queue, free_func);
    }
    queue.front = 0;
    queue.rear = 0;
}

template <typename T, class F>
inline void circular_queue_clear(struct CircularQueue<T>& queue, F free_func) {
    circular_queue_clear(queue, std::function<void(T)>(free_func));
}

template <typename T>
bool circular_queue_is_empty(struct CircularQueue<T>& queue) {
    if (!queue.base) return true;
    return queue.front == queue.rear;
}

template <typename T>
bool circular_queue_is_full(struct CircularQueue<T>& queue) {
    if (!queue.base) return false;
    return (queue.rear + 1) % queue.size == queue.front;
}

template <typename T>
size_t circular_queue_length(struct CircularQueue<T>& queue) {
    if (!queue.base) return 0;
    return queue.rear >= queue.front ? queue.rear - queue.front : queue.rear + queue.size - queue.front;
}

template <typename T>
bool circular_queue_get_head(struct CircularQueue<T>& queue, T& head) {
    if (!queue.base || queue.rear == queue.front) return false;
    head = queue.base[queue.front];
    return true;
}

template <typename T>
bool circular_queue_get_back(struct CircularQueue<T>& queue, T& back) {
    if (!queue.base || queue.rear == queue.front) return false;
    size_t pos = queue.rear ? queue.rear - 1 : queue.size - 1;
    back = queue.base[pos];
    return true;
}

template <typename T>
bool circular_queue_push(struct CircularQueue<T>& queue, T data) {
    if (!queue.base) return false;
    if ((queue.rear + 1) % queue.size == queue.front) return false;
    queue.base[queue.rear++] = data;
    queue.rear %= queue.size;
    return true;
}

template <typename T>
bool circular_queue_pop(struct CircularQueue<T>& queue, T& data) {
    if (!queue.base) return false;
    if (queue.rear == queue.front) return false;
    data = queue.base[queue.front++];
    queue.front %= queue.size;
    return true;
}

template <typename T, typename ... Args>
void circular_queue_iter(struct CircularQueue<T>& queue, std::function<void(T, Args...)> callback, Args... args) {
    if (!queue.base || !callback) return;
    if (queue.front == queue.rear) return;
    size_t now = queue.front;
    while (now != queue.rear) {
        callback(queue.base[now], args...);
        now = (now + 1) % queue.size;
    }
}

template <typename T, class F, typename ... Args>
inline void circular_queue_iter(struct CircularQueue<T>& queue, F callback, Args... args) {
    circular_queue_iter(queue, std::function<void(T, Args...)>(callback), args...);
}

#endif
