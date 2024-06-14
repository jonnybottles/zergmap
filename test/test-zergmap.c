#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "zergmap_func.h"

// Create test for file_hrd_parse.
START_TEST(test_file_hdr_parse)
{
	struct file file = { 0, 0, NULL };
	struct stats stats = { 0, 0, 0, 0, 0, 0, 0 };
	struct pcap_file_header pcap_fh;

	// Tests a valid file .pcap file to ensure it is processed by
	// file_hdr_parse, which returns a 0 if processed.
	file.pcap_data = fopen("test/samp/G_normal.pcap", "rb");
	ck_assert_int_eq(file_hdr_parse(&stats, &file, &pcap_fh), 0);
	fclose(file.pcap_data);

	// Tests a big endian .pcap file to ensure it is not processed by
	// file_hdr_parse, which returns a 0 if processed.
	file.pcap_data = fopen("test/samp/M_bigendian.pcap", "rb");
	ck_assert_int_eq(file_hdr_parse(&stats, &file, &pcap_fh), 0);
	fclose(file.pcap_data);
}

END_TEST
// Create test for packet_hdr_parse.
START_TEST(test_packet_hdr_parse)
{
	struct pcap_file_header pcap_fh;
	struct pcap_packet_header pcap_ph;
	struct stats stats = { 0, 0, 0, 0, 0, 0, 0 };
	pcap_ph.eth = NULL;
	pcap_ph.eth_802q = NULL;
	pcap_ph.ipv4 = NULL;
	pcap_ph.ipv6 = NULL;
	pcap_ph.udp = NULL;
	pcap_ph.udp = NULL;
	struct zerg_header zhdr =
		{ 0, 0, 0, 0, 0, 0, NULL, NULL, 0, false, 0, false };
	struct file file = { 0, 0, NULL };
	graph *zerg_network = graph_create();
	qt *zerg_tree = create_quadTree(create_box(create_point(0, 0), 1000));

	// Tests a valid file .pcap file to ensure it is processed by
	// pkt_hdr_parse, which returns a 0 if processed.
	file.pcap_data = fopen("test/samp/G_normal.pcap", "rb");
	file_hdr_parse(&stats, &file, &pcap_fh);
	ck_assert_int_eq(packet_hdr_parse
			(&stats, &file, &pcap_ph, &zhdr, &pcap_fh,
			zerg_network, zerg_tree), 0);
	fclose(file.pcap_data);
	graph_destroy(zerg_network);
	qt_destroy(zerg_tree);
	destroy_zerg(&zhdr);

}

END_TEST Suite *test_zergmap_suite(void)
{

	const TTest *core_tests[] = {
		test_file_hdr_parse,
		test_packet_hdr_parse,
		NULL
	};

	// Test case collects unit tests.
	TCase *tc = tcase_create("Core");

	const TTest **current = core_tests;
	while (*current) {
		tcase_add_test(tc, *current);
		++current;
	}
	// Suite collects test cases.
	Suite *s = suite_create("zergmap_tests");
	suite_add_tcase(s, tc);

	return s;
}
