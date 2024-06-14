#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "graph.h"
#include "set.h"
#include "map.h"
#include "pathfinding.h"

START_TEST(test_create_destroy)
{
	graph *g = graph_create(NULL);
	ck_assert_ptr_nonnull(g);
	graph_destroy(g);
}

END_TEST START_TEST(test_add_remove_vertices)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 1;
	zerg1->version = 4;
	zerg1->len = 24;
	zerg1->dstid = 100;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 1;
	zerg2->version = 4;
	zerg2->len = 24;
	zerg2->dstid = 100;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	ck_assert(graph_add_vertex(g, zerg1));
	ck_assert(graph_add_vertex(g, zerg2));

	ck_assert(graph_contains(g, 10));
	ck_assert(graph_contains(g, 11));
	ck_assert(!graph_contains(g, 12));

	graph_add_edge(g, 11, 10, 3.14);
	graph_add_edge(g, 10, 11, 2.71);

	graph_remove_vertex(g, 11);

	ck_assert(graph_contains(g, 10));
	ck_assert(!graph_contains(g, 11));

	ck_assert_double_nan(graph_edge_distance(g, 10, 11));

	free(zerg1);
	free(zerg2);

	graph_destroy(g);
}

END_TEST START_TEST(test_graph_capacity)
{
	graph *g = graph_create(NULL);

	enum { OVERCAPACITY = 24 };

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 1;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	//XXX This should cause a resize in a list of capacity 16 or less
	for (size_t n = 1; n < OVERCAPACITY; ++n) {
		ck_assert(graph_add_vertex(g, zerg1));
		zerg1->srcid++;
	}

	for (size_t n = 1; n < OVERCAPACITY; ++n) {
		ck_assert(graph_contains(g, n));
		for (size_t m = 1; m < OVERCAPACITY; ++m) {
			ck_assert(graph_add_edge(g, n, m, n * m));
		}
	}

	for (size_t n = 1; n < OVERCAPACITY; ++n) {
		for (size_t m = 1; m < OVERCAPACITY; ++m) {
			ck_assert_double_eq_tol(graph_edge_distance(g, n, m),
						n * m, 0.0001);
		}
	}

	free(zerg1);
	graph_destroy(g);
}

END_TEST START_TEST(test_no_multiple_vertices)
{
	graph *g = graph_create(NULL);

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	ck_assert(graph_add_vertex(g, zerg1));
	ck_assert(graph_add_vertex(g, zerg2));

	ck_assert(!graph_add_vertex(g, zerg1));
	ck_assert(graph_contains(g, 10));

	graph_remove_vertex(g, 10);
	ck_assert(!graph_contains(g, 10));

	free(zerg1);
	free(zerg2);
	graph_destroy(g);
}

END_TEST START_TEST(test_no_duplicate_edges)
{
	graph *g = graph_create(NULL);

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 12;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	ck_assert(graph_add_vertex(g, zerg1));
	ck_assert(graph_add_vertex(g, zerg2));
	ck_assert(graph_add_vertex(g, zerg3));

	ck_assert(graph_add_edge(g, zerg1->srcid, zerg2->srcid, 3.14));
	ck_assert(graph_add_edge(g, zerg2->srcid, zerg1->srcid, 2.71));

	ck_assert(!graph_add_edge(g, 10, 11, 6.02));

	free(zerg1);
	free(zerg2);
	free(zerg3);
	graph_destroy(g);
}

END_TEST START_TEST(test_add_edge)
{
	graph *g = graph_create(NULL);

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);

	graph_add_edge(g, 10, 11, 3.14);

	ck_assert_double_eq_tol(graph_edge_distance(g, 10, 11), 3.14, 0.00001);
	ck_assert_double_nan(graph_edge_distance(g, 11, 10));
	ck_assert_double_nan(graph_edge_distance(g, 11, 12));
	ck_assert_double_nan(graph_edge_distance(g, 12, 11));

	graph_remove_edge(g, 10, 11);
	ck_assert_double_nan(graph_edge_distance(g, 10, 11));

	free(zerg1);
	free(zerg2);
	graph_destroy(g);
}

END_TEST static uint16_t total = 0;
static void vert_sum(uint16_t data)
{
	total += data;
}

