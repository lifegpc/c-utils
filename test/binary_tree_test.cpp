#include "gtest/gtest.h"
#include "binary_search_tree.h"
#include <string>

TEST(BinaryTreeTest, BinaryTree1) {
    struct BinaryTree<int>* top = binary_tree_new(1);
    top->left = binary_tree_new(2);
    top->right = binary_tree_new(3);
    top->left->left = binary_tree_new(4);
    top->left->right = binary_tree_new(5);
    top->right->right = binary_tree_new(6);
    std::string text;
    binary_tree_dfs(top, [&text](struct BinaryTree<int>* ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele->data);
    });
    GTEST_ASSERT_EQ(text, "4,5,2,6,3,1");
    text = "";
    binary_tree_lnr(top, [&text](struct BinaryTree<int>* ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele->data);
    });
    GTEST_ASSERT_EQ(text, "4,2,5,1,3,6");
    text = "";
    binary_tree_nlr(top, [&text](struct BinaryTree<int>* ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele->data);
    });
    GTEST_ASSERT_EQ(text, "1,2,4,5,3,6");
    text = "";
    binary_tree_bfs(top, [&text](struct BinaryTree<int>* ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele->data);
    });
    GTEST_ASSERT_EQ(text, "1,2,3,4,5,6");
    binary_tree_clear(top);
}

TEST(BinaryTreeTest, BinarySearchTree1) {
    BinarySearchTree<int, int>* tree = nullptr;
    binary_search_tree_insert(tree, 100, 3);
    GTEST_ASSERT_EQ(tree->data.key, 100);
    GTEST_ASSERT_EQ(tree->data.value, 3);
    binary_search_tree_insert(tree, 100, 20);
    GTEST_ASSERT_EQ(tree->data.value, 20);
    binary_search_tree_insert(tree, 20, 45);
    binary_search_tree_insert(tree, 40, 13);
    binary_search_tree_insert(tree, 33, 23);
    binary_search_tree_insert(tree, 77, 33);
    binary_search_tree_insert(tree, 120, 222);
    std::string keys;
    std::string values;
    binary_search_tree_iter(tree, [&keys, &values](int key, int value) {
        if (!keys.empty()) {
            keys += ",";
            values += ",";
        }
        keys += std::to_string(key);
        values += std::to_string(value);
    });
    GTEST_ASSERT_EQ(keys, "20,33,40,77,100,120");
    GTEST_ASSERT_EQ(values, "45,23,13,33,20,222");
    int v;
    GTEST_ASSERT_EQ(binary_search_tree_get(tree, 100, v), true);
    GTEST_ASSERT_EQ(v, 20);
    GTEST_ASSERT_EQ(binary_search_tree_get(tree, 33, v), true);
    GTEST_ASSERT_EQ(v, 23);
    GTEST_ASSERT_EQ(binary_search_tree_get(tree, 119, v), false);
    GTEST_ASSERT_EQ(v, 23);
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 77, &v), true);
    GTEST_ASSERT_EQ(v, 33);
    binary_search_tree_insert(tree, 27, 311);
    keys = "";
    values = "";
    binary_search_tree_iter(tree, [&keys, &values](int key, int value) {
        if (!keys.empty()) {
            keys += ",";
            values += ",";
        }
        keys += std::to_string(key);
        values += std::to_string(value);
    });
    GTEST_ASSERT_EQ(keys, "20,27,33,40,100,120");
    GTEST_ASSERT_EQ(values, "45,311,23,13,20,222");
    binary_search_tree_insert(tree, 80, 122);
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 33, &v), true);
    GTEST_ASSERT_EQ(v, 23);
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 40, &v), true);
    GTEST_ASSERT_EQ(v, 13);
    keys = "";
    values = "";
    binary_search_tree_iter(tree, [&keys, &values](int key, int value) {
        if (!keys.empty()) {
            keys += ",";
            values += ",";
        }
        keys += std::to_string(key);
        values += std::to_string(value);
    });
    GTEST_ASSERT_EQ(keys, "20,27,80,100,120");
    GTEST_ASSERT_EQ(values, "45,311,122,20,222");
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 120), true);
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 120, &v), false);
    GTEST_ASSERT_EQ(v, 13);
    auto j = binary_search_tree_get_node(tree, 20);
    GTEST_ASSERT_EQ(j->data.key, 20);
    binary_search_tree_clear(tree);
}

TEST(BinaryTreeTest, BinarySearchTree2) {
    BinarySearchTree<int, int>* tree = nullptr;
    binary_search_tree_insert(tree, 100, 3);
    int v;
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 100, &v), true);
    GTEST_ASSERT_EQ(v, 3);
    GTEST_ASSERT_EQ(tree, nullptr);
    binary_search_tree_insert(tree, 2, 3);
    binary_search_tree_insert(tree, 1, 2);
    binary_search_tree_insert(tree, 3, 4);
    GTEST_ASSERT_EQ(binary_search_tree_delete(tree, 2, &v), true);
    GTEST_ASSERT_EQ(v, 3);
    GTEST_ASSERT_EQ(tree->data.key, 1);
    GTEST_ASSERT_EQ(tree->data.value, 2);
    binary_search_tree_insert(tree, -100, 123);
    binary_search_tree_insert(tree, -200, 200);
    binary_search_tree_clear(tree);
}

TEST(BinaryTreeTest, BinarySearchTree3) {
    BinarySearchMap<int, int> map;
    map.insert(20, 30);
    map.insert(30, 40);
    map.insert(10, 20);
    map[20] = 55;
    map[33] = 44;
    GTEST_ASSERT_EQ(map[20], 55);
    GTEST_ASSERT_EQ(map[33], 44);
    int v;
    GTEST_ASSERT_EQ(map.get(33, v), true);
    GTEST_ASSERT_EQ(v, 44);
    GTEST_ASSERT_EQ(map.del(20, &v), true);
    GTEST_ASSERT_EQ(v, 55);
    std::string keys;
    std::string values;
    map.iter([&keys, &values](int key, int value) {
        if (!keys.empty()) {
            keys += ",";
            values += ",";
        }
        keys += std::to_string(key);
        values += std::to_string(value);
    });
    GTEST_ASSERT_EQ(keys, "10,30,33");
    GTEST_ASSERT_EQ(values, "20,40,44");
}
