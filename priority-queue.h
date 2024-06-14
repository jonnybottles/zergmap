#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct pqueue_ pqueue;

enum priority_type { MIN, MAX };

pqueue *pqueue_create(enum priority_type type);

double pqueue_get_next_priority(const pqueue * pq);

// Returns top item without removing it
uint16_t pqueue_get_next(pqueue * pq);

double pqueue_get_priority(const pqueue * pq, uint16_t obj);

bool pqueue_is_empty(const pqueue * pq);

bool pqueue_enqueue(pqueue * pq, double priority, uint16_t obj);

// Remove top item and return it
uint16_t pqueue_dequeue(pqueue * pq);

void pqueue_print(pqueue * pq);

void pqueue_destroy(pqueue * pq);

#endif
