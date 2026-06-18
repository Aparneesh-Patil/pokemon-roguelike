#ifndef HEAP_H
#define HEAP_H

#include "queue.h"

struct heap_item {
    heap_item *parent;
    heap_item *child;
    heap_item *left;
    heap_item *right;
    point_t cell;
    bool mark;
    int degree;
    int distance;
};

struct heap {
    heap_item *min;
    int size;
};

int heap_init(heap *h);
int heap_destroy(heap *h);
heap_item *heap_insert(heap *h, point_t cell_coord, int node_distance);
heap_item *heap_extract_min(heap *h);
void heap_consolidate(heap *h);
int heap_decrease_key(heap *h, heap_item *node, int new_key);
int heap_delete_key(heap *h, heap_item *node);

#endif