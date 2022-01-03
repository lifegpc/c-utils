#ifndef _UTIL_C_LINKED_LIST_H
#define _UTIL_C_LINKED_LIST_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
/**
 * @brief Struct are same with struct LinkedList<void*>
*/
typedef struct c_linked_list {
    void* d;
    struct c_linked_list* prev;
    struct c_linked_list* next;
} c_linked_list;
int c_linked_list_append(c_linked_list** list, void* data);
int c_linked_list_append_head(c_linked_list** list, void* data);
void c_linked_list_clear(c_linked_list** list, void(*free_func)(void*));
size_t c_linked_list_count(c_linked_list* list);
void c_linked_list_free_tail(c_linked_list** list, void(*free_func)(void*));
void c_linked_list_remove(c_linked_list** node, void(*free_func)(void*));
c_linked_list* c_linked_list_tail(c_linked_list* list);
#ifdef __cplusplus
}
#endif
#endif
