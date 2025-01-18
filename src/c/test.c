#include <stdio.h>
#include <stdlib.h>
#include "ll.h"

void ll_debug_numberprint(struct ll_list *list) {
    printf("[%d]: ", ll_length(list));
    for(struct ll_item *item = list->head; item != NULL; item = item->next) {
        printf("%d ", *(int *)item->item);
    }
    printf("\n");
}

int main(void) {
    struct ll_list *list = ll_create();

    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;

    ll_push(list, &a);
    ll_push(list, &b);
    ll_push(list, &c);

    ll_debug_numberprint(list);

    ll_delete(list, 1);
    ll_debug_numberprint(list);

    ll_delete(list, 0);
    ll_debug_numberprint(list);

    ll_push(list, &d);
    ll_debug_numberprint(list);

    ll_delete(list, 1);
    ll_debug_numberprint(list);

    free(ll_freeall(list));



    return 0;
}