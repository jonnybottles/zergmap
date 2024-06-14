#include "graph.h"
#include "zergmap.h"
#include "set.h"
#include "map.h"
#include "priority-queue.h"
#include "pathfinding.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

struct edge {
	struct node *to;
	double distance;

	struct edge *next;
};

struct node {
	uint16_t id;
	struct zerg_header *zerg;
	double lat;
	double lon;
	bool health;
	float alt;
	bool kill;
	struct edge *edges;

	struct node *next;
};

struct graph_ {
	struct node *nodes;
};

graph *graph_create()
{
	graph *g = malloc(sizeof(*g));

	if (!g) {
		return NULL;
	}

	g->nodes = NULL;
	return g;
}

graph *graph_clone(graph * g)
{
	if (!g) {
		return NULL;
	}

	graph *new = malloc(sizeof(*new));
	if (!new) {
		return NULL;
	}

	new->nodes = NULL;

	struct node *curr = g->nodes;
	while (curr) {
		graph_add_vertex(new, curr->zerg);

		curr = curr->next;
	}
	curr = g->nodes;

	while (curr) {
		struct edge *curr_edge = curr->edges;
		while (curr_edge) {
			graph_add_edge(new, curr->id, curr_edge->to->id,
					curr_edge->distance);

			curr_edge = curr_edge->next;
		}
		curr = curr->next;
	}

	return new;
}

// a vertex holds the zerg id, coordinates, a pointer to other zerg data, such
// as hit points, a flag indicating whether to remove, and initializes edge
// data. Vertexes prepend to the start of the graph.
bool graph_add_vertex(graph * g, struct zerg_header *zerg)
{
	if (!g || !zerg) {
		return false;
	}

	struct node *curr = g->nodes;

	while (curr) {
		if (curr->id == zerg->srcid) {
			return false;
		}

		curr = curr->next;
	}

	struct node *new = NULL;
	new = malloc(sizeof(*new));
	if (!new) {
		return false;
	}

	new->id = zerg->srcid;
	new->lat = NAN;
	new->lon = NAN;
	new->alt = NAN;
	if (zerg->gps_payload != NULL) {
		new->lat = zerg->gps_payload->latitude;
		new->lon = zerg->gps_payload->longitude;
		new->alt = zerg->gps_payload->altitude;
	}

	new->kill = false;
	new->health = false;
	if (zerg->low_health) {
		new->health = true;
	}
	new->zerg = zerg;

	new->edges = NULL;

	new->next = g->nodes;
	g->nodes = new;

	return true;
}

// update the coordinates of a zerg id already in the graph,
// checks if the difference between the old and new coordinates
// is outside the accuracy value
bool graph_update_coord(graph * g, uint16_t id, struct zerg_header *zerg)
{
	if (!g || !zerg || !zerg->gps_payload) {
		return false;
	}

	struct node *curr = g->nodes;

	while (curr) {
		if (curr->id == id) {
			double difference = haversine(curr->lat, curr->lon,
							zerg->gps_payload->
							latitude,
							zerg->gps_payload->
							longitude);
			// the haversine function returns kilometers; need value
			// in meters for this comparison
			if (difference * 1000 > zerg->gps_payload->accuracy) {
				return false;
			} else {
				curr->lat = zerg->gps_payload->latitude;
				curr->lon = zerg->gps_payload->longitude;
				curr->alt = zerg->gps_payload->altitude;
				return true;
			}
		}

		curr = curr->next;
	}

	return false;
}

// checks if the graph has the specified zerg ID
bool graph_contains(const graph * g, uint16_t data)
{
	if (!g) {
		return false;
	}

	struct node *curr = g->nodes;
	while (curr) {
		if (curr->id == data) {
			return true;
		}

		curr = curr->next;
	}

	return false;
}

