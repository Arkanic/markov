#ifndef LL_H
struct ll_list {
    struct ll_item *head;
    struct ll_item *current;
};

struct ll_item {
    void *item;
    struct ll_item *next;
};

struct ll_list *ll_create(void);
void ll_push(struct ll_list *list, void *ptr);
unsigned int ll_length(struct ll_list *list);
void *ll_get(struct ll_list *list, unsigned int pos);
void *ll_delete(struct ll_list *list, unsigned int pos);
void **ll_freeall(struct ll_list *list);
#endif

#define LL_H