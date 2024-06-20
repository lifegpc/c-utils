#include "gtest/gtest.h"
#include "circular_queue.h"
#include <string>

TEST(Queue_Test, CircularQueue1) {
    struct CircularQueue<int> queue;
    EXPECT_EQ(init_circular_queue(queue, 3), true);
    EXPECT_EQ(circular_queue_is_empty(queue), true);
    int v;
    EXPECT_EQ(circular_queue_get_back(queue, v), false);
    EXPECT_EQ(circular_queue_get_head(queue, v), false);
    EXPECT_EQ(circular_queue_pop(queue, v), false);
    EXPECT_EQ(circular_queue_push(queue, 1), true);
    EXPECT_EQ(circular_queue_push(queue, 2), true);
    EXPECT_EQ(circular_queue_is_full(queue), true);
    EXPECT_EQ(circular_queue_is_empty(queue), false);
    EXPECT_EQ(circular_queue_push(queue, 3), false);
    EXPECT_EQ(circular_queue_pop(queue, v), true);
    EXPECT_EQ(v, 1);
    EXPECT_EQ(circular_queue_is_full(queue), false);
    EXPECT_EQ(circular_queue_push(queue, 3), true);
    EXPECT_EQ(circular_queue_get_head(queue, v), true);
    EXPECT_EQ(v, 2);
    EXPECT_EQ(circular_queue_get_back(queue, v), true);
    EXPECT_EQ(v, 3);
    EXPECT_EQ(circular_queue_length(queue), 2);
    EXPECT_EQ(queue.front, 1);
    EXPECT_EQ(queue.rear, 0);
    std::string text;
    circular_queue_iter(queue, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    EXPECT_EQ(text, "2,3");
    EXPECT_EQ(circular_queue_pop(queue, v), true);
    EXPECT_EQ(v, 2);
    EXPECT_EQ(queue.front, 2);
    EXPECT_EQ(circular_queue_length(queue), 1);
    EXPECT_EQ(circular_queue_push(queue, 4), true);
    EXPECT_EQ(circular_queue_pop(queue, v), true);
    EXPECT_EQ(v, 3);
    EXPECT_EQ(queue.front, 0);
    EXPECT_EQ(queue.rear, 1);
    circular_queue_clear(queue);
    EXPECT_EQ(queue.rear, 0);
    EXPECT_EQ(circular_queue_is_empty(queue), true);
    free_circular_queue(queue);
}

TEST(Queue_Test, CircularQueue2) {
    struct CircularQueue<int> queue;
    EXPECT_EQ(init_circular_queue(queue, 5), true);
    EXPECT_EQ(circular_queue_push(queue, 1), true);
    EXPECT_EQ(circular_queue_push(queue, 2), true);
    EXPECT_EQ(circular_queue_push(queue, 3), true);
    EXPECT_EQ(circular_queue_push(queue, 4), true);
    std::string text;
    circular_queue_iter(queue, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    EXPECT_EQ(text, "1,2,3,4");
    int v;
    EXPECT_EQ(circular_queue_pop(queue, v), true);
    EXPECT_EQ(v, 1);
    EXPECT_EQ(circular_queue_push(queue, 5), true);
    EXPECT_EQ(circular_queue_length(queue), 4);
    EXPECT_EQ(queue.front, 1);
    EXPECT_EQ(queue.rear, 0);
    text = "";
    circular_queue_iter(queue, [&text](int ele) {
        if (!text.empty()) {
            text += ",";
        }
        text += std::to_string(ele);
    });
    EXPECT_EQ(text, "2,3,4,5");
    free_circular_queue(queue);
}
