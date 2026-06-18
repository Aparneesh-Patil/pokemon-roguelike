#include <cstdlib>
#include "turnpq.h"
#include "trainer.h"

static int less(character *x, character *y);
static void swap(character **x, character **y);

static void sift_up(turnpq *pq, int i);
static void sift_down(turnpq *pq, int i);

int turnpq_init(turnpq *pq, int cap) {
    if (!pq) return -1;

    pq->size = 0;

    // max capacity for the amount of characters on the map (includes pc)
    if (cap <= 0) {
        pq->cap = MAX_TRAINERS + 1;
    } else {
        pq->cap = cap;
    }

    pq->a = new character*[pq->cap];
    if (!pq->a) return -1;

    return 0;
}

void turnpq_destroy(turnpq *pq) {
    delete[] pq->a;
    pq->a = nullptr;
    pq->size = 0;
    pq->cap = 0;
}

static void sift_up(turnpq *pq, int i) {
    int p;

    while (i > 0) {
        p = (i - 1) / 2;
        if (less(pq->a[p], pq->a[i])) break;
        swap(&pq->a[p], &pq->a[i]);
        i = p;
    }
}

static void sift_down(turnpq *pq, int i) {
    int l, r, m;

    while (1) {
        l = 2 * i + 1;
        r = 2 * i + 2;
        m = i;

        if (l < pq->size && less(pq->a[l], pq->a[m])) m = l;
        if (r < pq->size && less(pq->a[r], pq->a[m])) m = r;

        if (m == i) break;
        swap(&pq->a[i], &pq->a[m]);
        i = m;
    }
}

int turnpq_push(turnpq *pq, character *c) {
    if (!pq || !pq->a) return -1;
    if (pq->size >= pq->cap) return -1;

    pq->a[pq->size] = c;
    sift_up(pq, pq->size);
    pq->size++;
    return 0;
}

character *turnpq_peek(turnpq *q) {
    if (!q || q->size == 0) {
        return nullptr;
    }

    return q->a[0];
}

character *turnpq_pop(turnpq *pq) {
    character *min;

    if (pq->size <= 0) {
        return nullptr;
    }

    min = pq->a[0];
    pq->size--;

    if (pq->size > 0) {
        pq->a[0] = pq->a[pq->size];
        sift_down(pq, 0);
    }

    return min;
}

// less than for two character objects
static int less(character *x, character *y) {
    if (x->next_time != y->next_time) {
        return x->next_time < y->next_time;
    }

    return x->id < y->id;
}

// swaps two character pointers
static void swap(character **x, character **y) {
    character *temp;
    temp = *x;
    *x = *y;
    *y = temp;
}