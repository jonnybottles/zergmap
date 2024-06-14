#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stdint.h>

typedef struct set_ set;

set *set_create(void);

bool set_add(set * s, uint16_t id);

bool set_contains(const set * s, uint16_t id);

void print_set(set * s);

void set_destroy(set * s);

#endif
