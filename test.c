#include <stdio.h>
#include <stdlib.h>
#include "ll.h"
#include "hashmap.h"

void ll_debug_numberprint(struct ll_list *list) {
    printf("[%d]: ", ll_length(list));
    for(struct ll_item *item = list->head; item != NULL; item = item->next) {
        printf("%d ", *(int *)item->item);
    }
    printf("\n");
}

int main(void) {
    printf("linked list:\n");
    struct ll_list *list = ll_create();

    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int e = 5;

    ll_push(list, &a);
    ll_push(list, &b);
    ll_push(list, &c);

    int **res = ll_freeall(list);
    for(int i = 0; i < 3; i++) {
        printf("%d ", *res[i]);
    }
    printf("\n");
    free(res);

    printf("hashmap:\n");
    struct hm_map *map = hm_create(2);
    printf("created\n");

    hm_insert(map, "a", &a);
    printf("first ");
    hm_insert(map, "b", &b);
    printf("second ");
    hm_insert(map, "c", &c);
    printf("third ");
    hm_insert(map, "d", &d);
    printf("fourth\n");

    // double up
    hm_insert(map, "e", &e);

    printf("%d %d %d %d %d\n", *(int *)hm_get(map, "a"), *(int *)hm_get(map, "b"), *(int *)hm_get(map, "c"), *(int *)hm_get(map, "d"), *(int *)hm_get(map, "e"));

    int *i = hm_delete(map, "e");
    printf("deleted: %d\n", *i);

    int **results = hm_freeall(map);
    printf("freed\n");
    for(int i = 0; i < 4; i++) {
        printf("%d ", *results[i]);
    }
    printf("\n");

    return 0;
}