START_TEST(test_iterate_vertices)
{
	graph *g = graph_create(NULL);

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 2;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 3;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 5;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);
	graph_add_vertex(g, zerg3);

	graph_iterate_vertices(g, vert_sum);
	ck_assert_uint_eq(total, 10);

	free(zerg1);
	free(zerg2);
	free(zerg3);
	graph_destroy(g);
}

END_TEST START_TEST(test_vertex_equivalency)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 10;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	ck_assert(!graph_add_vertex(g, zerg2));

	free(zerg1);
	free(zerg2);
	graph_destroy(g);
}

END_TEST static double sum_total = 0;
static void sum(double distance, uint16_t dst)
{
	//dst is totally unused for this test
	(void)dst;

	sum_total += distance;
}

START_TEST(test_neighbors)
{
	graph *g = graph_create(NULL);

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 12;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);
	graph_add_vertex(g, zerg3);

	graph_add_edge(g, 10, 11, 3.14);
	graph_add_edge(g, 10, 12, 2.71);
	graph_add_edge(g, 12, 10, 6.02);

	ck_assert_uint_eq(graph_neighbors(g, 10, NULL), 2);
	ck_assert_uint_eq(graph_neighbors(g, 12, NULL), 1);
	ck_assert_uint_eq(graph_neighbors(g, 11, NULL), 0);

	ck_assert_uint_eq(graph_neighbors(g, 13, NULL), 0);

	graph_neighbors(g, 10, sum);
	ck_assert_double_eq_tol(3.14 + 2.71, sum_total, 0.0001);

	free(zerg1);
	free(zerg2);
	free(zerg3);
	graph_destroy(g);
}

END_TEST START_TEST(test_change_distance)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 12;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);
	graph_add_vertex(g, zerg3);

	graph_add_edge(g, 10, 11, 3.14);
	graph_add_edge(g, 10, 12, 2.71);
	graph_add_edge(g, 12, 10, 6.02);

	ck_assert_double_eq(graph_edge_distance(g, 10, 11), 3.14);
	ck_assert(change_edge_distance(g, 10, 11, 5.55));
	ck_assert_double_eq(graph_edge_distance(g, 10, 11), 5.55);
	ck_assert(!change_edge_distance(g, 12, 11, 99.1));
	ck_assert_double_eq(graph_edge_distance(g, 12, 10), 6.02);
	ck_assert_double_eq(graph_edge_distance(g, 10, 12), 2.71);

	free(zerg1);
	free(zerg2);
	free(zerg3);
	graph_destroy(g);
}

END_TEST START_TEST(test_low_health)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->status_payload = malloc(sizeof(*zerg1->status_payload));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;
	zerg1->status_payload->hit_points = 5;
	zerg1->status_payload->max_hit_points = 100;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->status_payload = malloc(sizeof(*zerg2->status_payload));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;
	zerg2->status_payload->hit_points = 55;
	zerg2->status_payload->max_hit_points = 100;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);

	ck_assert(low_health(g, 10, 10));
	ck_assert(!low_health(g, 11, 10));
	// should return false because id 12 is not in the graph
	ck_assert(!low_health(g, 12, 10));

	free(zerg1->status_payload);
	free(zerg1);
	free(zerg2->status_payload);
	free(zerg2);
	graph_destroy(g);
}

END_TEST START_TEST(test_haversine)
{
	ck_assert_double_nan(haversine(NAN, 100.01, 30.2, 101.05));
	ck_assert_double_nan(haversine(29.1, NAN, 30.2, 101.05));
	ck_assert_double_nan(haversine(29.1, 100.01, NAN, 101.05));
	ck_assert_double_nan(haversine(29.1, 100.01, 30.2, NAN));

	ck_assert_double_eq(haversine(10.0, 10.0, 100.0, 100.0), 0.0);

	double distance = haversine(30.0, 35.0, 100.0, 101.0);
	// check that corresponding negative differences get the same result
	ck_assert_double_eq(haversine(35.0, 30.0, 100.0, 101.0), distance);
	ck_assert_double_eq_tol(haversine(30.0, 35.0, 101.0, 100.0),
				distance, 0.0001);
}

