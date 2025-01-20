#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "ll.h"

unsigned long hm_hash(char *key, unsigned short size_exp) {
    unsigned long h = 3323198485ul;
    for(; *key; ++key) {
        h ^= *key;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }

    unsigned long mask = (1 << size_exp) - 1;
    h = h & mask;

    return h;
}

struct hm_map *hm_create(unsigned short size_exp) {
    struct hm_map *map = (struct hm_map *)malloc(sizeof(struct hm_map));

    map->size_exp = size_exp;
    map->items = 0;

    unsigned long long truesize = 1 << size_exp;
    struct ll_list **buckets = (struct ll_list **)malloc(sizeof(struct ll_list *) * truesize);
    for(int i = 0; i < truesize; i++) buckets[i] = NULL;

    map->buckets = buckets;
}

void hm_insert(struct hm_map *map, char *key, void *data) {
    struct hm_element *element = (struct hm_element *)malloc(sizeof(struct hm_element));
    element->key = (char *)malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(element->key, key);
    element->value = data;

    unsigned long hash = hm_hash(key, map->size_exp);
    if(map->buckets[hash] == NULL) map->buckets[hash] = ll_create();

    ll_push(map->buckets[hash], element);
    map->items++;
}

void *hm_get(struct hm_map *map, char *key) {
    unsigned long hash = hm_hash(key, map->size_exp);
    struct ll_list *bucket = map->buckets[hash];
    if(bucket == NULL) return NULL; // nothing was ever inserted here
    else {
        unsigned int length = ll_length(bucket);
        if(length == 0) return NULL; // linked list doesn't have any items
        else {
            for(int i = 0; i < length; i++) {
                struct hm_element *element = (struct hm_element *)ll_get(bucket, i);
                if(strcmp(key, element->key) == 0) return element->value;
            }

            return NULL; // couldn't find anything
        }
    }
}

void *hm_delete(struct hm_map *map, char *key) {
    unsigned long hash = hm_hash(key, map->size_exp);
    struct ll_list *bucket = map->buckets[hash];
    if(bucket == NULL) return NULL;
    else {
        unsigned int length = ll_length(bucket);
        if(length == 0) return NULL;
        else {
            int index = -1;
            for(int i = 0; i < length; i++) {
                struct hm_element *temp = (struct hm_element *)ll_get(bucket, i);
                if(strcmp(key, temp->key) == 0) index = i;
            }
            if(index == -1) return NULL; // not found

            // from here we know it was found
            struct hm_element *element = (struct hm_element *)ll_get(bucket, index);
            free(element->key);
            void *data = element->value;

            free(element);
            ll_delete(bucket, index);

            map->items--;

            return data;
        }
    }
}

void **hm_freeall(struct hm_map *map) {
    unsigned int hashmap_items_index = 0;
    void **hashmap_items = (void **)malloc(sizeof(void *) * map->items);

    for(int i = 0; i < (1 << map->size_exp); i++) {
        struct ll_list *bucket = map->buckets[i];
        if(bucket == NULL) continue;

        unsigned int len = ll_length(bucket);
        struct hm_element **values = ll_freeall(bucket);
        for(int j = 0; j < len; j++) {
            printf("%d\n", hashmap_items_index);
            struct hm_element *value = values[j];
            hashmap_items[hashmap_items_index] = value->value;
            hashmap_items_index++;

            free(value->key);
            free(value);
        }
        free(values);
    }

    free(map->buckets);
    free(map);

    return hashmap_items;
}