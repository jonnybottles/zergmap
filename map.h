#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct map_ map;

map *map_create(void);

bool map_put(map * m, uint16_t key, void *value);

void *map_get(map * m, uint16_t key);

bool map_contains(const map * m, const uint16_t key);

void map_delete(map * m, uint16_t key);

void map_destroy(map * m);

#endif
