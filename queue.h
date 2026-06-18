#ifndef QUEUE_H
#define QUEUE_H

// coordinate point
struct point_t {
    int x;
    int y;
};

struct queue_item {
    // store by value (avoids ownership issues)
    point_t p;
    queue_item *next;
};

struct queue {
    queue_item *head;
    queue_item *tail;
    int size;
};

int queue_init(queue *s);
int queue_destroy(queue *s);
int queue_push(queue *s, point_t t);
int queue_pop(queue *s, point_t *out);
int queue_size(queue *s, int *size);

#endif