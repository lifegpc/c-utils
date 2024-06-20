#ifndef _UTIL_STACK_H
#define _UTIL_STACK_H
#include <functional>
#include <stddef.h>
#include <malloc.h>

template <typename T>
struct Stack {
    T* base;
    T* top;
    size_t capacity;
    size_t inc_step;
};


/**
 * @brief Initailize stack and allocate memory for it.
 * @param stack Stack
 * @param capacity Default capacity
 */
template <typename T>
bool init_stack(struct Stack<T>& stack, size_t capacity = 1, size_t inc_step = 1) {
    if (inc_step < 1) inc_step = 1;
    if (capacity < 1) capacity = 1;
    stack.base = (T*)malloc(capacity * sizeof(T));
    if (!stack.base) return false;
    stack.top = stack.base;
    stack.capacity = capacity;
    stack.inc_step = inc_step;
    return true;
}

template <typename T>
void free_stack(struct Stack<T>& stack, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!stack.base) return;
    if (free_func) {
        stack_iter(stack, free_func);
    }
    free(stack.base);
    stack.base = nullptr;
    stack.top = nullptr;
    stack.capacity = 0;
    stack.inc_step = 0;
}

template <typename T, class F>
inline void free_stack(struct Stack<T>& stack, F free_func) {
    free_stack(stack, std::function<void(T)>(free_func));
}

template <typename T>
void clear_stack(struct Stack<T>& stack, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!stack.base) return;
    if (free_func) {
        stack_iter(stack, free_func);
    }
    stack.top = stack.base;
}

template <typename T, class F>
inline void clear_stack(struct Stack<T>& stack, F free_func) {
    clear_stack(stack, std::function<void(T)>(free_func));
}

template <typename T>
bool stack_is_empty(struct Stack<T>& stack) {
    if (!stack.base) return true;
    return stack.base == stack.top;
}

template <typename T>
size_t stack_length(struct Stack<T>& stack) {
    if (!stack.base) return 0;
    return stack.top - stack.base;
}

template <typename T>
bool stack_get_top(struct Stack<T>& stack, T& ele) {
    if (!stack.base) return false;
    if (stack.base == stack.top) return false;
    ele = *(stack.top - 1);
    return true;
}

template <typename T>
bool stack_push(struct Stack<T>& stack, T ele) {
    if (!stack.base) return false;
    if (stack.top - stack.base >= stack.capacity) {
        T* obase = stack.base;
        stack.base = (T*)realloc(stack.base, (stack.capacity + stack.inc_step) * sizeof(T));
        if (!stack.base) {
            /// Free original buffer
            free(obase);
            stack.top = nullptr;
            stack.capacity = 0;
            stack.inc_step = 0;
            return false;
        }
        stack.top = stack.base + stack.capacity;
        stack.capacity += stack.inc_step;
    }
    *(stack.top++) = ele;
    return true;
}

template <typename T>
bool stack_pop(struct Stack<T>& stack, T& ele) {
    if (!stack.base) return false;
    if (stack.top == stack.base) return false;
    ele = *(--stack.top);
    return true;
}

template <typename T, typename ... Args>
void stack_iter(struct Stack<T>& stack, std::function<void(T, Args...)> callback, Args... args) {
    if (!stack.base || !callback) return;
    if (stack.top == stack.base) return;
    T* now = stack.top;
    while (now > stack.base) {
        now -= 1;
        callback(*now, args...);
    }
}

template <typename T, class F, typename ... Args>
inline void stack_iter(struct Stack<T>& stack, F callback, Args... args) {
    stack_iter(stack, std::function<void(T, Args...)>(callback), args...);
}

#endif
