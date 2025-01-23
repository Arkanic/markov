#include "ll.h"

#ifndef HASHMAP_H
struct hm_map {
    unsigned short size_exp;
    unsigned long items;

    struct ll_list **buckets;
};

struct hm_element {
    char *key;
    void *value;
};

struct hm_map *hm_create(unsigned short size_exp);
void hm_insert(struct hm_map *map, char *key, void *data);
void *hm_get(struct hm_map *map, char *key);
void *hm_delete(struct hm_map *map, char *key);
void **hm_values(struct hm_map *map);
void **hm_freeall(struct hm_map *map);
#endif

#define HASHMAP_H