END_TEST START_TEST(test_graph_clone)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 12;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);
	graph_add_vertex(g, zerg3);

	graph_add_edge(g, 10, 11, 3.14);
	graph_add_edge(g, 10, 12, 2.71);
	graph_add_edge(g, 12, 10, 6.02);

	graph *new = graph_clone(g);

	ck_assert(graph_contains(new, 10));
	ck_assert(graph_contains(new, 11));
	ck_assert(graph_contains(new, 12));

	ck_assert_double_eq(graph_edge_distance(new, 10, 11), 3.14);
	ck_assert_double_eq(graph_edge_distance(new, 10, 12), 2.71);
	ck_assert_double_eq(graph_edge_distance(new, 12, 10), 6.02);

	free(zerg1);
	free(zerg2);
	free(zerg3);
	graph_destroy(new);
	graph_destroy(g);
}

END_TEST START_TEST(test_connectivity)
{
	graph *g = graph_create();

	struct zerg_header *zerg1 = NULL;
	zerg1 = malloc(sizeof(*zerg1));
	zerg1->srcid = 10;
	zerg1->type = 0;
	zerg1->version = 0;
	zerg1->len = 0;
	zerg1->dstid = 0;
	zerg1->sequence_id = 0;
	zerg1->status_payload = NULL;
	zerg1->gps_payload = NULL;
	zerg1->payload_sz = 0;
	zerg1->health_flag = false;
	zerg1->low_health = false;
	zerg1->health_threshold = 0;

	struct zerg_header *zerg2 = NULL;
	zerg2 = malloc(sizeof(*zerg2));
	zerg2->srcid = 11;
	zerg2->type = 0;
	zerg2->version = 0;
	zerg2->len = 0;
	zerg2->dstid = 0;
	zerg2->sequence_id = 0;
	zerg2->status_payload = NULL;
	zerg2->gps_payload = NULL;
	zerg2->payload_sz = 0;
	zerg2->health_flag = false;
	zerg2->low_health = false;
	zerg2->health_threshold = 0;

	struct zerg_header *zerg3 = NULL;
	zerg3 = malloc(sizeof(*zerg3));
	zerg3->srcid = 12;
	zerg3->type = 0;
	zerg3->version = 0;
	zerg3->len = 0;
	zerg3->dstid = 0;
	zerg3->sequence_id = 0;
	zerg3->status_payload = NULL;
	zerg3->gps_payload = NULL;
	zerg3->payload_sz = 0;
	zerg3->health_flag = false;
	zerg3->low_health = false;
	zerg3->health_threshold = 0;

	graph_add_vertex(g, zerg1);
	graph_add_vertex(g, zerg2);
	graph_add_vertex(g, zerg3);

	graph_add_edge(g, 10, 11, 3.14);
	graph_add_edge(g, 10, 12, 2.71);
	graph_add_edge(g, 12, 10, 6.02);

	pqueue *candidates = check_connectivity(g);

	ck_assert_uint_eq(pqueue_dequeue(candidates), 11);
	ck_assert_uint_eq(pqueue_dequeue(candidates), 12);
	ck_assert_uint_eq(pqueue_dequeue(candidates), 10);

	free(zerg1);
	free(zerg2);
	free(zerg3);

	pqueue_destroy(candidates);
	graph_destroy(g);
}

END_TEST Suite * test_graph_suite(void)
{
	// This list needs to be inside a function
	// Type needs to be const TTest * instead of TFun
	const TTest *core_tests[] = {
		test_create_destroy,
		test_add_remove_vertices,
		test_add_edge,
		test_iterate_vertices,
		test_no_multiple_vertices,
		test_no_duplicate_edges,
		test_vertex_equivalency,
		test_neighbors,
		test_graph_capacity,
		test_change_distance,
		test_low_health,
		test_haversine,
		test_graph_clone,
		test_connectivity,
		NULL
	};

	// Test case collects unit tests
	TCase *tc = tcase_create("Core");

	const TTest **current = core_tests;
	while (*current) {
		tcase_add_test(tc, *current);
		++current;
	}

	// suite collects test cases
	Suite *s = suite_create("Graph");
	suite_add_tcase(s, tc);

	return s;
}
