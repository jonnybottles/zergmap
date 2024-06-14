#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "map.h"

START_TEST(test_create_destroy)
{
	map *m = map_create();
	ck_assert_ptr_nonnull(m);
	map_destroy(m);
}

END_TEST START_TEST(test_put_get)
{
	map *m = map_create();
	ck_assert_ptr_null(map_get(m, 10));

	char *zerg1 = malloc(sizeof(*zerg1));
	char *zerg2 = malloc(sizeof(*zerg2));
	char *zerg3 = malloc(sizeof(*zerg3));
	char *zerg4 = malloc(sizeof(*zerg4));
	char *zerg5 = malloc(sizeof(*zerg5));
	char *zerg6 = malloc(sizeof(*zerg6));
	char *zerg7 = malloc(sizeof(*zerg7));
	// Cast to remove const-ness
	map_put(m, 10, (char *)zerg1);
	map_put(m, 11, (char *)zerg2);
	map_put(m, 12, (char *)zerg3);
	map_put(m, 13, (char *)zerg4);
	map_put(m, 14, (char *)zerg5);
	map_put(m, 15, (char *)zerg6);
	map_put(m, 16, (char *)zerg7);

	ck_assert_ptr_nonnull(map_get(m, 10));
	ck_assert_ptr_nonnull(map_get(m, 13));
	ck_assert_ptr_null(map_get(m, 17));

	map_destroy(m);
}

END_TEST START_TEST(test_contains)
{
	map *m = map_create();

	char *zerg1 = malloc(sizeof(*zerg1));
	char *zerg2 = malloc(sizeof(*zerg2));
	char *zerg3 = malloc(sizeof(*zerg3));
	char *zerg4 = malloc(sizeof(*zerg4));
	char *zerg5 = malloc(sizeof(*zerg5));
	char *zerg6 = malloc(sizeof(*zerg6));
	char *zerg7 = malloc(sizeof(*zerg7));
	char *zerg8 = malloc(sizeof(*zerg8));

	// Cast to remove const-ness
	map_put(m, 10, (char *)zerg1);
	map_put(m, 11, (char *)zerg2);
	map_put(m, 12, (char *)zerg3);
	map_put(m, 13, (char *)zerg4);
	map_put(m, 14, (char *)zerg5);
	map_put(m, 15, (char *)zerg6);
	map_put(m, 16, (char *)zerg7);
	map_put(m, 17, (char *)zerg8);

	ck_assert(map_contains(m, 10));
	ck_assert(map_contains(m, 11));
	ck_assert(map_contains(m, 12));
	ck_assert(map_contains(m, 13));
	ck_assert(map_contains(m, 14));
	ck_assert(map_contains(m, 15));
	ck_assert(map_contains(m, 16));
	ck_assert(map_contains(m, 17));

	ck_assert(!map_contains(m, 18));
	ck_assert(!map_contains(m, 19));
	ck_assert(!map_contains(m, 20));
	ck_assert(!map_contains(m, 21));
	ck_assert(!map_contains(m, 22));

	map_destroy(m);
}

END_TEST Suite * test_map_suite(void)
{
	// This list needs to be inside a function
	// Type needs to be const TTest * instead of TFun
	const TTest *core_tests[] = {
		test_create_destroy,
		test_put_get,
		test_contains,
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
	Suite *s = suite_create("Map");
	suite_add_tcase(s, tc);

	return s;
}
