#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "priority-queue.h"

START_TEST(test_create_destroy)
{
	pqueue *pq = pqueue_create(MIN);
	ck_assert_ptr_nonnull(pq);
	pqueue_destroy(pq);
}

END_TEST START_TEST(test_enqueue_min)
{
	pqueue *pq = pqueue_create(MIN);
	// Cast away const-ness
	ck_assert(pqueue_enqueue(pq, 3.14, 10));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 3.14, 0.0001);
	ck_assert(pqueue_enqueue(pq, 2.71, 11));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 2.71, 0.0001);
	ck_assert(pqueue_enqueue(pq, 6.02, 12));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 2.71, 0.0001);

	ck_assert_double_eq(pqueue_dequeue(pq), 11);
	ck_assert_double_eq(pqueue_dequeue(pq), 10);
	ck_assert_double_eq(pqueue_dequeue(pq), 12);
	ck_assert_double_eq(pqueue_dequeue(pq), 0xffff);

	pqueue_destroy(pq);
}

END_TEST START_TEST(test_enqueue_max)
{
	pqueue *pq = pqueue_create(MAX);
	// Cast away const-ness
	ck_assert(pqueue_enqueue(pq, 3.14, 10));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 3.14, 0.0001);
	ck_assert(pqueue_enqueue(pq, 2.71, 11));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 3.14, 0.0001);
	ck_assert(pqueue_enqueue(pq, 6.02, 12));
	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 6.02, 0.0001);

	ck_assert_double_eq(pqueue_dequeue(pq), 12);
	ck_assert_double_eq(pqueue_dequeue(pq), 10);
	ck_assert_double_eq(pqueue_dequeue(pq), 11);
	ck_assert_double_eq(pqueue_dequeue(pq), 0xffff);

	pqueue_destroy(pq);
}

END_TEST START_TEST(test_capacity)
{
	pqueue *pq = pqueue_create(MIN);
	for (size_t n = 0; n < 1000; ++n) {
		// Cast away const-ness
		ck_assert(pqueue_enqueue(pq, 3.14, 10));
	}

	ck_assert_double_eq_tol(pqueue_get_next_priority(pq), 3.14, 0.0001);

	pqueue_destroy(pq);
}

END_TEST Suite * test_pqueue_suite(void)
{
	// This list needs to be inside a function
	// Type needs to be const TTest * instead of TFun
	const TTest *core_tests[] = {
		test_create_destroy,
		test_enqueue_min,
		test_enqueue_max,
		test_capacity,
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
	Suite *s = suite_create("Priority Queue");
	suite_add_tcase(s, tc);

	return s;
}
