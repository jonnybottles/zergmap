#include "map.h"
#include "pathfinding.h"
#include <stdlib.h>
#include <string.h>

#include <openssl/md5.h>

struct pair {
	uint16_t key;
	void *value;
};

struct map_ {
	struct pair *data;
	size_t sz;
	size_t cap;
};

static const double LOAD_FACTOR = 0.75;

// Seems like a reasonable starting capacity?
enum { DEFAULT_MAP_CAPACITY = 8 };

map *map_create(void)
{
	map *m = malloc(sizeof(*m));
	if (!m) {
		return NULL;
	}
	m->sz = 0;
	m->cap = DEFAULT_MAP_CAPACITY;

	m->data = calloc(m->cap, sizeof(*m->data));
	if (!m->data) {
		free(m);
		return NULL;
	}

	return m;
}

bool map_put(map * m, uint16_t key, void *value)
{
	if (!m) {
		return false;
	}

	if (1.0 * m->sz / m->cap > LOAD_FACTOR) {
		map *new_map = map_create();
		new_map->cap = 2 * m->cap;
		free(new_map->data);
		new_map->data = calloc(new_map->cap, sizeof(*new_map->data));
		if (!new_map->data) {
			map_destroy(new_map);
			return false;
		}

		for (size_t n = 0; n < m->cap; ++n) {
			if (m->data[n].key) {
				//TODO ABC
				map_put(new_map, m->data[n].key,
					m->data[n].value);
			}
		}
		free(m->data);

		m->data = new_map->data;
		m->cap = new_map->cap;
		free(new_map);
	}

	size_t idx = key % m->cap;
	while (m->data[idx].key) {
		if (m->data[idx].key == key) {
			m->data[idx].value = value;
			return true;
		}

		idx++;
		idx %= m->cap;
	}

	m->data[idx].key = key;
	if (m->data[idx].key) {
		m->data[idx].value = value;
		m->sz++;
		return true;
	}

	return false;
}

void *map_get(map * m, uint16_t key)
{
	if (!m || !key) {
		return NULL;
	}

	size_t idx = key % m->cap;

	while (m->data[idx].key) {
		if (m->data[idx].key == key) {
			return m->data[idx].value;
		}

		idx++;
		idx %= m->cap;
	}
	return NULL;
}

bool map_contains(const map * m, const uint16_t key)
{
	if (!m || !key) {
		return false;
	}

	size_t idx = key % m->cap;

	while (m->data[idx].key) {
		if (m->data[idx].key == key) {
			return true;
		}

		idx++;
		idx %= m->cap;
	}
	return false;
}

void map_destroy(map * m)
{
	if (!m) {
		return;
	}

	for (size_t n = 0; n < m->cap; ++n) {
		if (m->data[n].value) {
			free(m->data[n].value);
		}
	}
	free(m->data);
	free(m);
}

void map_print(map * m)
{
	if (!m) {
		return;
	}

	for (size_t n = 0; n < m->cap; ++n) {
		if (m->data[n].value) {
			printf("%zu %u %u\n", n, m->data[n].key,
				((struct step *)m->data[n].value)->data);
		}

	}
}
