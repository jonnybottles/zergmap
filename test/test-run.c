#include <check.h>

// Function signatures for test-run.
Suite *test_map_suite(void);
Suite *test_pqueue_suite(void);
Suite *test_graph_suite(void);
Suite *test_quadTree_suite(void);
Suite *test_zergmap_suite(void);

int main(void)
{
	// Creates suite objects to capture test results.
	Suite *map = test_map_suite();
	Suite *pqueue = test_pqueue_suite();
	Suite *graph = test_graph_suite();
	Suite *qt = test_quadTree_suite();
	Suite *zergmap_tests = test_zergmap_suite();

	// Creates Srunner object for running test suites.
	SRunner *sr = srunner_create(graph);
	srunner_add_suite(sr, map);
	srunner_add_suite(sr, pqueue);
	srunner_add_suite(sr, qt);
	srunner_add_suite(sr, zergmap_tests);

	srunner_run_all(sr, CK_NORMAL);

	int failed = srunner_ntests_failed(sr);

	srunner_free(sr);

	// Convert failed to be either 0 or 1
	return !!failed;
}
