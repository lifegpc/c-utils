#ifndef _UTIL_BINARY_TREE_H
#define _UTIL_BINARY_TREE_H
#include <functional>
#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include "linked_queue.h"

template <typename T>
struct BinaryTree {
    T data;
    struct BinaryTree* left;
    struct BinaryTree* right;
};

template <typename T>
void binary_tree_clear(struct BinaryTree<T>*& top, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!top) return;
    binary_tree_dfs(top, [&free_func](struct BinaryTree<T>* node) {
        if (free_func) {
            free_func(node->data);
        }
        node->left = nullptr;
        node->right = nullptr;
        free(node);
    });
    top = nullptr;
}

template <typename T, class F>
inline void binary_tree_clear(struct BinaryTree<T>*& top, F free_func) {
    binary_tree_clear(top, std::function<void(T)>(free_func));
}

template <typename T>
struct BinaryTree<T>* binary_tree_clone(struct BinaryTree<T>* tree, std::function<void(T)> free_func = std::function<void(T)>()) {
    if (!tree) return nullptr;
    T newdata(tree->data);
    struct BinaryTree<T>* now = binary_tree_new(newdata);
    if (now) {
        if (tree->left) {
            now->left = binary_tree_clone(tree->left, free_func);
            if (!now->left) goto end;
        }
        if (tree->right) {
            now->right = binary_tree_clone(tree->right, free_func);
            if (!now->right) goto end;
        }
    }
    return now;
end:
    binary_tree_clear(now, free_func);
    return now;
}

template <typename T, class F>
inline struct BinaryTree<T>* binary_tree_clone(struct BinaryTree<T>* tree, F free_func) {
    return binary_tree_clone(tree, std::function<void(T)>(free_func));
}

template <typename T, typename... Args>
void binary_tree_dfs(struct BinaryTree<T>* top, std::function<void(struct BinaryTree<T>*, Args...)> callback, Args... args) {
    if (!top) return;
    if (top->left) binary_tree_dfs(top->left, callback, args...);
    if (top->right) binary_tree_dfs(top->right, callback, args...);
    callback(top, args...);
}

template <typename T, class F, typename... Args>
inline void binary_tree_dfs(struct BinaryTree<T>* top, F callback, Args... args) {
    binary_tree_dfs(top, std::function<void(struct BinaryTree<T>*, Args...)>(callback), args...);
}

#define binary_tree_lrn binary_tree_dfs

template <typename T, typename... Args>
void binary_tree_lnr(struct BinaryTree<T>* top, std::function<void(struct BinaryTree<T>*, Args...)> callback, Args... args) {
    if (!top) return;
    if (top->left) binary_tree_lnr(top->left, callback, args...);
    callback(top, args...);
    if (top->right) binary_tree_lnr(top->right, callback, args...);
}

template <typename T, class F, typename... Args>
inline void binary_tree_lnr(struct BinaryTree<T>* top, F callback, Args... args) {
    binary_tree_lnr(top, std::function<void(struct BinaryTree<T>*, Args...)>(callback), args...);
}

template <typename T, typename... Args>
void binary_tree_nlr(struct BinaryTree<T>* top, std::function<void(struct BinaryTree<T>*, Args...)> callback, Args... args) {
    if (!top) return;
    callback(top, args...);
    if (top->left) binary_tree_nlr(top->left, callback, args...);
    if (top->right) binary_tree_nlr(top->right, callback, args...);
}

template <typename T, class F, typename... Args>
inline void binary_tree_nlr(struct BinaryTree<T>* top, F callback, Args... args) {
    binary_tree_nlr(top, std::function<void(struct BinaryTree<T>*, Args...)>(callback), args...);
}

template <typename T, typename... Args>
void binary_tree_bfs(struct BinaryTree<T>* top, std::function<void(struct BinaryTree<T>*, Args...)> callback, Args... args) {
    if (!top) return;
    struct LinkedQueue<struct BinaryTree<T>*> queue;
    linked_queue_init(queue);
    linked_queue_push(queue, top);
    struct BinaryTree<T>* tmp;
    while (linked_queue_pop(queue, tmp)) {
        if (tmp->left) linked_queue_push(queue, tmp->left);
        if (tmp->right) linked_queue_push(queue, tmp->right);
        callback(tmp, args...);
    }
}

template <typename T, class F, typename... Args>
inline void binary_tree_bfs(struct BinaryTree<T>* top, F callback, Args... args) {
    binary_tree_bfs(top, std::function<void(struct BinaryTree<T>*, Args...)>(callback), args...);
}

template <typename T>
struct BinaryTree<T>* binary_tree_new(T data) {
    struct BinaryTree<T>* node = (struct BinaryTree<T>*)malloc(sizeof(struct BinaryTree<T>));
    if (!node) return nullptr;
    node->data = data;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

template <typename T>
inline bool binary_tree_is_leaf(struct BinaryTree<T>* node) {
    return node->left == nullptr && node->right == nullptr;
}

template <typename T>
inline bool binary_tree_node_is_full(struct BinaryTree<T>* node) {
    return node->left != nullptr && node->right != nullptr;
}

#endif
