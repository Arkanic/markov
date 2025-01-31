#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "ll.h"

// quadruple hashmap size for every resize (+2 bits to hashmap addressing)
#define RESIZE_INCREMENT_BITCOUNT 2
// once the number of items is 75% of the total bucket count, resize
#define MAX_FILL_CAPACITY 0.75

// hash function
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

struct ll_list **_hm_bucket_init(unsigned short size_exp) {
    unsigned long long truesize = 1 << size_exp;
    struct ll_list **buckets = (struct ll_list **)malloc(sizeof(struct ll_list *) * truesize);
    for(int i = 0; i < truesize; i++) buckets[i] = NULL;

    return buckets;
}

struct hm_map *hm_create(unsigned short size_exp) {
    struct hm_map *map = (struct hm_map *)malloc(sizeof(struct hm_map));

    map->size_exp = size_exp;
    map->items = 0;

    map->buckets = _hm_bucket_init(size_exp);

    return map;
}

void hm_bucket_print(struct hm_map *map) {
    printf("BUCKET SIZE: %d ITEMS: %lu\n", map->size_exp, map->items);
    for(unsigned int i = 0; i < (1 << map->size_exp); i++) {
        if(map->buckets[i] == NULL) {
            printf("  |- bucket %04u empty\n", i);
        } else {
            printf("  |- bucket %04u\n", i);
            for(int j = 0; j < ll_length(map->buckets[i]); j++) {
                struct hm_element *elem = ll_get(map->buckets[i], j);
                printf("       |- entry \"%s\"\n", elem->key);
            }
        }
    }
}

struct hm_element **_hm_flatten(struct hm_map *map) {
    unsigned int hashmap_items_index = 0;
    struct hm_element **hashmap_items = (struct hm_element **)malloc(sizeof(struct hm_element *) * map->items);

    for(int i = 0; i < (1 << map->size_exp); i++) {
        struct ll_list *bucket = map->buckets[i];
        if(bucket == NULL) continue;

        unsigned int len = ll_length(bucket);
        struct hm_element **values = (struct hm_element **)ll_freeall(bucket);
        for(int j = 0; j < len; j++) {
            struct hm_element *value = values[j];
            hashmap_items[hashmap_items_index] = value;
            hashmap_items_index++;
        }
        free(values);
    }

    return hashmap_items;
}

void _hm_insert_element(struct hm_map *map, struct hm_element *element) {
    unsigned long hash = hm_hash(element->key, map->size_exp);
    if(map->buckets[hash] == NULL) map->buckets[hash] = ll_create();

    ll_push(map->buckets[hash], element);
}

void _hm_resize_map(struct hm_map *map) {
    struct hm_element **elements = _hm_flatten(map);
    free(map->buckets);

    map->size_exp += RESIZE_INCREMENT_BITCOUNT;
    map->buckets = _hm_bucket_init(map->size_exp);

    for(unsigned long i = 0; i < map->items; i++) {
        _hm_insert_element(map, elements[i]);
    }

    free(elements);
}

void hm_insert(struct hm_map *map, char *key, void *data) {
    struct hm_element *element = (struct hm_element *)malloc(sizeof(struct hm_element));
    element->key = (char *)malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(element->key, key);
    element->value = data;

    _hm_insert_element(map, element);
    map->items++;

    // now check if we need to resize
    if(map->items > (unsigned long)(MAX_FILL_CAPACITY * (1 << map->size_exp))) {
        if((map->size_exp + RESIZE_INCREMENT_BITCOUNT) <= 32) _hm_resize_map(map);
    }
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

void **hm_values(struct hm_map *map) {
    unsigned int hashmap_items_index = 0;
    void **hashmap_items = (void **)malloc(sizeof(void *) * map->items);

    for(int i = 0; i < (1 << map->size_exp); i++) {
        struct ll_list *bucket = map->buckets[i];
        if(bucket == NULL) continue;

        unsigned int len = ll_length(bucket);
        for(int j = 0; j < len; j++) {
            struct hm_element *elem = ll_get(bucket, j);
            hashmap_items[hashmap_items_index] = elem->value;
            hashmap_items_index++;
        }
    }

    return hashmap_items;
}

void **hm_freeall(struct hm_map *map) {
    unsigned int hashmap_items_index = 0;
    void **hashmap_items = (void **)malloc(sizeof(void *) * map->items);

    for(int i = 0; i < (1 << map->size_exp); i++) {
        struct ll_list *bucket = map->buckets[i];
        if(bucket == NULL) continue;

        unsigned int len = ll_length(bucket);
        struct hm_element **values = (struct hm_element **)ll_freeall(bucket);
        for(int j = 0; j < len; j++) {
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