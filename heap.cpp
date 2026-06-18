#include <cstdlib>
#include <climits>
#include <cmath>
#include "heap.h"

// helper methods
static void free_all(heap_item *start);
static void cut(heap *h, heap_item *x, heap_item *y);
static void cascading_cut(heap *h, heap_item *y);

// initialize the heap
int heap_init(heap *h) {
    h->min = nullptr;
    h->size = 0;
    return 0;
}

// clear the heap
int heap_destroy(heap *h) {
    if (!h) return -1;

    if (h->min) {
        free_all(h->min);
        h->min = nullptr;
    }

    h->size = 0;
    return 0;
}

static void free_all(heap_item *start) {
    heap_item *x = start;
    heap_item *next = nullptr;

    start->left->right = nullptr;

    while (x) {
        next = x->right;

        if (x->child) {
            free_all(x->child);
            x->child = nullptr;
        }

        delete x;
        x = next;
    }
}

heap_item *heap_insert(heap *h, point_t cell_coord, int node_distance) {
    heap_item *node = new heap_item();

    if (!node) return nullptr;

    if (h->size == 0) {
        node->parent = nullptr;
        node->child = nullptr;
        node->left = node;
        node->right = node;
        node->cell = cell_coord;
        node->mark = false;
        node->degree = 0;
        node->distance = node_distance;

        h->min = node;
    } else {
        heap_item *temp1 = h->min->left;

        node->parent = nullptr;
        node->child = nullptr;
        node->left = temp1;
        node->right = h->min;
        node->cell = cell_coord;
        node->mark = false;
        node->degree = 0;
        node->distance = node_distance;

        h->min->left = node;
        temp1->right = node;

        if (h->min->distance > node->distance) {
            h->min = node;
        }
    }

    h->size++;
    return node;
}

heap_item *heap_extract_min(heap *h) {
    heap_item *z = h->min;
    if (!z) return nullptr;

    heap_item *root = (z->left != z) ? z->left : nullptr;

    // remove z
    z->left->right = z->right;
    z->right->left = z->left;

    // add children to root list
    if (z->child) {
        heap_item *x = z->child;
        heap_item *start = x;

        do {
            x->parent = nullptr;
            x->mark = false;
            x = x->right;
        } while (x != start);

        if (!root) {
            root = start;
        } else {
            heap_item *root_left = root->left;
            heap_item *child_left = start->left;

            root_left->right = start;
            start->left = root_left;

            child_left->right = root;
            root->left = child_left;
        }

        z->child = nullptr;
    }

    h->size--;
    h->min = root;

    if (h->min) heap_consolidate(h);

    z->left = z;
    z->right = z;
    z->parent = nullptr;
    z->mark = false;

    return z;
}

void heap_consolidate(heap *h) {
    if (!h || !h->min) return;

    int n = h->size;
    int D = (n > 0) ? (int)std::floor(std::log2(n)) + 3 : 3;

    heap_item **A = new heap_item*[D]();
    
    int root_count = 1;
    heap_item *w = h->min->right;
    while (w != h->min) {
        root_count++;
        w = w->right;
    }

    heap_item **roots = new heap_item*[root_count];

    w = h->min;
    for (int i = 0; i < root_count; i++) {
        roots[i] = w;
        w = w->right;
    }

    for (int i = 0; i < root_count; i++) {
        roots[i]->left = roots[i];
        roots[i]->right = roots[i];
        roots[i]->parent = nullptr;
        roots[i]->mark = false;
    }

    for (int i = 0; i < root_count; i++) {
        heap_item *x = roots[i];
        int d = x->degree;

        while (true) {
            if (d >= D) break;

            if (!A[d]) {
                A[d] = x;
                break;
            }

            heap_item *y = A[d];
            A[d] = nullptr;

            if (y->distance < x->distance) std::swap(x, y);

            y->parent = x;
            y->mark = false;

            if (!x->child) {
                x->child = y;
                y->left = y;
                y->right = y;
            } else {
                heap_item *c = x->child;
                heap_item *cl = c->left;

                y->right = c;
                y->left = cl;
                cl->right = y;
                c->left = y;
            }

            x->degree++;
            d = x->degree;
        }
    }

    h->min = nullptr;

    for (int i = 0; i < D; i++) {
        heap_item *x = A[i];
        if (!x) continue;

        x->parent = nullptr;
        x->left = x;
        x->right = x;

        if (!h->min) {
            h->min = x;
        } else {
            heap_item *m = h->min;
            heap_item *ml = m->left;

            x->right = m;
            x->left = ml;
            ml->right = x;
            m->left = x;

            if (x->distance < h->min->distance) {
                h->min = x;
            }
        }
    }

    delete[] roots;
    delete[] A;
}

int heap_decrease_key(heap *h, heap_item *node, int new_key) {
    if (new_key > node->distance) return -1;

    node->distance = new_key;
    heap_item *y = node->parent;

    if (y && y->distance > node->distance) {
        cut(h, node, y);
        cascading_cut(h, y);
    }

    if (!h->min || node->distance < h->min->distance) {
        h->min = node;
    }

    return 0;
}

static void cut(heap *h, heap_item *x, heap_item *y) {
    if (x->right == x) {
        y->child = nullptr;
        y->degree = 0;
    } else {
        x->left->right = x->right;
        x->right->left = x->left;

        if (y->child == x) y->child = x->right;
        y->degree--;
    }

    x->parent = nullptr;
    x->mark = false;
    x->left = x;
    x->right = x;

    if (!h->min) {
        h->min = x;
    } else {
        heap_item *m = h->min;
        heap_item *ml = m->left;

        x->right = m;
        x->left = ml;
        ml->right = x;
        m->left = x;

        if (x->distance < h->min->distance) {
            h->min = x;
        }
    }
}

static void cascading_cut(heap *h, heap_item *y) {
    heap_item *z = y->parent;

    if (z) {
        if (!y->mark) {
            y->mark = true;
        } else {
            cut(h, y, z);
            cascading_cut(h, z);
        }
    }
}

int heap_delete_key(heap *h, heap_item *node) {
    heap_decrease_key(h, node, INT_MIN);
    heap_item *z = heap_extract_min(h);

    if (z) delete z;
    return 0;
}