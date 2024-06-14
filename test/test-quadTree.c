#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "../quadTree.h"

START_TEST(test_create_point)
{
	point *p = create_point(100, 100);
	ck_assert_ptr_nonnull(p);
	point_destroy(p);
}

END_TEST START_TEST(test_create_box)
{
	box *b = create_box(create_point(0, 0), 100);
	ck_assert_ptr_nonnull(b);

	box_destroy(b);
}

END_TEST START_TEST(test_box_contains_point)
{
	point *p = create_point(100, 100);
	point *p2 = create_point(1000, 1000);
	ck_assert_ptr_nonnull(p);
	ck_assert_ptr_nonnull(p2);

	box *b = create_box(create_point(0, 0), 100);
	ck_assert_ptr_nonnull(b);

	ck_assert(box_contains_point(b, p));
	ck_assert(!box_contains_point(b, p2));

	point_destroy(p);
	point_destroy(p2);
	box_destroy(b);
}

END_TEST START_TEST(test_box_intersects)
{
	box *a = create_box(create_point(100, 100), 100);
	ck_assert_ptr_nonnull(a);
	box *b = create_box(create_point(200, 200), 150);
	ck_assert_ptr_nonnull(b);
	box *c = create_box(create_point(50, 50), 10);
	ck_assert_ptr_nonnull(c);

	ck_assert(box_intersects(a, b));
	ck_assert(box_intersects(a, c));
	ck_assert(box_intersects(b, c));

	box_destroy(a);
	box_destroy(b);
	box_destroy(c);
}

END_TEST START_TEST(test_create_qt)
{
	point *center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);

	box *boundry = create_box(center, 15);
	ck_assert_ptr_nonnull(boundry);

	qt *quadTree = create_quadTree(boundry);
	ck_assert_ptr_nonnull(quadTree);

	qt_destroy(quadTree);
}

END_TEST START_TEST(test_point_destroy)
{
	point *center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);
	point_destroy(center);
}

END_TEST START_TEST(test_box_destroy)
{
	point *center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);

	box *boundry = create_box(center, 30);
	ck_assert_ptr_nonnull(boundry);

	box_destroy(boundry);
}

END_TEST START_TEST(test_quadTree_destroy)
{
	point *center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);

	box *boundry = create_box(center, 30);
	ck_assert_ptr_nonnull(boundry);

	qt *quadTree = create_quadTree(boundry);
	ck_assert_ptr_nonnull(quadTree);

	qt_destroy(quadTree);
}

END_TEST START_TEST(test_point_insert)
{
	point *center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);

	box *boundry = create_box(center, 30);
	ck_assert_ptr_nonnull(boundry);

	qt *quadTree = create_quadTree(boundry);
	ck_assert_ptr_nonnull(quadTree);

	point *a = create_point(125, 125);
	ck_assert_ptr_nonnull(a);
	point *b = create_point(150, 150);
	ck_assert_ptr_nonnull(b);

	ck_assert(qt_insert(quadTree, a));

	qt_destroy(quadTree);
}

END_TEST START_TEST(test_quadTree_search)
{
	point *center = NULL;
	center = create_point(100, 100);
	ck_assert_ptr_nonnull(center);

	box *boundry = NULL;
	boundry = create_box(center, 30);
	ck_assert_ptr_nonnull(boundry);

	qt *quadTree = NULL;
	quadTree = create_quadTree(boundry);
	ck_assert_ptr_nonnull(quadTree);

	point *a = NULL;
	a = create_point(100, 100);
	ck_assert_ptr_nonnull(a);

	point *b = NULL;
	b = create_point(115, 115);
	ck_assert_ptr_nonnull(b);

	point *c = NULL;
	c = create_point(120, 120);
	ck_assert_ptr_nonnull(c);

	qt_insert(quadTree, a);
	qt_insert(quadTree, b);
	qt_insert(quadTree, c);

	qt_destroy(quadTree);

}

END_TEST Suite * test_quadTree_suite(void)
{
	// This list needs to be inside a function
	// Type needs to be const TTest * instead of TFun
	const TTest *core_tests[] = {
		test_create_point,
		test_create_box,
		test_box_contains_point,
		test_box_intersects,
		test_create_qt,
		test_point_destroy,
		test_box_destroy,
		test_quadTree_destroy,
		test_point_insert,
		test_quadTree_search,
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
	Suite *s = suite_create("quadTree");
	suite_add_tcase(s, tc);

	return s;
}
