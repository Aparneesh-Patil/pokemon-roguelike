#include <cstdlib>
#include "queue.h"

int queue_init(queue *s) {
    s->head = nullptr;
    s->tail = nullptr;
    s->size = 0;

    return 0;
}

int queue_destroy(queue *s) {
    queue_item *tmp;

    while ((tmp = s->head)) {
        s->head = s->head->next;
        delete tmp;
    }

    s->size = 0;
    s->tail = nullptr;
    return 0;
}

int queue_push(queue *s, point_t t) {
    queue_item *tmp = new queue_item();

    if (!tmp) {
        // allocation failed
        return -1;
    }

    tmp->p = t;
    tmp->next = nullptr;

    if (!s->tail) {
        s->head = tmp;
        s->tail = tmp;
    } else {
        s->tail->next = tmp;
        s->tail = tmp;
    }

    s->size++;
    return 0;
}

int queue_pop(queue *s, point_t *out) {
    if (!s->head) {
        return -1;
    }

    queue_item *tmp = s->head;

    *out = s->head->p;
    s->head = s->head->next;

    if (!s->head) {
        s->tail = nullptr;
    }

    delete tmp;

    s->size--;
    return 0;
}

int queue_size(queue *s, int *size) {
    *size = s->size;
    return 0;
}