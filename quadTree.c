#include "quadTree.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

// Most of the implementation for the quadTree came from here:
// https://github.com/TheKeySpammer/QuadTree-Visualization

struct point {
	uint16_t zerg_id;
	double lat;
	double lon;
	uint32_t alt;
};

struct boundingBox {
	point *center;
	double halfDimension;
};

struct quadTree {
	box *boundry;
	point **points;
	size_t sz;

	struct quadTree *NW;
	struct quadTree *NE;
	struct quadTree *SW;
	struct quadTree *SE;
};

// Creates and returns a point object.
point *create_point(double lat, double lon)
{
	// Dynamically allocates space for point object.
	point *p = malloc(sizeof(*p));
	if (!p) {
		printf("Unable to allocate space!\n");
		return NULL;
	}
	// Assigns lat / lon values to point object.
	p->lat = lat;
	p->lon = lon;
	p->alt = 0;
	p->zerg_id = 0;

	return p;
}

// Creates and returns a point object for Zerg points.
point *create_zerg_point(struct zerg_header *z_hdr)
{
	// Designates enum for payload types.
	enum PAYLOAD_TYPE { MESSAGE, STATUS, COMMAND, GPS };

	// Dynamically allocates space for point object.
	point *p = malloc(sizeof(*p));
	if (!p) {
		printf("Unable to allocate space!\n");
		return NULL;
	}

	p->zerg_id = z_hdr->srcid;
	p->alt = 0;
	p->lat = 0;
	p->lon = 0;

	// If Zerg payload type is message or command, return point
	// as no GPS coordinates will be present.
	if (z_hdr->type == MESSAGE || z_hdr->type == COMMAND
		|| z_hdr->type == STATUS) {
		return p;
	}
	// If / else statements evaluate of a given lat / long are greater
	// than zero, which determines whether a given lat / long are prepended
	// with a negative sign, determining NORTH / SOUTH / EAST / WEST values. 
	if (z_hdr->gps_payload->latitude_double > 0) {
		p->lat = z_hdr->gps_payload->latitude_double;
	} else {
		p->lat = -z_hdr->gps_payload->latitude_double;
	}
	if (z_hdr->gps_payload->longitude_double > 0) {
		p->lon = z_hdr->gps_payload->longitude_double;
	} else {
		p->lon = -z_hdr->gps_payload->longitude_double;
	}

	// Assigns Zerg header altitude value to point alt.
	p->alt = z_hdr->gps_payload->altitude_float;

	return p;
}

// Prints the point object to stdout.
void point_print(point * p)
{
	printf("Zerg ID %d lat %lf lon %lf\n", p->zerg_id, p->lat, p->lon);
}

// Creates and returns a new bounding box object.
box *create_box(point * center, double halfDimension)
{
	// Dynamically allocates space for box object.
	box *a = malloc(sizeof(*a));
	if (!a) {
		printf("Unable to allocate space!\n");
		return NULL;
	}
	// Assigns center / halfDimension values to box.
	a->center = center;
	a->halfDimension = halfDimension;

	return a;
}

// Returns true or false depending on if the point is located in the bounding
// box.
bool box_contains_point(box * boundry, point * point)
{
	if (point->lat < boundry->center->lat - boundry->halfDimension
		|| point->lat > boundry->center->lat + boundry->halfDimension) {
		return false;
	}

	if (point->lon < boundry->center->lat - boundry->halfDimension
		|| point->lon > boundry->center->lon + boundry->halfDimension) {
		return false;
	}

	return true;
}

// Returns true or false if one bounding box intersects the second bounding
// box.
bool box_intersects(box * self, box * check)
{
	if (self->center->lat + self->halfDimension >
		check->center->lat - check->halfDimension) {
		return true;
	}
	if (self->center->lat - self->halfDimension <
		check->center->lat + check->halfDimension) {
		return true;
	}
	if (self->center->lon + self->halfDimension >
		check->center->lon - check->halfDimension) {
		return true;
	}
	if (self->center->lon - self->halfDimension <
		check->center->lon + check->halfDimension) {
		return true;
	}

	return false;
}

