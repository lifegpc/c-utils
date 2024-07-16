#ifndef _UTIL_BINARY_SEARCH_TREE_H
#define _UTIL_BINARY_SEARCH_TREE_H
#include <exception>
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

template <typename K, typename V>
bool binary_search_tree_delete(BinarySearchTree<K, V>*& root, K key, V* deleted_value, std::function<int(K, K)> comp, std::function<void(K)> free_key = std::function<void(K)>(), std::function<void(V)> free_value = std::function<void(V)>()) {
    if (!root) return false;
    BinarySearchTree<K, V>* cur = root, *prev = nullptr;
    while (cur) {
        int re = comp(key, cur->data.key);
        if (re == 0) {
            break;
        }
        prev = cur;
        cur = re < 0 ? cur->left : cur->right;
    }
    if (!cur) return false;
    if (binary_tree_is_leaf(cur)) {
        if (prev) prev->left == cur ? prev->left = nullptr : prev->right = nullptr;
        else root = nullptr;
    } else if (!cur->right) {
        if (prev) prev->left == cur ? prev->left = cur->left : prev->right = cur->left;
        else root = cur->left;
    } else if (!cur->left) {
        if (prev) prev->left == cur ? prev->left = cur->right : prev->right = cur->right;
        else root = cur->right;
    } else {
        prev = cur;
        BinarySearchTree<K, V>* tmp = cur->left;
        while (tmp->right) {
            prev = tmp;
            tmp = tmp->right;
        }
        K tk = tmp->data.key;
        V tv = tmp->data.value;
        tmp->data.key = cur->data.key;
        tmp->data.value = cur->data.value;
        cur->data.key = tk;
        cur->data.value = tv;
        cur = tmp;
        prev->left == cur ? prev->left = nullptr : prev->right = nullptr;
    }
    if (free_key) free_key(cur->data.key);
    if (deleted_value) *deleted_value = cur->data.value;
    if (!deleted_value && free_value) free_value(cur->data.value);
    free(cur);
    return true;
}

template <typename K, typename V, class F, class G, class H>
inline bool binary_search_tree_delete(BinarySearchTree<K, V>*& root, K key, V* deleted_value, F comp, G free_key, H free_value) {
    return binary_search_tree_delete(root, key, deleted_value, std::function<int(K, K)>(comp), std::function<void(K)>(free_key), std::function<void(V)>(free_value));
}

template <typename K, typename V>
bool binary_search_tree_delete(BinarySearchTree<K, V>*& root, K key, V* deleted_value = nullptr) {
    return binary_search_tree_delete(root, key, deleted_value, [](K k1, K k2) {
        return k1 == k2 ? 0 : k1 < k2 ? -1 : 1;
    }, nullptr, nullptr);
}

template <typename K, typename V>
bool binary_search_tree_get(BinarySearchTree<K, V>* root, K key, V& value, std::function<int(K, K)> comp) {
    if (!root) return false;
    BinarySearchTree<K, V>* cur = root;
    while (cur) {
        int re = comp(key, cur->data.key);
        if (re == 0) {
            value = cur->data.value;
            return true;
        }
        cur = re < 0 ? cur->left : cur->right;
    }
    return false;
}

template <typename K, typename V, class F>
inline bool binary_search_tree_get(BinarySearchTree<K, V>* root, K key, V& value, F comp) {
    return binary_search_tree_get(root, key, value, std::function<int(K, K)>(comp));
}

template <typename K, typename V>
bool binary_search_tree_get(BinarySearchTree<K, V>* root, K key, V& value) {
    return binary_search_tree_get(root, key, value, [](K k1, K k2) {
        return k1 == k2 ? 0 : k1 < k2 ? -1 : 1;
    });
}

template <typename K, typename V>
BinarySearchTree<K, V>* binary_search_tree_get_node(BinarySearchTree<K, V>* root, K key, std::function<int(K, K)> comp) {
    if (!root) return nullptr;
    BinarySearchTree<K, V>* cur = root;
    while (cur) {
        int re = comp(key, cur->data.key);
        if (re == 0) {
            return cur;
        }
        cur = re < 0 ? cur->left : cur->right;
    }
    return nullptr;
}

template <typename K, typename V, class F>
inline BinarySearchTree<K, V>* binary_search_tree_get_node(BinarySearchTree<K, V>* root, K key, F comp) {
    return binary_search_tree_get_node(root, key, std::function<int(K, K)>(comp));
}