// returns total number of nodes in the graph
size_t zerg_count(graph * g)
{
	if (!g) {
		return 0;
	}
	size_t count = 0;
	struct node *curr = g->nodes;

	while (curr) {
		count++;
		curr = curr->next;
	}
	return count;
}

bool graph_mark_remove(graph * g, uint16_t data)
{
	if (!g) {
		return false;
	}

	struct node *curr = g->nodes;
	while (curr) {
		if (curr->id == data) {
			curr->kill = true;
			return true;
		}
		curr = curr->next;
	}

	return false;
}

bool graph_is_marked(graph * g, uint16_t data)
{
	if (!g) {
		return false;
	}
	struct node *curr = g->nodes;
	while (curr) {
		if (curr->id == data) {
			if (curr->kill == true) {
				return true;
			} else {
				return false;
			}
		}
		curr = curr->next;
	}

	return false;
}

// checks to see if a zerg is below a certain percentage of health
bool low_health(graph * g, uint16_t data, double percent)
{
	if (!g) {
		return false;
	}

	struct node *curr = g->nodes;
	while (curr) {
		if (curr->id == data) {
			if ((double)curr->zerg->status_payload->hit_points
				/ (double)curr->zerg->status_payload->max_hit_points
				< percent / 100) {
				return true;
			}
		}
		curr = curr->next;
	}

	return false;
}

// iterates to the specified node, removes its edges, deletes the node
void graph_remove_vertex(graph * g, uint16_t data)
{
	if (!g || !data) {
		return;
	}

	struct node *to_delete = NULL;

	struct node **curr = &g->nodes;
	while (*curr) {
		if ((*curr)->id == data) {
			to_delete = *curr;

			struct node *node_with_edges = g->nodes;
			while (node_with_edges) {
				struct edge *possible_incoming_edge =
					node_with_edges->edges;
				struct edge **curr_edge =
					&node_with_edges->edges;

				while (possible_incoming_edge) {
					if (possible_incoming_edge->to ==
						to_delete) {
						*curr_edge =
							possible_incoming_edge->
							next;
						free(possible_incoming_edge);
						break;
					}
					curr_edge =
						&possible_incoming_edge->next;
					possible_incoming_edge =
						possible_incoming_edge->next;
				}

				node_with_edges = node_with_edges->next;
			}

			*curr = to_delete->next;

			struct edge *curr_edge = to_delete->edges;
			while (curr_edge) {
				struct edge *tmp = curr_edge->next;
				free(curr_edge);

				curr_edge = tmp;
			}

			free(to_delete);
			return;
		}

		curr = &(*curr)->next;
	}
}

// adds a connection and distance between a source and destination
bool graph_add_edge(graph * g, const uint16_t src, const uint16_t dst,
			double dist)
{
	if (!g || !src || !dst || isnan(dist)) {
		return false;
	}

	struct node *src_node = NULL;
	struct node *dst_node = NULL;

	struct node *curr = g->nodes;
	// While there are nodes left to check,
	// and we have not found the nodes for the
	// src or dst values, keep looking
	while (curr && (!src_node || !dst_node)) {
		if (curr->id == src) {
			src_node = curr;
		}

		if (curr->id == dst) {
			dst_node = curr;
		}

		curr = curr->next;
	}

	// If one of the two can't be found, don't add
	if (!src_node || !dst_node) {
		return false;
	}

	struct edge *duplicate_edge_detector = src_node->edges;
	while (duplicate_edge_detector) {
		if (duplicate_edge_detector->to == dst_node) {
			return false;
		}

		duplicate_edge_detector = duplicate_edge_detector->next;
	}

	struct edge *new = malloc(sizeof(*new));
	if (!new) {
		return false;
	}

	new->to = dst_node;
	new->distance = dist;

	new->next = src_node->edges;
	src_node->edges = new;

	return true;
}

