#include <stdlib.h>
#include "ll.h"

struct ll_list *ll_create(void) {
    struct ll_list *list = (struct ll_list *)malloc(sizeof(struct ll_list));
    list->head = NULL;
    list->current = NULL;

    return list;
}

void ll_push(struct ll_list *list, void *ptr) {
    struct ll_item *item = (struct ll_item *)malloc(sizeof(struct ll_item));
    item->item = ptr;
    item->next = NULL;

    if(list->head == NULL) list->head = item;
    if(list->current != NULL) list->current->next = item;
    list->current = item;
}

unsigned int ll_length(struct ll_list *list) {
    unsigned int len = 0;
    for(struct ll_item *current = list->head; current != NULL; current = current->next) len++;

    return len;
}

void *ll_get(struct ll_list *list, unsigned int pos) {
    struct ll_item *item = list->head;
    for(unsigned int i = 0; (item != NULL) && (i < pos); i++, item = item->next);

    void *ptr;
    if(item == NULL) ptr = NULL;
    else ptr = item->item;

    return ptr;
}

void *ll_delete(struct ll_list *list, unsigned int pos) {
    if(list->head == NULL) return;

    struct ll_item *current = list->head;
    struct ll_item *previous = NULL;

    for(unsigned int i = 0; (current != NULL) && (i < pos); i++, previous = current, current = current->next);

    if(current == list->head) {
        if(current->next == NULL) {
            list->head = NULL;
            list->current = NULL;
        } else {
            list->head = current->next;
        }
    } else if(current == list->current) {
        list->current = previous;
        previous->next = NULL;
    } else { // somewhere in the middle, there will be a next item
        previous->next = current->next;
    }

    void *ptr = current->item;
    free(current);

    return ptr;
}

void **ll_freeall(struct ll_list *list) {
    if(list->head == NULL) return NULL;

    unsigned int list_len = ll_length(list);
    void **list_items = (void **)malloc(sizeof(void *) * list_len);

    struct ll_item *item = list->head;
    int i = 0;
    while(item != NULL) {
        struct ll_item *current = item;
        list_items[i] = current->item;
        item = item->next;
        i++;

        free(current);
    }

    free(list);

    return list_items;
}