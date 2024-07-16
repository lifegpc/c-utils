#ifndef _UTIL_BINARY_SEARCH_TREE_H
#define _UTIL_BINARY_SEARCH_TREE_H
#include "binary_tree.h"

template <typename K, typename V>
struct BinarySearchTreePair {
    K key;
    V value;
};

template <typename K, typename V>
using BinarySearchTree = BinaryTree<BinarySearchTreePair<K, V>>;

template <typename K, typename V>
void binary_search_tree_clear(BinarySearchTree<K, V>*& root, std::function<void(K)> free_key = std::function<void(K)>(), std::function<void(V)> free_value = std::function<void(V)>()) {
    if (free_key || free_value) {
        binary_tree_clear(root, [&free_key, &free_value](BinarySearchTreePair<K, V> e) {
            if (free_key) free_key(e.key);
            if (free_value) free_value(e.value);
        });
    } else {
        binary_tree_clear(root);
    }
}

template <typename K, typename V, class F, class G>
inline void binary_search_tree_clear(BinarySearchTree<K, V>*& root, F free_key, G free_value = nullptr) {
    binary_search_tree_clear(root, std::function<void(K)>(free_key), std::function<void(V)>(free_value));
}

template <typename K, typename V, typename... Args>
void binary_search_tree_iter(BinarySearchTree<K, V>* root, std::function<void(K, V, Args...)> callback, Args... args) {
    binary_tree_lnr(root, [&callback](BinarySearchTree<K, V>* e, Args... args) {
        callback(e->data.key, e->data.value, args...);
    }, args...);
}

template <typename K, typename V, class F, typename... Args>
inline void binary_search_tree_iter(BinarySearchTree<K, V>* root, F callback, Args... args) {
    binary_search_tree_iter(root, std::function<void(K, V, Args...)>(callback), args...);
}

template <typename K, typename V>
bool binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value, std::function<int(K, K)> comp, std::function<void(K)> free_key = std::function<void(K)>(), std::function<void(V)> free_value = std::function<void(V)>()) {
    if (!root) {
        root = binary_tree_new<BinarySearchTreePair<K, V>>({ key, value });
        if (!root) {
            if (free_key) free_key(key);
            if (free_value) free_value(value);
        }
        return root != nullptr;
    }
    BinarySearchTree<K, V>* cur = root;
    int re = comp(key, cur->data.key);
    while (!binary_tree_is_leaf(cur)) {
        if (re == 0) {
            break;
        }
        if (re < 0 && !cur->left) break;
        if (re > 0 && !cur->right) break;
        cur = re < 0 ? cur->left : cur->right;
        re = comp(key, cur->data.key);
    }
    if (re == 0) {
        if (free_value) free_value(cur->data.value);
        if (free_key) free_key(key);
        cur->data.value = value;
        return true;
    }
    BinarySearchTree<K, V>* node = binary_tree_new<BinarySearchTreePair<K, V>>({key, value});
    if (!node) {
        if (free_key) free_key(key);
        if (free_value) free_value(value);
        return false;
    }
    if (re == -1) {
        cur->left = node;
    } else {
        cur->right = node;
    }
    return true;
}

template <typename K, typename V, class F>
inline bool binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value, F comp) {
    return binary_search_tree_insert(root, key, value, std::function<int(K, K)>(comp));
}

template <typename K, typename V>
bool binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value) {
    return binary_search_tree_insert(root, key, value, [](K k1, K k2) {
        return k1 == k2 ? 0 : k1 < k2 ? -1 : 1;
    });
}

#endif
