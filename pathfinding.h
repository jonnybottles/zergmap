#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "graph.h"

struct step {
	uint16_t data;
	double distance;

	struct step *next;
};

struct step *path_dijkstra(const graph * g, uint16_t start, uint16_t end);

bool suurballe(graph * g, uint16_t start, uint16_t end);

void print_path(struct step *path);

void path_destroy(struct step *path);

#endif
