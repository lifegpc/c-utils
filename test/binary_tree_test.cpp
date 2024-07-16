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
    binary_search_tree_clear(tree);
}
