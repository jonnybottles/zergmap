#include "priority-queue.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

struct item {
	double priority;
	uint16_t obj;
};

struct pqueue_ {
	struct item *data;
	size_t sz;
	size_t cap;
	enum priority_type type;
};

// Seems reasonable to start with?
enum { DEFAULT_HEAP_CAPACITY = 8 };

static size_t parent(size_t idx)
{
	return (idx - 1) / 2;
}

static size_t left(size_t idx)
{
	return 2 * idx + 1;
}

static size_t right(size_t idx)
{
	return 2 * idx + 2;
}

static void swap(struct item *arr, size_t a, size_t b)
{
	uint16_t tmp = arr[a].obj;
	arr[a].obj = arr[b].obj;
	arr[b].obj = tmp;

	double tmp_priority = arr[a].priority;
	arr[a].priority = arr[b].priority;
	arr[b].priority = tmp_priority;
}

pqueue *pqueue_create(enum priority_type type)
{
	pqueue *pq = malloc(sizeof(*pq));
	if (!pq) {
		return NULL;
	}

	pq->sz = 0;
	pq->cap = DEFAULT_HEAP_CAPACITY;
	pq->type = type;
	pq->data = malloc(pq->cap * sizeof(*pq->data));
	if (!pq->data) {
		free(pq);
		return NULL;
	}

	return pq;
}

double pqueue_get_next_priority(const pqueue * pq)
{
	if (!pq || pq->sz == 0) {
		return NAN;
	}

	return pq->data[0].priority;
}

uint16_t pqueue_get_next(pqueue * pq)
{
	if (!pq || pq->sz == 0) {
		return 0xffff;
	}

	return pq->data[0].obj;
}

bool pqueue_is_empty(const pqueue * pq)
{
	if (!pq) {
		return true;
	}

	return pq->sz == 0;
}

bool pqueue_enqueue(pqueue * pq, double priority, uint16_t obj)
{
	if (!pq || isnan(priority)) {
		return false;
	}

	if (pq->sz + 1 >= pq->cap) {
		struct item *tmp = realloc(pq->data, 2 * pq->cap
					* sizeof(*pq->data));
		if (!tmp) {
			return false;
		}
		pq->cap *= 2;
		pq->data = tmp;
	}

	pq->data[pq->sz].priority = priority;
	pq->data[pq->sz++].obj = obj;

	size_t idx = pq->sz - 1;

	// Bubble up!
	while (idx > 0 &&
		(pq->type == MIN ? (pq->data[parent(idx)].priority > priority)
		: (pq->data[parent(idx)].priority < priority))) {

		swap(pq->data, idx, parent(idx));

		idx = parent(idx);
	}

	return true;
}

double pqueue_get_priority(const pqueue * pq, uint16_t obj)
{
	if (!pq || !obj) {
		return NAN;
	}

	for (size_t n = 0; n < pq->sz; ++n) {
		if (pq->data[n].obj == obj) {
			return pq->data[n].priority;
		}
	}

	return NAN;
}

uint16_t pqueue_dequeue(pqueue * pq)
{
	if (!pq || pq->sz == 0) {
		return 0xffff;
	}

	uint16_t result = pq->data[0].obj;

	pq->sz--;
	swap(pq->data, 0, pq->sz);
	size_t idx = 0;

	// Bubble down!
	while (idx < pq->sz) {
		// Zero children case
		if (pq->sz <= left(idx)) {
			return result;
		}
		// One-child case
		if (pq->sz <= right(idx)) {
			if (pq->type == MIN
				? pq->data[idx].priority >
				pq->data[left(idx)].priority
				: pq->data[idx].priority <
				pq->data[left(idx)].priority) {
				swap(pq->data, idx, left(idx));
			}

			return result;
		}

		double our_priority = pq->data[idx].priority;
		double left_priority = pq->data[left(idx)].priority;
		double right_priority = pq->data[right(idx)].priority;

		if (pq->type == MIN
			? (our_priority < left_priority &&
			our_priority < right_priority)
			: (our_priority > left_priority &&
			our_priority > right_priority)) {
			return result;
		}

		if (pq->type == MIN
			? left_priority < right_priority
			: left_priority > right_priority) {
			swap(pq->data, idx, left(idx));
			idx = left(idx);
		} else {
			swap(pq->data, idx, right(idx));
			idx = right(idx);
		}
	}

	return result;
}

// used for debugging purposes
void pqueue_print(pqueue * pq)
{
	if (!pq) {
		return;
	}

	for (size_t n = 0; n < pq->cap; n++) {
		printf("id: %u connections: %lf\n", pq->data[n].obj,
			pq->data[n].priority);
	}
}

void pqueue_destroy(pqueue * pq)
{
	if (!pq) {
		return;
	}

	free(pq->data);
	free(pq);
}
