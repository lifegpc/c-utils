#include "c_linked_list.h"
#include "linked_list.h"

int c_linked_list_append(c_linked_list** list, void* data) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)(*list);
    auto re = linked_list_append(li, &data);
    *list = (c_linked_list*)li;
    return re;
}

int c_linked_list_append_head(c_linked_list** list, void* data) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)(*list);
    auto re = linked_list_append_head(li, &data);
    *list = (c_linked_list*)li;
    return re;
}

void c_linked_list_clear(c_linked_list** list, void(*free_func)(void*)) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)(*list);
    linked_list_clear(li, free_func);
    *list = (c_linked_list*)li;
}

size_t c_linked_list_count(c_linked_list* list) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)list;
    return linked_list_count(li);
}

void c_linked_list_free_tail(c_linked_list** list, void(*free_func)(void*)) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)(*list);
    linked_list_free_tail(li, free_func);
    *list = (c_linked_list*)li;
}

void c_linked_list_remove(c_linked_list** node, void(*free_func)(void*)) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)(*node);
    linked_list_remove(li);
    *node = (c_linked_list*)li;
}

c_linked_list* c_linked_list_tail(c_linked_list* list) {
    struct LinkedList<void*>* li = (struct LinkedList<void*>*)list;
    return (c_linked_list*)linked_list_tail(li);
}
