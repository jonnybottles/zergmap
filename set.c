#include "set.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <openssl/md5.h>

struct set_ {
	uint16_t *data;
	size_t sz;
	size_t cap;
};

// Seems like a good default?
enum { DEFAULT_HASH_CAPACITY = 8 };

// 75% seems like a good idea?
static const double LOAD_FACTOR = 0.75;

set *set_create(void)
{
	set *s = malloc(sizeof(*s));
	if (!s) {
		return NULL;
	}

	s->sz = 0;
	s->cap = DEFAULT_HASH_CAPACITY;
	s->data = calloc(s->cap, sizeof(*s->data));

	return s;
}

bool set_add(set * s, uint16_t id)
{
	if (!s) {
		return false;
	}

	if (1.0 * s->sz / s->cap > LOAD_FACTOR) {
		set *new_set = set_create();
		new_set->cap = 2 * s->cap;
		free(new_set->data);
		new_set->data = calloc(new_set->cap, sizeof(*new_set->data));
		if (!new_set->data) {
			set_destroy(new_set);
			return false;
		}

		for (size_t n = 0; n < s->cap; ++n) {
			if (s->data[n]) {
				set_add(new_set, s->data[n]);
			}
		}
		free(s->data);

		s->data = new_set->data;
		s->cap = new_set->cap;
		free(new_set);
	}

	size_t idx = id % s->cap;

	// PROBING
	while (s->data[idx]) {
		if (s->data[idx] == id) {
			// Good arguments for true or false return in this case;
			// using false to indicate "did not insert" vs.
			// using true to indicate "item is now in set"
			return false;
		}

		++idx;
		idx %= s->cap;
	}

	s->data[idx] = id;
	if (s->data[idx]) {
		s->sz++;
	}

	return s->data[idx] != 0;
}

bool set_contains(const set * s, uint16_t id)
{
	if (!s) {
		return false;
	}

	size_t idx = id % s->cap;

	while (s->data[idx]) {
		if (s->data[idx] == id) {
			return true;
		}

		++idx;
		idx %= s->cap;
	}

	return false;
}

void set_destroy(set * s)
{
	if (!s) {
		return;
	}

	for (size_t n = 0; n < s->cap; ++n) {
	}
	free(s->data);
	free(s);
}

// used for debugging purposes; also used with recommend_removal function
void print_set(set * s)
{
	if (!s) {
		return;
	}

	for (size_t n = 0; n < s->cap; n++) {
		if (s->data[n] != 0) {
			printf("#%u \n", s->data[n]);
		}
	}
	puts("\n");
}
