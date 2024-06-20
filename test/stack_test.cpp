#include "gtest/gtest.h"
#include "stack.h"
#include "linked_stack.h"
#include <string>

TEST(Stack_Test, Stack1) {
    struct Stack<int> stack;
    ASSERT_EQ(init_stack(stack, 10), true) << "Can not initiailze stack.";
    ASSERT_EQ(stack_is_empty(stack), true) << "Stack is not empty.";
    ASSERT_EQ(stack_push(stack, 3), true) << "Failed to push elements to stack";
    int top;
    ASSERT_EQ(stack_get_top(stack, top), true) << "Failed to get top element.";
    ASSERT_EQ(top, 3) << "Value is not expeced.";
    ASSERT_EQ(stack_length(stack), 1) << "Length should be 1";
    ASSERT_EQ(stack_is_empty(stack), false) << "Stack is empty.";
    ASSERT_EQ(stack_push(stack, 5), true) << "Failed to push to stack.";
    ASSERT_EQ(stack_length(stack), 2) << "Length should be 2";
    std::string text;
    stack_iter(stack, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    ASSERT_EQ(text, "5,3") << "Failed to join.";
    ASSERT_EQ(stack_pop(stack, top), true) << "Failed to pop.";
    ASSERT_EQ(top, 5) << "Poped value not equal.";
    ASSERT_EQ(stack_length(stack), 1) << "Length should be 1";
    free_stack(stack);
}

TEST(Stack_Test, Stack2) {
    struct Stack<int> stack;
    ASSERT_EQ(init_stack(stack, 1, 2), true) << "Can not initiailze stack.";
    ASSERT_EQ(stack_push(stack, 3), true) << "Failed to push elements to stack";
    ASSERT_EQ(stack_push(stack, 4), true) << "Failed to push elements to stack";
    ASSERT_EQ(stack.capacity, 3) << "Capacity not expected";
    ASSERT_EQ(stack_push(stack, 5), true) << "Failed to push elements to stack";
    ASSERT_EQ(stack_push(stack, 6), true) << "Failed to push elements to stack";
    ASSERT_EQ(stack.capacity, 5) << "Capacity not expected";
    std::string text;
    stack_iter(stack, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    ASSERT_EQ(text, "6,5,4,3") << "Failed to join.";
    ASSERT_EQ(stack_length(stack), 4) << "Length should be 4";
    free_stack(stack);
}

TEST(Stack_Test, LinkedStack1) {
    struct LinkedStack<int>* top = nullptr;
    EXPECT_EQ(linked_stack_push(top, 3), true);
    EXPECT_EQ(top->next, nullptr);
    EXPECT_EQ(top->d, 3);
    EXPECT_EQ(linked_stack_push(top, 4), true);
    EXPECT_EQ(top->d, 4);
    EXPECT_EQ(top->next->d, 3);
    int e;
    EXPECT_EQ(linked_stack_pop(top, e), true);
    EXPECT_EQ(e, 4);
    EXPECT_EQ(linked_stack_push(top, 8), true);
    EXPECT_EQ(linked_stack_push(top, 9), true);
    EXPECT_EQ(linked_stack_push(top, 10), true);
    std::string text;
    linked_stack_iter(top, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    EXPECT_EQ(text, "10,9,8,3");
    EXPECT_EQ(linked_stack_length(top), 4);
    linked_stack_clear(top);
    EXPECT_EQ(top, nullptr);
}
