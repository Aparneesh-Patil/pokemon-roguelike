#ifndef TURNPQ_H
#define TURNPQ_H

#include "trainer.h"

class turnpq {
public:
    character **a;
    int size;
    int cap;

    turnpq() : a(nullptr), size(0), cap(0) {}
};

int  turnpq_init(turnpq *pq, int cap);
void turnpq_destroy(turnpq *pq);
int  turnpq_push(turnpq *pq, character *c);
character *turnpq_pop(turnpq *pq);
character *turnpq_peek(turnpq *q);

#endif