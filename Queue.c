/* Copyright Isar Ioana-Teodora, Turcu Arina-Emanuela 313CA */

#include <stdlib.h>
#include <stdio.h>

#include "Queue.h"

void init_q(struct Queue *q) {
    q->list = malloc(sizeof(struct LinkedList));
    if (q->list == NULL) {
        perror("Not enough memory to initialize the queue!");
        return;
    }

    init_list(q->list);
}

int get_size_q(struct Queue *q) {
    return q->list->size;
}

int is_empty_q(struct Queue *q) {
    if (q->list->size == 0) {
        return 1;
    }
    return 0;
}

void* front(struct Queue *q) {
    return q->list->head->data;
}

void dequeue(struct Queue *q) {
    struct Node *be_gone;
    be_gone = remove_nth_node(q->list, 0);
    free(be_gone);
}

void enqueue(struct Queue *q, void *new_data) {
    add_nth_node(q->list, 0x7fffffff, new_data);
}

void clear_q(struct Queue *q) {
    while (q->list->size > 0) {
        dequeue(q);
    }
}

void purge_q(struct Queue *q) {
    free_list(&q->list);
}
