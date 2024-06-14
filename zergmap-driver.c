#include "zergmap_func.h"
#include "graph.h"
#include "quadTree.h"

#include <string.h>
#include <getopt.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{

	struct pcap_file_header pcap_fh = { 0 };
	struct pcap_packet_header pcap_ph = { 0 };
	struct stats stats = { 0 };
	struct zerg_header zhdr = { 0 };
	struct file file = { 0 };

	int options;
	bool stats_flag = false;
	opterr = 0;

	// Parses command line arguments with getopt. -h will trigger the health
	// flag. -s will print packet / pcap stats at the end of the program.
	// The default case will call usage.
	while ((options = getopt(argc, argv, "sh:")) != -1) {
		switch (options) {
		case 'h':
			zhdr.health_flag = true;
			char *err;
			zhdr.health_threshold = strtol(optarg, &err, 10);
			if (*err) {
				fprintf(stderr, "%s is not a number. \n",
					optarg);
				usage();
			}
			break;
		case 's':
			stats_flag = true;
			break;
		default:
			usage();
			break;
		}
	}

	// Normalizes argc / argv to zero.
	argc -= optind;
	argv += optind;

	// Exits program if no arguments are passed.
	if (argc == 0) {
		usage();
		exit(EXIT_SUCCESS);
	}

	graph *zerg_network = graph_create();
	point *initial = create_point(0, 0);
	box *initial_box = create_box(initial, 1000);
	qt *zerg_tree = create_quadTree(initial_box);

	// Used to evaluate return value of strstr in follow on code.
	char *str_ret;

	if (argc >= 1) {

		// Sets argument count to file.total files.
		file.total_files = argc;

		// For loop iterates through every filename in the argv array to
		// make sure the file is a .pcap extension. For every file that is
		// a .pcap, it is opened with fopen and the follow on functions are
		// executed.
		for (file.count = 0; file.count < file.total_files;
			 file.count++) {

			// Sets packet count back to zero after each file have been completely read.
			stats.packet_count = 0;

			// Validates that the filename has a .pcap extension.
			str_ret = strstr(argv[file.count], ".pcap");
			if (!str_ret) {
				if (argc == 1) {
					printf
						("Please enter a valid .PCAP file.\n");
					usage();
					exit(EXIT_SUCCESS);
				}
			}
			// If there is a file with a valid .pcap extension it is opened.
			file.pcap_data = fopen(argv[file.count], "rb");
			if (!file.pcap_data) {
				fprintf(stderr, "unable to open %s: %s. \n",
					argv[file.count], strerror(errno));
				graph_destroy(zerg_network);
				qt_destroy(zerg_tree);
				exit(EXIT_SUCCESS);
			}
			// Call file_hdr_parse to begin parsing file header.
			if (file_hdr_parse(&stats, &file, &pcap_fh)) {
				fclose(file.pcap_data);
				graph_destroy(zerg_network);
				qt_destroy(zerg_tree);
				exit(EXIT_FAILURE);
			}
			// If the file has the appropriate file header, call
			// packet_header_parse.
			if (packet_hdr_parse
				(&stats, &file, &pcap_ph, &zhdr, &pcap_fh,
				 zerg_network, zerg_tree)) {
				graph_destroy(zerg_network);
				qt_destroy(zerg_tree);
				destroy_headers(&pcap_ph);
				fclose(file.pcap_data);
				exit(EXIT_FAILURE);
			}
			// Closes each file after being reading all bytes.
			fclose(file.pcap_data);
		}
	}

	print_quadTree_points(zerg_tree, zerg_network);

	pqueue *to_remove = check_connectivity(zerg_network);

	recommend_removal(zerg_network, to_remove, zerg_count(zerg_network));

	// If the stats flag is true, stats are printed.
	if (stats_flag) {
		print_stats(&stats);
	}
	// Displays all Zerg IDs with health below the default or user defined
	// threshold
	show_low_health_zerg(zerg_network);

	// Displays all Zerg IDs that have no associated GPS data.
	show_no_gps(zerg_network);

	graph_destroy(zerg_network);
	qt_destroy(zerg_tree);
	destroy_zerg(&zhdr);

	return (EXIT_SUCCESS);
}