// Creates and returns a new quadTree object.
qt *create_quadTree(box * boundry)
{
	// Dynamically allocates space for quadTree.
	qt *qt = malloc(sizeof(*qt));
	if (!qt) {
		printf("Unable to allocate space!\n");
		return NULL;
	}
	qt->NW = NULL;
	qt->NE = NULL;
	qt->SW = NULL;
	qt->SE = NULL;
	// 2048 seemed like a large enough number.
	qt->sz = 2048;

	qt->boundry = boundry;

	// Dynamically allocates space for quadTree points.
	qt->points = malloc(sizeof(*qt->points) * qt->sz);
	if (!qt->points) {
		printf("Unable to allocate space!\n");
		free(qt);
		return NULL;
	}
	// initializes all the points to NULL.
	for (size_t i = 0; i < qt->sz; ++i) {
		qt->points[i] = NULL;
	}

	return qt;
}

// Frees memory allocated for point objects.
void point_destroy(point * p)
{
	if (!p) {
		return;
	}
	free(p);
}

// Frees memory allocated for boxes.
void box_destroy(box * b)
{
	if (!b) {
		return;
	}
	point_destroy(b->center);
	free(b);
}

// Frees memory allocated for quadTree.
void qt_destroy(qt * qt)
{

	if (!qt) {
		return;
	}

	for (size_t i = 0; i < qt->sz; ++i) {
		free(qt->points[i]);

	}

	free(qt->points);
	box_destroy(qt->boundry);
	qt_destroy(qt->NW);
	qt_destroy(qt->NE);
	qt_destroy(qt->SW);
	qt_destroy(qt->SE);
	free(qt);
}

// Frees memory allocated for results array.
void result_destroy(point ** result)
{;
	for (size_t i = 2048; i > 0; --i) {
		point_destroy(result[i]);
	}
	free(result);
}

// Returns a size_t count of how many points are in the quadTree.
size_t number_of_points(qt * qt)
{
	size_t i;

	for (i = 0; i < qt->sz; ++i) {
		if (qt->points[i] == NULL) {
			return i;
		}
	}
	return i;
}

// This function is used with the quadTree to divide into regions.
qt *subdivide(qt * root)
{
	double halfDimension = root->boundry->halfDimension / 2;

	point *nw_point =
		create_point(root->boundry->center->lat - halfDimension,
			root->boundry->center->lon + halfDimension);
	root->NW = create_quadTree(create_box(nw_point, halfDimension));

	point *ne_point =
		create_point(root->boundry->center->lat + halfDimension,
			root->boundry->center->lon + halfDimension);
	root->NE = create_quadTree(create_box(ne_point, halfDimension));

	point *sw_point =
		create_point(root->boundry->center->lat - halfDimension,
			root->boundry->center->lon - halfDimension);
	root->SW = create_quadTree(create_box(sw_point, halfDimension));

	point *se_point =
		create_point(root->boundry->center->lat + halfDimension,
			root->boundry->center->lon - halfDimension);
	root->SE = create_quadTree(create_box(se_point, halfDimension));

	return root;
}

// Returns true or false if a point is inserted into the quadTree.
bool qt_insert(qt * root, point * point)
{
	if (!box_contains_point(root->boundry, point)) {
		return false;
	}

	size_t points_size = number_of_points(root);

	if (points_size < root->sz && root->NW == NULL) {
		root->points[points_size] = point;
		return true;
	}

	if (root->NW == NULL) {
		subdivide(root);
	}

	if (qt_insert(root->NW, point)) {
		return true;
	}
	if (qt_insert(root->NE, point)) {
		return true;
	}
	if (qt_insert(root->SW, point)) {
		return true;
	}
	if (qt_insert(root->SE, point)) {
		return true;
	}

	return false;
}

