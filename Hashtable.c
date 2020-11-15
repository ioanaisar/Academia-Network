/* Copyright Isar Ioana-Teodora, Turcu Arina-Emanuela 313CA */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Hashtable.h"
#include "publications.h"

/*
 * Functii de comparare a cheilor:
 */
int compare_function_ints(void *a, void *b) {
    int int_a = *((int *)a);
    int int_b = *((int *)b);

    if (int_a == int_b) {
        return 0;
    } else if (int_a < int_b) {
        return -1;
    } else {
        return 1;
    }
}

int compare_function_strings(void *a, void *b) {
    char *str_a = (char *)a;
    char *str_b = (char *)b;

    return strcmp(str_a, str_b);
}

unsigned int hash_function_int(void *a) {
    /*
     * Credits: https://stackoverflow.com/a/12996028/7883884
     */
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_string(void *a) {
    /*
     * Credits: http://www.cse.yorku.ca/~oz/hash.html
     */
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

    return hash;
}

void init_ht(struct Hashtable *ht, int hmax,
        unsigned int (*hash_function)(void *),
        int (*compare_function)(void *, void *)) {
    int i;
    ht->buckets = (struct LinkedList *)malloc(hmax * sizeof(struct LinkedList));

    for (i = 0; i < hmax; ++i) {
        init_list(ht->buckets + i);
    }

    ht->size = 0;
    ht->hmax = hmax;
    ht->hash_function = hash_function;
    ht->compare_function = compare_function;
}

struct info *get_info(struct Hashtable *ht, int index, void *key) {
    struct Node *curr;
    struct LinkedList *buck;
    int i = 0;

    buck = &(ht->buckets[index]);

    curr = buck->head;

    while (curr != NULL) {
        if (ht->compare_function(((struct info *)curr->data)->key, key) == 0) {
            return (struct info *)curr->data;
        }
        curr = curr->next;
        i++;
    }

    return NULL;
}

void put(struct Hashtable *ht, void *key, int key_size_bytes, void *value) {
    int index;
    struct info *data;

    index = ht->hash_function(key) % ht->hmax;
    data = get_info(ht, index, key);

    if (data == NULL) {
        data = malloc(sizeof(struct info));
        if (data == NULL) {
            printf("Couldn't alloc\n");
            return;
        }

        data->key = malloc(key_size_bytes);
        if (data->key == NULL) {
            printf("Couldn't alloc\n");
            return;
        }

        memcpy(data->key, key, key_size_bytes);
        data->value = value;

        add_nth_node(&(ht->buckets[index]), 0x7fffffff, data);

        ht->size++;
    } else {
        free(data->value);
        data->value = value;
    }
}

void put2(struct Hashtable *ht, void *key, int key_size_bytes, void *value) {
    int index;
    struct info *data;

    index = ht->hash_function(key) % ht->hmax;

    data = malloc(sizeof(struct info));
    if (data == NULL) {
        printf("Couldn't alloc\n");
        return;
    }

    data->key = malloc(key_size_bytes);
    if (data->key == NULL) {
        printf("Couldn't alloc\n");
        return;
    }

    memcpy(data->key, key, key_size_bytes);
    data->value = value;

    add_nth_node(&(ht->buckets[index]), 0, data);

    ht->size++;
}

void *get(struct Hashtable *ht, void *key) {
    int index;
    struct info *information;

    index = ht->hash_function(key) % ht->hmax;

    information = get_info(ht, index, key);

    if (information == NULL) {
        return NULL;
    } else {
        return information->value;
    }
}

int has_key(struct Hashtable *ht, void *key) {
    void *check;
    check = get(ht, key);

    if (check != NULL) {
        return 1;
    }

    return 0;
}

void remove_ht_entry(struct Hashtable *ht, void *key) {
    int index, i;
    struct LinkedList *buck;
    struct Node *entry;

    index = ht->hash_function(key) % ht->hmax;
    buck = &ht->buckets[index];

    entry = buck->head;
    i = 0;

    while (entry != NULL) {
        if (ht->compare_function(((struct info *)entry->data)->key, key) == 0) {
            free(((struct info *)entry->data)->key);

            free(((struct info *)entry->data)->value);
            entry = remove_nth_node(buck, i);
            free(entry);

            break;
        }

        i++;
        entry = entry->next;
    }
}

void free_ht(struct Hashtable *ht) {
    /* TODO */
    int i;
    struct LinkedList *buck;
    void *key;

    for (i = 0; i < ht->hmax; ++i) {
        buck = &(ht->buckets[i]);

        while (buck->size > 0) {
            key = ((struct info *)buck->head->data)->key;
            remove_ht_entry(ht, key);
        }
    }

    free(ht->buckets);
    free(ht);
}

int get_ht_size(struct Hashtable *ht) {
    if (ht == NULL) {
        return -1;
    }

    return ht->size;
}

int get_ht_hmax(struct Hashtable *ht) {
    if (ht == NULL) {
        return -1;
    }

    return ht->hmax;
}
