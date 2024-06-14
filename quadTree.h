#ifndef QUADTREE_H
#define QUADTREE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "zergmap.h"
#include "graph.h"

typedef struct point point;
typedef struct boundingBox box;
typedef struct quadTree qt;

// Creates and returns a point object.
point *create_point(double lat, double lon);

// Creates and returns a point object for Zerg points.
point *create_zerg_point(struct zerg_header *z_hdr);

// Prints the point object to stdout.
void point_print(point * p);

// Creates and returns a new bounding box object.
box *create_box(point * center, double halfDimension);

// Returns true or false depending on if the point is located in the bounding
// box.
bool box_contains_point(box * boundry, point * point);

// Returns true or false if one bounding box intersects the second bounding
// box.
bool box_intersects(box * self, box * check);

// Creates and returns a new quadTree object.
qt *create_quadTree(box * boundry);

// Frees memory allocated for point objects.
void point_destroy(point * p);

// Frees memory allocated for boxes.
void box_destroy(box * b);

// Frees memory allocated for quadTree.
void qt_destroy(qt * qt);

// Returns a size_t count of how many points are in the quadTree.
size_t number_of_points(qt * qt);

// This function is used with the quadTree to divide into regions.
qt *subdivide(qt * root);

// Returns true or false if a point is inserted into the quadTree.
bool qt_insert(qt * root, point * point);

// Returns an array of point objects that are in the specified search box.
point **quadTree_search(qt * root, box * range);

// Prints results after performing a search.
void print_search_results(point ** result);

// Frees memory allocated for results array.
void result_destroy(point ** result);

// Prints points within a quadTree.
void print_quadTree_points(qt * qt, graph * g);

// Adds edges to graph.
void point_add_edges(graph * g, qt * qt, point ** result);

// Returns difference for two given altitude values.
uint32_t return_alt(uint32_t alt, uint32_t alt2);
#endif
