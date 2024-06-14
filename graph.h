#ifndef GRAPH_H
#define GRAPH_H

#include "zergmap.h"
#include "map.h"
#include "priority-queue.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

typedef struct graph_ graph;

graph *graph_create();

graph *graph_clone(graph * g);

bool graph_add_vertex(graph * g, struct zerg_header *zerg);

bool graph_update_coord(graph * g, uint16_t id, struct zerg_header *zerg);

bool graph_contains(const graph * g, uint16_t data);

size_t zerg_count(graph * g);

bool graph_mark_remove(graph * g, uint16_t data);

bool graph_is_marked(graph * g, uint16_t data);

bool low_health(graph * g, uint16_t data, double percent);

void graph_remove_vertex(graph * g, uint16_t data);

bool graph_add_edge(graph * g, const uint16_t src, const uint16_t dst,
			double dist);

double haversine(double lat1, double lon1, double lat2, double lon2);

double graph_edge_distance(const graph * g, const uint16_t src,
			const uint16_t dst);

bool change_edge_distance(const graph * g, const uint16_t src,
			const uint16_t dst, double dist);

void graph_remove_edge(graph * g, const uint16_t src, const uint16_t dst);

void graph_iterate_vertices(graph * g, void (*func)(uint16_t id));

// Returns the number of neighbors of vertex
size_t graph_neighbors(graph * g, const uint16_t vertex,
			void (*func)(double distance, uint16_t dst));

pqueue *check_connectivity(graph * g);

void recommend_removal(graph * g, pqueue * pq, size_t zerg_count);

void show_low_health_zerg(graph * g);

void show_no_gps(graph * g);

void graph_print(graph * g);

void graph_destroy(graph * g);

#endif