// calculates the distance between two coordinates
double haversine(double lat1, double lat2, double lon1, double lon2)
{
	double dlon = 0;
	double dlat = 0;
	// degrees to radian conversion
	double rad = 3.141592653589793 / 180;

	// take the difference of the two latitudes and longitudes
	dlon = lon2 - lon1;
	dlat = lat2 - lat1;

	// The variables a, c, and d split up the Haversine equation into
	// multiple steps. 'a' calculates everything underneath the giant
	// square root; 'c' takes the arcsine of the squareroot of a's result
	// and multiplies by 2. The final step is to multiply by the mean
	// radius of the planet.

	double a = 0;
	double c = 0;
	double d = 0;
	a = pow(sin(dlat / 2 * rad), 2) + cos(lat2 * rad)
		* cos(lat1 * rad) * pow(sin(dlon / 2 * rad), 2);

	c = 2 * asin(sqrt(a));

	// multiply by radius of planet
	d = 6371 * c;

	return d;
}

// returns the distance between a source and destination id
double graph_edge_distance(const graph * g, const uint16_t src,
			const uint16_t dst)
{
	if (!g || !src || !dst) {
		return NAN;
	}

	struct node *curr_node = g->nodes;
	while (curr_node) {
		if (curr_node->id == src) {
			struct edge *curr_edge = curr_node->edges;

			while (curr_edge) {
				if (curr_edge->to->id == dst) {
					return curr_edge->distance;
				}

				curr_edge = curr_edge->next;
			}

			return NAN;
		}

		curr_node = curr_node->next;
	}

	return NAN;
}

// changes the distance between a source and destination, used in suurballe
bool change_edge_distance(const graph * g, const uint16_t src,
			const uint16_t dst, double dist)
{
	if (!g || isnan(dist)) {
		return false;
	}

	struct node *curr_node = g->nodes;
	while (curr_node) {
		if (curr_node->id == src) {
			struct edge *curr_edge = curr_node->edges;
			while (curr_edge) {
				if (curr_edge->to->id == dst) {
					curr_edge->distance = dist;
					return true;
				}
				curr_edge = curr_edge->next;
			}
		}
		curr_node = curr_node->next;
	}
	return false;
}

// removes an edge between a source and destination node
void graph_remove_edge(graph * g, const uint16_t src, const uint16_t dst)
{
	if (!g || !src || !dst) {
		return;
	}

	struct node *src_node = g->nodes;
	while (src_node) {
		if (src_node->id == src) {

			struct edge **curr = &src_node->edges;

			while (*curr) {
				if ((*curr)->to->id == dst) {
					struct edge *to_delete = *curr;

					*curr = to_delete->next;

					free(to_delete);
					return;
				}

				curr = &(*curr)->next;
			}

		}

		src_node = src_node->next;
	}

}

// iterates through the graph and executes a user-specified function
void graph_iterate_vertices(graph * g, void (*func)(uint16_t id))
{
	if(!g || !func) {
		return;
	}

	struct node *curr = g->nodes;

	while (curr) {
		func(curr->id);

		curr = curr->next;
	}
}

// returns number of nodes a node has edges to, used in djikstra algorithm
// the *func is intended to be the visit_node function in pathfinding.c
size_t graph_neighbors(graph * g, const uint16_t vertex,
			void (*func)(double distance, uint16_t dst))
{
	if(!g || !vertex) {
		return 0;
	}

	struct node *curr = g->nodes;

	while (curr) {
		if (curr->id == vertex) {

			size_t count = 0;
			struct edge *curr_edge = curr->edges;
			while (curr_edge) {
				if (func) {
					func(curr_edge->distance,
						 curr_edge->to->id);
				}

				++count;
				curr_edge = curr_edge->next;
			}

			return count;
		}

		curr = curr->next;
	}

	return 0;
}

