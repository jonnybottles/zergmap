#include "pathfinding.h"

#include <math.h>
#include <stdlib.h>

#include "map.h"
#include "priority-queue.h"
#include "set.h"
#include "graph.h"

// Global to get around limited graph_neighbors function
// Use only with path_dijkstra, not any other function!
static pqueue *to_process;
static set *processed;
static map *previous;

// Global to get around limited graph_neighbors function
static double curr_distance;
static uint16_t curr_node;

static struct step *path_clone(struct step *path);

static void visit_node(double neigh_distance, uint16_t data)
{
	if (set_contains(processed, data)) {
		return;
	}

	double known_distance = pqueue_get_priority(to_process, data);

	if (isnan(known_distance)
		|| known_distance > curr_distance + neigh_distance) {
		if (data != curr_node) {
			struct step *curr_step = malloc(sizeof(*curr_step));
			curr_step->data = curr_node;
			curr_step->distance = neigh_distance;
			curr_step->next = NULL;
			// if a faster route was found to ID, free the old step
			if (map_contains(previous, data)) {
				free(map_get(previous, data));
			}
			map_put(previous, data, curr_step);
			pqueue_enqueue(to_process, curr_distance +
					neigh_distance, data);
		}

	}
}

struct step *path_dijkstra(const graph * g, uint16_t start, uint16_t end)
{
	to_process = pqueue_create(MIN);
	if (!to_process) {
		return NULL;
	}

	processed = set_create();
	if (!processed) {
		return NULL;
	}
	previous = map_create();
	if (!previous) {
		return NULL;
	}

	map_put(previous, start, NULL);

	pqueue_enqueue(to_process, 0, start);
	while (!pqueue_is_empty(to_process)) {
		curr_distance = pqueue_get_next_priority(to_process);
		curr_node = pqueue_dequeue(to_process);

		graph_neighbors((graph *) g, curr_node, visit_node);

		set_add(processed, curr_node);

		if (curr_node == end) {
			break;
		}
	}
	struct step *root = NULL;

	uint16_t curr_node = end;
	while (map_contains(previous, curr_node)) {

		struct step *curr_step = map_get(previous, curr_node);
		struct step *copy = path_clone(curr_step);
		if (!curr_step) {
			struct step *final = malloc(sizeof(*final));
			if (!final) {
				pqueue_dequeue(to_process);
				map_destroy(previous);
				set_destroy(processed);
				return NULL;
			}
			final->data = curr_node;
			final->distance = 0;
			final->next = root;
			root = final;
			break;
		}

		const uint16_t tmp = curr_node;
		curr_node = copy->data;
		copy->data = tmp;

		copy->next = root;
		root = copy;
	}

	pqueue_destroy(to_process);
	map_destroy(previous);
	set_destroy(processed);

	return root;
}

struct step *path_clone(struct step *path)
{
	if (!path) {
		return NULL;
	}

	struct step *new = malloc(sizeof(*new));
	new->data = path->data;
	new->distance = path->distance;
	new->next = NULL;

	return new;

}