template <typename K, typename V>
BinarySearchTree<K, V>* binary_search_tree_get_node(BinarySearchTree<K, V>* root, K key) {
    return binary_search_tree_get_node(root, key, [](K k1, K k2) {
        return k1 == k2 ? 0 : k1 < k2 ? -1 : 1;
    });
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
BinarySearchTree<K, V>* binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value, std::function<int(K, K)> comp, std::function<void(K)> free_key = std::function<void(K)>(), std::function<void(V)> free_value = std::function<void(V)>(), bool noupdate = false) {
    if (!root) {
        root = binary_tree_new<BinarySearchTreePair<K, V>>({ key, value });
        if (!root) {
            if (free_key) free_key(key);
            if (free_value) free_value(value);
        }
        return root;
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
        if (free_value && !noupdate) free_value(cur->data.value);
        if (free_key) free_key(key);
        if (!noupdate) cur->data.value = value;
        return cur;
    }
    BinarySearchTree<K, V>* node = binary_tree_new<BinarySearchTreePair<K, V>>({key, value});
    if (!node) {
        if (free_key) free_key(key);
        if (free_value) free_value(value);
        return nullptr;
    }
    if (re == -1) {
        cur->left = node;
    } else {
        cur->right = node;
    }
    return node;
}

template <typename K, typename V, class F>
inline BinarySearchTree<K, V>* binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value, F comp) {
    return binary_search_tree_insert(root, key, value, std::function<int(K, K)>(comp));
}

template <typename K, typename V>
BinarySearchTree<K, V>* binary_search_tree_insert(BinarySearchTree<K, V>*& root, K key, V value) {
    return binary_search_tree_insert(root, key, value, [](K k1, K k2) {
        return k1 == k2 ? 0 : k1 < k2 ? -1 : 1;
    });
}

template <typename K, typename V, class Compare = std::less<K>>
class BinarySearchMap {
private:
    BinarySearchTree<K, V>* tree = nullptr;
    std::function<int(K, V)> comp_func;
    std::function<void(K)> free_key_func;
    std::function<void(V)> free_value_func;
public:
    BinarySearchMap(std::function<void(K)> free_key = std::function<void(K)>(), std::function<void(V)> free_value = std::function<void(V)>(), Compare cmp = Compare()) {
        comp_func = std::function([&cmp] (K k1, K k2) {
            return cmp(k1, k2) ? -1 : cmp(k2, k1) ? 1 : 0;
        });
        free_key_func = free_key;
        free_value_func = free_value;
    }
    template <class F, class G>
    BinarySearchMap(F free_key, G free_value = std::function<void(V)>(), Compare cmp = Compare()) : BinarySearchMap(std::function<void(K)>(free_key), std::function<void(V)>(free_value), cmp) {}
    BinarySearchMap(const BinarySearchMap& other) {
        tree = binary_tree_clone(other.tree, [&other](BinarySearchTreePair<K, V> e) {
            if (other.free_key_func) other.free_key_func(e.key);
            if (other.free_value_func) other.free_value_func(e.value);
        });
        if (!tree) {
            throw std::runtime_error("Failed to clone map.");
        }
        comp_func = other.comp_func;
        free_key_func = other.free_key_func;
        free_value_func = other.free_value_func;
    }
    ~BinarySearchMap() {
        clear();
    }
    void inline clear() {
        binary_search_tree_clear(tree, free_key_func, free_value_func);
    }
    bool inline del(K key, V* deleted_value = nullptr) {
        return binary_search_tree_delete(tree, key, deleted_value, comp_func, free_key_func, free_value_func);
    }
    bool inline get(K key, V& value) const {
        return binary_search_tree_get(tree, key, value, comp_func);
    }
    bool inline get(K key, V& value) {
        return binary_search_tree_get(tree, key, value, comp_func);
    }
    bool inline insert(K key, V value) {
        return binary_search_tree_insert(tree, key, value, comp_func, free_key_func, free_value_func) != nullptr;
    }
    template <typename ...Args>
    void inline iter(std::function<void(K, V, Args...)> callback, Args... args) {
        return binary_search_tree_iter(tree, callback, args...);
    }
    template <class F, typename ...Args>
    void inline iter(F callback, Args... args) {
        return binary_search_tree_iter(tree, std::function<void(K, V, Args...)>(callback), args...);
    }
    const V& operator[](K key) const {
        BinarySearchTree<K, V>* node = binary_search_tree_get_node(tree, key, comp_func);
        if (!node) {
            throw std::runtime_error("No such node.");
        }
        return node->data.value;
    }
    V& operator[](K key) {
        V tmp = V();
        BinarySearchTree<K, V>* node = binary_search_tree_insert(tree, key, tmp, comp_func, free_key_func, free_value_func, true);
        if (!node) {
            throw std::runtime_error("Failed to insert new node.");
        }
        return node->data.value;
    }
};

#endif