// returns a priority queue containing all candidates to remove and the number
// of connections they have, including immediate neighbors and nodes that
// have two disjoint paths
pqueue *check_connectivity(graph * g)
{
	if (!g) {
		return NULL;
	}

	pqueue *to_remove = pqueue_create(MIN);

	graph *new = g;

	struct node *curr = new->nodes;
	uint16_t saved_id = 0;
	while (curr) {
		double count = 0;
		saved_id = curr->id;
		struct node *to_iterate = new->nodes;
		while (to_iterate) {
			if (curr->id != to_iterate->id &&
				suurballe(new, curr->id, to_iterate->id)) {
				count++;
			}
			to_iterate = to_iterate->next;
		}
		pqueue_enqueue(to_remove, count, saved_id);

		curr = curr->next;
	}

	return to_remove;
}

// used output at end of program; either it displays zerg IDs that should be
// removed, indicate that too many removals would be required, or that the
// graph is complete, meaning that none should be removed
void recommend_removal(graph * g, pqueue * pq, size_t zerg_count)
{
	if (!g || !pq) {
		return;
	}
	size_t total_recommended = 0;
	set *remove = set_create();

	for (size_t n = 0; n < zerg_count; n++) {
		double priority = pqueue_get_next_priority(pq);
		uint16_t zerg = pqueue_dequeue(pq);
		// in a complete graph, all nodes should have nodes - 1 number
		// of connections; decrease the priority threshold with each
		// ID added
		if (priority < zerg_count - 1 - total_recommended
			&& zerg_count > 1) {
			set_add(remove, zerg);
			graph_remove_vertex(g, zerg);
			total_recommended++;
		}

		pqueue *new = check_connectivity(g);
		pqueue_destroy(pq);
		pq = new;

		if (total_recommended > zerg_count / 2) {
			puts("Total removals exceeds half the total zerg\n");
			pqueue_destroy(pq);
			set_destroy(remove);
			return;
		}
	}

	if (total_recommended == 0) {
		printf("Zerg are fully connected\n");
		pqueue_destroy(pq);
		set_destroy(remove);
		return;
	}

	printf("Recommended Zerg IDs to remove: \n");
	print_set(remove);
	pqueue_destroy(pq);
	set_destroy(remove);

}

// displays zerg ids that are below the specified health threshold
void show_low_health_zerg(graph * g)
{
	if (!g) {
		return;
	}
	bool print = false;
	set *low = set_create();
	struct node *curr = g->nodes;

	while (curr) {
		if (curr->health) {
			set_add(low, curr->id);
			print = true;
		}
		curr = curr->next;
	}

	if (print) {
		printf("Low Health: \n");
		print_set(low);
	}
	set_destroy(low);
}

// displays zerg that are not connected to other zerg
void show_no_gps(graph * g)
{
	if (!g) {
		return;
	}
	bool print = false;
	set *no_gps = set_create();
	struct node *curr = g->nodes;

	while (curr) {
		if (isnan(curr->lat) && isnan(curr->lon)) {
			set_add(no_gps, curr->id);
			print = true;
		}
		curr = curr->next;
	}

	if (print) {
		printf("Zerg IDs not connected to other Zerg: \n");
		print_set(no_gps);
	}
	set_destroy(no_gps);
}

// used for debugging purposes
void graph_print(graph * g)
{
	if (!g) {
		return;
	}

	struct node *curr = g->nodes;
	while (curr) {
		printf("%u: ", curr->id);
		struct edge *curr_edge = curr->edges;
		while (curr_edge) {
			printf("%u, ", curr_edge->to->id);
			curr_edge = curr_edge->next;
		}
		puts("");
		curr = curr->next;
	}
}

// frees edges, frees nodes, frees graph
void graph_destroy(graph * g)
{
	struct node *curr_node = g->nodes;

	while (curr_node) {
		struct node *next = curr_node->next;

		struct edge *curr_edge = curr_node->edges;
		while (curr_edge) {
			struct edge *tmp = curr_edge->next;
			free(curr_edge);

			curr_edge = tmp;
		}

		free(curr_node);

		curr_node = next;
	}

	free(g);
}