// Returns an array of point objects that are in the specified search box.
point **quadTree_search(qt * root, box * range)
{
	// 2048 was picked randomly to allow a huge array.
	point **result = NULL;
	result = malloc(sizeof(*result) * 2048);
	if (!result) {
		printf("Unable to allocate memory!\n");
		return NULL;
	}
	// Initializes the points to NULL.
	size_t index = 0;
	for (size_t i = 0; i < 2048; ++i) {
		result[i] = NULL;
	}
	// Copies points into results array.
	size_t points_size = number_of_points(root);
	for (size_t i = 0; i < points_size; ++i) {
		if (box_contains_point(range, root->points[i])) {
			result[index++] = root->points[i];
		}
	}

	if (root->NW == NULL) {
		return result;
	}

	size_t i = 0;

	point **nw_search = quadTree_search(root->NW, range);
	while (nw_search[i] != NULL && i < 2048) {
		result[index++] = nw_search[i];
	}

	point **ne_search = quadTree_search(root->NE, range);
	while (ne_search[i] != NULL && i < 2048) {
		result[index++] = ne_search[i];
	}

	point **sw_search = quadTree_search(root->SW, range);
	while (sw_search[i] != NULL && i < 2048) {
		result[index++] = sw_search[i];
	}

	point **se_search = quadTree_search(root->SE, range);
	while (se_search[i] != NULL && i < 2048) {
		result[index++] = se_search[i];
	}

	return result;
}

// Prints results after performing a search.
void print_search_results(point ** result)
{
	size_t i = 0;

	while (result[i] != NULL && i < 2048) {
		point_print(result[i]);
		++i;
	}
	puts("");
}

// Prints points within a quadTree.
void print_quadTree_points(qt * qt, graph * g)
{
	for (size_t i = 0; i < qt->sz; ++i) {
		if (qt->points[i] == NULL) {
			return;
		}
		point *search_point =
			create_point(qt->points[i]->lat, qt->points[i]->lon);
		box *search = create_box(search_point, 30);
		point **result = quadTree_search(qt, search);

		point_add_edges(g, qt, result);

		box_destroy(search);
		// used free instead of result_destroy()
		free(result);
	}
}

// Adds edges to graph.
void point_add_edges(graph * g, qt * qt, point ** result)
{
	for (size_t i = 0; i < qt->sz; ++i) {
		if (result[i] == NULL) {
			return;
		}
		// Each run of the loop will find the distance between lat and lon
		// of the two zerg. The altitude variable is used to determine 
		// the distance when zerg are in the same exact lat and lon.
		// Edges are added if the distance is greater than 0 and less
		// than 15m.
		for (size_t j = i + 1; j < qt->sz; ++j) {
			if (result[j] == NULL) {
				continue;
			}
			double connection =
				haversine(result[i]->lat, result[j]->lat,
					result[i]->lon, result[j]->lon);
			uint32_t altitude =
				return_alt(result[i]->alt, result[j]->alt);
			if (connection * 1000 > 0 && connection * 1000 <= 15.00) {
				if (graph_add_edge(g, result[i]->zerg_id,
						result[j]->zerg_id,
						haversine(result[i]->lat,
								result[j]->lat,
								result[i]->lon,
								result[j]->lon))) {
				}
			} else if (connection < 0.001) {
				if (altitude <= 15.00) {
					graph_add_edge(g, result[i]->zerg_id,
							result[j]->zerg_id,
							altitude);
				}
			}
			if (connection * 1000 > 0 && connection * 1000 <= 15.00) {
				if (graph_add_edge(g, result[j]->zerg_id,
						   result[i]->zerg_id,
						   haversine(result[j]->lat,
								result[i]->lat,
								result[j]->lon,
								result[i]->lon))) {
				}
			} else if (connection < 0.0001) {
				if (altitude <= 15.00) {
					graph_add_edge(g, result[j]->zerg_id,
							result[i]->zerg_id,
							altitude);
				}
			}
		}
	}
	// used free instead of result_destroy()
	free(result);
}

// Returns difference for two given altitude values.
uint32_t return_alt(uint32_t alt, uint32_t alt2)
{
	if (alt > alt2) {
		return alt - alt2;
	} else {
		return alt2 - alt;
	}
}