bool suurballe(graph * g, uint16_t start, uint16_t end)
{
	if (!g) {
		return false;
	}

	struct step *P1 = path_dijkstra(g, start, end);
	if (!P1) {
		return false;
	}

	set *path_nodes = set_create();
	if (!path_nodes) {
		path_destroy(P1);
		return false;
	}
	struct step *path1 = P1;
	// add nodes of shortest path to a set
	// however, we don't want the starting node in the set
	path1 = path1->next;
	while (path1) {
		// we also don't want the ending node in the set
		if (path1->next != NULL) {
			set_add(path_nodes, path1->data);
		}
		path1 = path1->next;
	}
	// reset local path variable to start of shortest path
	// for next iteration
	path1 = P1;
	while (path1 && path1->next) {
		// Make the distance on path1 expensive to travel
		change_edge_distance(g, path1->data, path1->next->data, 9999);
		path1 = path1->next;
	}

	struct step *P2 = path_dijkstra(g, start, end);
	if (!P2) {
		path_destroy(P1);
		set_destroy(path_nodes);
		return false;
	}

	path1 = P1;
	// reset the distances in the graph to what they were before making 
	// them expensive, now that we have the two paths
	while (path1 && path1->next) {
		change_edge_distance(g, path1->data, path1->next->data,
					path1->next->distance);
		path1 = path1->next;
	}

	struct step *path2 = P2;

	bool overlap = false;
	while (path2) {
		if (set_contains(path_nodes, path2->data)) {
			overlap = true;
		}
		path2 = path2->next;
	}
	// if there wasn't overlap, we already have two disjoint paths
	// no need to continue
	if (!overlap) {
		path_destroy(P1);
		path_destroy(P2);
		set_destroy(path_nodes);
		return true;
	}
	// modify overlapping edges to find disjoint paths, if they exist
	path1 = P1->next;
	path2 = P2->next;
	while (path1) {
		// iterate through path2 while holding path1 on a single node
		// modify the edges to overlapping nodes
		while (path2 && path1->next) {
			if (path1->next->data == path2->data
				&& path2->next != NULL) {
				change_edge_distance(g, path1->data,
							path1->next->data, 9999);
			}
			path2 = path2->next;
		}
		// increment path1 to the next node; reset path2
		path1 = path1->next;
		path2 = P2->next;
	}

	path1 = P1->next;
	path2 = P2->next;
	while (path2) {
		// iterate through path1 while holding path1 on a single node
		// modify the edges to overlapping nodes
		while (path1 && path2->next) {
			if (path2->next->data == path1->data
				&& path1->next != NULL) {
				change_edge_distance(g, path2->data,
							path2->next->data, 9999);
			}
			path1 = path1->next;
		}
		// increment path2 to next step; reset path1
		path2 = path2->next;
		path1 = P1->next;
	}
	// new shortest path based on changing the edges of overlapping nodes
	struct step *P3 = path_dijkstra(g, start, end);
	if (!P3) {
		path_destroy(P1);
		path_destroy(P2);
		set_destroy(path_nodes);
		return false;
	}
	// new set for the new shortest path
	set *new_path = set_create();
	if (!new_path) {
		path_destroy(P1);
		path_destroy(P2);
		path_destroy(P3);
		set_destroy(path_nodes);
		return false;
	}
	// add path3 nodes to set, minus the start and end nodes
	struct step *path3 = P3;
	path3 = path3->next;
	while (path3) {
		if (path3->next != NULL) {
			set_add(new_path, path3->data);
		}
		path3 = path3->next;
	}
	// make path3 expensive to travel
	path3 = P3;
	while (path3 && path3->next) {
		change_edge_distance(g, path3->data, path3->next->data, 9999);
		path3 = path3->next;
	}
	// create fourth path, which avoids the expensive path3
	struct step *P4 = path_dijkstra(g, start, end);
	if (!P4) {
		path_destroy(P1);
		path_destroy(P2);
		path_destroy(P3);
		set_destroy(path_nodes);
		set_destroy(new_path);
		return false;
	}
	// change all expensive edges back to what they were before the final
	// check for overlapping nodes
	path3 = P3;
	while (path3 && path3->next) {
		change_edge_distance(g, path3->data, path3->next->data,
					path3->next->distance);
		path3 = path3->next;
	}
	path1 = P1->next;
	path2 = P2->next;
	while (path1) {
		while (path2 && path1->next) {
			if (path1->next->data == path2->data
				&& path2->next != NULL) {
				change_edge_distance(g, path1->data,
							path1->next->data,
							path1->next->distance);
			}
			path2 = path2->next;
		}
		path1 = path1->next;
		path2 = P2->next;
	}

	path1 = P1->next;
	path2 = P2->next;
	while (path2) {
		while (path1 && path2->next) {
			if (path2->next->data == path1->data
				&& path1->next != NULL) {
				change_edge_distance(g, path2->data,
							path2->next->data,
							path2->next->distance);
			}
			path1 = path1->next;
		}
		path2 = path2->next;
		path1 = P1->next;
	}
	// Final check: if there is overlap between path3 and path4
	// then two disjoint paths are not possible
	struct step *path4 = P4;
	while (path4) {
		if (set_contains(new_path, path4->data)) {
			path_destroy(P1);
			path_destroy(P2);
			path_destroy(P3);
			path_destroy(P4);
			set_destroy(path_nodes);
			set_destroy(new_path);
			return false;
		}
		path4 = path4->next;
	}

	path_destroy(P1);
	path_destroy(P2);
	path_destroy(P3);
	path_destroy(P4);
	set_destroy(path_nodes);
	set_destroy(new_path);
	return true;
}

// used for debugging purposes
void print_path(struct step *path)
{
	if (!path) {
		return;
	}
	struct step *curr = path;
	while (curr) {
		printf("%u ", curr->data);
		curr = curr->next;
	}
	puts("\n");
}

void path_destroy(struct step *path)
{
	while (path) {
		struct step *to_delete = path;
		path = path->next;
		free(to_delete);
	}
}
