#include "zergmap_func.h"
#include "graph.h"
#include "quadTree.h"

#include <arpa/inet.h>
#include <byteswap.h>
#include <errno.h>
#include <getopt.h>
#include <endian.h>
#include <limits.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Establishes ENUM for return codes.
// SUCCESS: Condition handled successfully, continue program until exit.
// FAIL: Something went wrong (e.g. unable to allocate memory)
// CONTINUE: Continue looping.
enum RETURN_VALUE { SUCCESS, FAIL, CONTINUE };

// Establishes ENUM for Ethernet types 802.X, IPv4, and IPv6.
enum ETHERNET_TYPE { ETH_802Q = 0x8100, IPV4 = 0x800, IPV6 = 0x86DD };

// Establishes ENUM for IP protocol types.
enum IP_PROTOCOL { UDP = 17, IP_6IN4 = 41 };

// Designates enum for payload types.
enum PAYLOAD_TYPE { MESSAGE, STATUS, COMMAND, GPS };

// Populates appropriate PCAP file header struct fields with values read in
// from .PCAP file.
int file_hdr_parse(struct stats *stats, struct file *file,
		   struct pcap_file_header *pcap_fh)
{
	// Begins reading .PCAP file header into PCAP File Header Struct.
	size_t pcap_fh_read = 0;
	pcap_fh_read =
		fread(pcap_fh, sizeof(char), PCAP_FHEADER_SIZE, file->pcap_data);

	if (pcap_fh->file_type != PCAP_FILE_TYPE_BE
		&& pcap_fh->file_type != PCAP_FILE_TYPE_LE) {
		fprintf(stderr,
			"Please provide a valid .PCAP file to decode.\n");
		return FAIL;

		// Fread() returns 0 if elements cannot be read from file. This check is
		// performed and handled for every fread throughout the program.
	} else if (pcap_fh_read == 0) {
		fprintf(stderr,
			"Unable to process all PCAP file header elements.\n");
		return FAIL;

	}
	// If a Big Endian file type header is detected, the appropriate bytes
	// are converted from Big to Little Endian.
	if (pcap_fh->file_type == PCAP_FILE_TYPE_BE) {
		pcap_fh->major_version = ntohs(pcap_fh->major_version);
		pcap_fh->minor_version = ntohs(pcap_fh->minor_version);
		pcap_fh->link_layer_type = ntohl(pcap_fh->link_layer_type);
	}
	// Validates that the PCAP link layer type is Ethernet.
	enum LINK_LAYER_TYPE { ETHERNET = 1 };
	if (pcap_fh->link_layer_type != ETHERNET) {
		printf
			("PCAP Link Layer is not Ethernet. Unable to process packet.");
		++stats->bad_packet_count;
		return FAIL;
	}
	// Increments pcap_files_processed for every file processed. This is used
	// later to optionally print out file parsing stats.
	++stats->pcap_files_processed;

	return SUCCESS;
}

// Iterates through every packet and calls follow on header parsing functions
// to populate corresponding header struct fields.
int packet_hdr_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph,
			struct zerg_header *zhdr, struct pcap_file_header *pcap_fh,
			graph * g, qt * qt)
{
	// Checks to ensure *file is not null.
	if (!file) {
		return FAIL;
	}
	// Initializes a return to capture the return values of functions that are
	// called within packet_hdr_parse.
	int return_value = 0;

	// Begin while loop for processing every packet.
	while (true) {

		// After the first loop is performed, the bytes_index (later defined) is set
		// to the beginning of each packet during every new iteration of the while
		// loop until no packets remain in the .PCAP file.
		++stats->packet_count;
		if (stats->packet_count > 1) {
			fseek(file->pcap_data, stats->bytes_index, SEEK_SET);
		}
		// Ref for calculating bytes read within while loop: https://
		// stackoverflow.com/questions/40674693/performance-comparison-
		// pcap-file-reading-cs-ifstream-vs-cs-fread

		// Bytes read is used later in the program to calculate payload size.
		// Resets file bytes_read to zero on every iteration of while loop.
		stats->bytes_read = 0;

		// Begins reading packet header into packet header struct.
		size_t pcap_ph_read = 0;
		pcap_ph_read =
			fread(pcap_ph, sizeof(char), PCAP_PHEADER_SIZE,
			file->pcap_data);

		if (pcap_ph_read == 0 && file->count == file->total_files - 1) {
			// If all files have been processed and no more bytes can be read
			// return to zergmap-driver.                    
			return SUCCESS;

			// If pcap_ph_read == 0, the end of the packet has been reached
			// and the next one is processed.
		} else if (pcap_ph_read == 0) {
			break;
		}
		// If file type is in Big Endian format, capture length is
		// converted from Big to Little Endian.
		if (pcap_fh->file_type == PCAP_FILE_TYPE_BE) {
			pcap_ph->capture_len = ntohl(pcap_ph->capture_len);
			pcap_ph->packet_length = pcap_ph->capture_len;
		}
		// Sets packet_len to capture_length to use this value throughout the
		// remainder of the program, as the stats struct is passed to multiple
		// programs.
		stats->packet_len = pcap_ph->capture_len;

		// If this is the first iteration of the while loop, the bytes_index includes
		// the PCAP File Header size. Else, it does not as the PCAP File Header is
		// only read once at the beginning of the program.
		if (stats->packet_count == 1) {
			stats->bytes_index =
				(stats->packet_len + PCAP_FHEADER_SIZE +
				PCAP_PHEADER_SIZE);
		} else if (stats->packet_count > 1) {
			stats->bytes_index +=
				stats->packet_len + PCAP_PHEADER_SIZE;
		}
		// After parsing the packet header, the ethernet header is parsed.
		if (eth_header_parse(stats, file, pcap_ph)) {
			return FAIL;
		}
		// If ethernet type is 802.q, fseek rewinds the file back 14 bytes
		// (size of ethernet header). At which point the next section of
		// the pcap stream is read into the 802.q header.
		if (pcap_ph->eth->ethernet_type == ETH_802Q) {
			fseek(file->pcap_data, -ETH_HEADER_SIZE, SEEK_CUR);
			stats->bytes_read -= ETH_HEADER_SIZE;

			if (eth_802q_header_parse(stats, file, pcap_ph)) {
				return FAIL;
			}

		}
		// If either the ethernet or 802.q headers contain an IPv4 packet,
		// the appropriate number of bytes are then read into the IPv4
		// struct.
		if (pcap_ph->eth->ethernet_type == IPV4
			|| (pcap_ph->eth_802q
			&& pcap_ph->eth_802q->ethernet_type == IPV4)) {

			if (ipv4_header_parse(stats, file, pcap_ph)) {
				return FAIL;
			}

			if (pcap_ph->eth->ethernet_type == IPV4
				&& pcap_ph->ipv4->protocol != UDP
				&& pcap_ph->ipv4->protocol != IP_6IN4) {
				printf
					("Unable to parse PCAP. Decode only supports UDP parsing.\n");
				++stats->bad_packet_count;
				destroy_headers(pcap_ph);
				continue;
			}
			// If a 6in4 packet is detected, the IPv6 struct is populated.
			if (pcap_ph->ipv4->protocol == IP_6IN4) {
				return_value =
					ipv6_header_parse(stats, file, pcap_ph);
				if (return_value == CONTINUE) {
					continue;
				} else if (return_value == FAIL) {
					return FAIL;
				}

			}
			// If either the ethernet or 802.q header contain an IPv6 packet,
			// the appropriate number of bytes are then read into the IPv6
			// struct.
		} else if (pcap_ph->eth->ethernet_type == IPV6
			   || (pcap_ph->eth_802q
				&& pcap_ph->eth_802q->ethernet_type == IPV6)) {

			return_value = ipv6_header_parse(stats, file, pcap_ph);
			if (return_value == CONTINUE) {
				continue;
			} else if (return_value == FAIL) {
				return FAIL;
			}

		}
		// Following IPv6 / IPv4 header structs being populated, UDP parse is
		// called.
		return_value = udp_header_parse(stats, file, pcap_ph);
		if (return_value == CONTINUE) {
			continue;
		} else if (return_value == FAIL) {
			return FAIL;
		}
		// Following the UDP header structs being populated
		// zerg_header_parse is called.
		return_value =
			zerg_header_parse(stats, file, zhdr, pcap_ph, g, qt);
		if (return_value == CONTINUE) {
			continue;
		} else if (return_value == FAIL) {
			return FAIL;
		}

		destroy_headers(pcap_ph);
		stats->total_packet_count += stats->packet_count;
	}
	return SUCCESS;
}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet 802q struct fields.
int eth_802q_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph)
{

	// Dynamically allocates space for eth_802q struct pointer.
	pcap_ph->eth_802q = malloc(sizeof(*pcap_ph->eth_802q));
	if (!pcap_ph->eth_802q) {
		perror("Unable to create space for ethernet header payload.\n");
		return FAIL;
	}
	// Begins reading ethernet header into 802.q header struct.
	stats->bytes_read +=
		fread(pcap_ph->eth_802q, sizeof(char), ETH_802Q_HEADER_SIZE,
		file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all 802.Q header elements.\n");
		return FAIL;
	}
	// Converts ethernet type value from Big to Little Endian.
	pcap_ph->eth_802q->ethernet_type =
		ntohs(pcap_ph->eth_802q->ethernet_type);

	return SUCCESS;

}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet struct fields.
int eth_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph)
{

	// Dynamically allocates space for eth struct pointer.
	pcap_ph->eth = malloc(sizeof(*pcap_ph->eth));
	if (!pcap_ph->eth) {
		perror("Unable to create space for ethernet header payload.\n");
		return FAIL;
	}
	// Begins reading eth header into eth header struct.
	stats->bytes_read +=
		fread(pcap_ph->eth, sizeof(char), ETH_HEADER_SIZE, file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all elements.\n");
		destroy_headers(pcap_ph);
		return FAIL;
	}
	// Converts ethernet type value from Big to Little Endian.
	pcap_ph->eth->ethernet_type = ntohs(pcap_ph->eth->ethernet_type);

	return SUCCESS;
}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// IPv4 struct fields.
int ipv4_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph)
{

	// Dynamically allocates space for ipv4 struct pointer.
	pcap_ph->ipv4 = malloc(sizeof(*pcap_ph->ipv4));
	if (!pcap_ph->ipv4) {
		perror("Unable to create space for ethernet header payload.\n");
		return FAIL;
	}
	// Begins reading IPv4 header into IPv4 header struct.
	stats->bytes_read +=
		fread(pcap_ph->ipv4, sizeof(char), IPV4_HEADER_SIZE,
		  file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all IPv4 header elements.\n");
		return FAIL;
	}
	// Converts IPv4 total_length value from Big to Little Endian.
	pcap_ph->ipv4->total_len = ntohs(pcap_ph->ipv4->total_len);

	return SUCCESS;

}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet IPv6 struct fields.
int ipv6_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph)
{

	// Dynamically allocates space for ipv6 struct pointer.
	pcap_ph->ipv6 = malloc(sizeof(*pcap_ph->ipv6));
	if (!pcap_ph->ipv6) {
		perror("Unable to create space for ethernet header payload.\n");
		return FAIL;
	}
	// Begins reading IPv6 header into IPv6 header struct.
	stats->bytes_read +=
		fread(pcap_ph->ipv6, sizeof(char), IPV6_HEADER_SIZE,
		file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all IPv6 header elements.\n");
		return FAIL;
	}
	// If the follow on protocol is not UDP, return to packet_header_parse.
	if (pcap_ph->ipv6->next_header != UDP) {
		printf
			("Unable to parse PCAP. Decode only supports UDP parsing.\n");
		++stats->bad_packet_count;
		destroy_headers(pcap_ph);
		return CONTINUE;
	}

	return SUCCESS;
}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// UDP struct fields.
int udp_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph)
{
	// Dynamically allocates space for ipv6 struct pointer.
	pcap_ph->udp = malloc(sizeof(*pcap_ph->udp));
	if (!pcap_ph->udp) {
		perror("Unable to create space for ethernet header payload.\n");
		return FAIL;
	}
	// Begins reading UDP header into UDP header struct.
	stats->bytes_read +=
		fread(pcap_ph->udp, sizeof(char), UDP_HEADER_SIZE, file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all UDP elements.\n");
		return FAIL;
	}
	// Reverseses endianess of udp dst prt / len.
	pcap_ph->udp->dst_prt = ntohs(pcap_ph->udp->dst_prt);
	pcap_ph->udp->udp_len = ntohs(pcap_ph->udp->udp_len);

	// Validates that the UDP destination port is a valid Zerg port.
	enum DEST_PORT { ZERG_DEST_PORT = 3751 };
	if (pcap_ph->udp->dst_prt != ZERG_DEST_PORT) {
		fprintf(stderr,
			"Unable to parse PCAP. UDP destination port must be %d.\n",
			ZERG_DEST_PORT);
		++stats->bad_packet_count;
		destroy_headers(pcap_ph);
		return CONTINUE;
	}

	return SUCCESS;

}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg header struct fields.
int zerg_header_parse(struct stats *stats, struct file *file,
			struct zerg_header *zhdr,
			struct pcap_packet_header *pcap_ph, graph * g, qt * qt)
{

	// Begins reading Zerg header into Zerg header struct.
	stats->bytes_read +=
		fread(zhdr, sizeof(char), ZERG_HEADER_SIZE, file->pcap_data);
	if (stats->bytes_read == 0) {
		fprintf(stderr, "Unable to read all Zerg Header elements.\n");
		return FAIL;
	}
	// Reverses endianess of zhdr -> len, which includes version and type
	// bits as they are encompassed under the same uint32_t space. Then bit
	// shift to the right 8 to get rid of unneeded bits.
	zhdr->len = ntohl(zhdr->len) >> 8;

	// Reverses endianess of srcid, dstid, and sequence id. 
	zhdr->srcid = ntohs(zhdr->srcid);
	zhdr->dstid = ntohs(zhdr->dstid);
	zhdr->sequence_id = ntohl(zhdr->sequence_id);

	// Calculates payload size by subtracting bytes read from packet_len. 
	zhdr->payload_sz = (stats->packet_len - stats->bytes_read);

	// Validation check against Zerg Header length.
	if (pcap_ph->udp->udp_len - UDP_HEADER_SIZE != zhdr->len) {
		printf
			("UDP data length inconsistent with remainder of Zerg Header.\n");
		++stats->bad_packet_count;
		destroy_headers(pcap_ph);
		return CONTINUE;

	}
	// Initializes return_value to capture return values from follow Zerg
	// payload parsing functions.
	int return_value = 0;

	// Begin switch statement against payload type to then call follow on
	// payload parsing function.
	switch (zhdr->type) {
	case MESSAGE:
		if (!graph_contains(g, zhdr->srcid)) {
			graph_add_vertex(g, zhdr);
		}
		point *zerg = create_zerg_point(zhdr);
		qt_insert(qt, zerg);

		// If a message payload is encountered fseek forward the number of
		// bytes that is the size of the payload.
		fseek(file->pcap_data, zhdr->payload_sz, SEEK_CUR);
		++stats->packet_count;
		break;
	case STATUS:
		return_value = zerg_status_parse(stats, file, zhdr, g);
		break;
	case COMMAND:
		if (!graph_contains(g, zhdr->srcid)) {
			graph_add_vertex(g, zhdr);
		}
		zerg = create_zerg_point(zhdr);
		qt_insert(qt, zerg);

		// If a command payload is encountered fseek forward the number of
		// bytes that is the size of the payload.
		fseek(file->pcap_data, zhdr->payload_sz, SEEK_CUR);
		++stats->packet_count;
		break;
	case GPS:
		return_value = zerg_gps_parse(file, zhdr, g, qt);
		break;
	default:
		usage();
		fprintf(stderr, "Improper payload type.");
		++stats->bad_packet_count;

	}
	if (return_value == FAIL) {
		return FAIL;
	}

	return SUCCESS;
}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg Status struct fields.
int zerg_status_parse(struct stats *stats, struct file *file,
			struct zerg_header *zhdr, graph * g)
{

	// Dynamically allocates space for bit packing struct fields.
	zhdr->status_payload = malloc(sizeof(*zhdr->status_payload));
	if (!zhdr->status_payload) {
		perror("Unable to create space for status struct.\n");
		return FAIL;

	}
	// Begins reading Zerg Status payload into Zerg Status struct.
	size_t zstatus_elements_read =
		fread(zhdr->status_payload, ZERG_STATUS_SIZE, 1, file->pcap_data);
	if (zstatus_elements_read == 0) {
		fprintf(stderr, "Unable to read all Zerg status elements.");
		destroy_zerg(zhdr);
		return SUCCESS;
	}
	// Reverses endianess of hit points and max hit points then shift to the
	// right.
	zhdr->status_payload->hit_points =
		ntohl(zhdr->status_payload->hit_points) >> 8;
	zhdr->status_payload->max_hit_points =
		ntohl(zhdr->status_payload->max_hit_points) >> 8;

	// Reverses endianess of speed, then uses void pointer to cast the reversed
	// hex value into a IEEE 754 single precision decimal (32 bits).
	// Ref: https://www.circuitstoday.com/void-pointers-in-c
	zhdr->status_payload->speed = ntohl(zhdr->status_payload->speed);

	// Dynamically allocates space for status name.
	size_t status_name_sz = zhdr->payload_sz - ZERG_STATUS_SIZE;
	zhdr->status_payload->name =
		malloc((status_name_sz) * sizeof(zhdr->status_payload->name) + 1);
	if (!zhdr->status_payload) {
		perror("Unable to create space for status payload name.\n");
		destroy_zerg(zhdr);
		return FAIL;
	}
	// Begins reading message into message variable.
	size_t zstatus_name_elements_read =
		fread(zhdr->status_payload->name, status_name_sz, 1,
		file->pcap_data);
	if (zstatus_name_elements_read == 0) {
		fprintf(stderr,
			"No ASCII string / elements present to populate name field.\n");
		++stats->bad_packet_count;
		destroy_zerg(zhdr);
		return SUCCESS;
	}
	// Null terminating string assistance provided by mentor Sgt Gourdine.
	zhdr->status_payload->name[status_name_sz] = '\0';

	// Establishes enum for the default health threshold of 10 percent.
	enum HEALTH { DEFAULT_HEALTH_THRESHOLD = 10 };

	// Calculates percent of health that the Zerg has.
	zhdr->status_payload->percent_health =
		((double)zhdr->status_payload->hit_points /
		 (double)zhdr->status_payload->max_hit_points) * 100;

	// If the -h flag is passed and the Zerg health is below that threshold,
	// the low health flag is set to true and used by graph.c for evaluation
	// later in the program.
	if (zhdr->health_flag) {
		if (zhdr->status_payload->percent_health <
			zhdr->health_threshold) {
			zhdr->low_health = true;
		}
		// If the -h flag is not passed and the Zerg health is below the,
		// default health threshold, the low health flag is set to true 
		// and used by graph.c for evaluation later in the program.
	} else if (zhdr->status_payload->percent_health <
		DEFAULT_HEALTH_THRESHOLD) {
		zhdr->low_health = true;
	}

	if (!graph_contains(g, zhdr->srcid)) {
		graph_add_vertex(g, zhdr);
	}

	destroy_zerg(zhdr);

	return SUCCESS;
}

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg GPS struct fields.
int zerg_gps_parse(struct file *file, struct zerg_header *zhdr, graph * g,
		qt * qt)
{

	size_t gps_sz = sizeof(struct zerg_gps);

	// Allocates space for gps payload struct pointer.
	zhdr->gps_payload = malloc(sizeof(*zhdr->gps_payload));
	if (!zhdr->gps_payload) {
		perror("Unable to create space for gps struct.\n");
		return FAIL;
	}
	// Begins reading GPS payload into GPS struct.
	size_t gps_elements_read =
		fread(zhdr->gps_payload, gps_sz, 1, file->pcap_data);
	if (gps_elements_read == 0) {
		fprintf(stderr, "Unable to read all GPS elements.\n");
		destroy_zerg(zhdr);
		return SUCCESS;
	}

	// bswap_64 swaps 8 64 bit integers / 8 byte hex values.
	// Ref: http://manpages.ubuntu.com/manpages/cosmic/man3/endian.3.html
	zhdr->gps_payload->latitude = be64toh(zhdr->gps_payload->latitude);

	zhdr->gps_payload->longitude = be64toh(zhdr->gps_payload->longitude);

	zhdr->gps_payload->altitude = ntohl(zhdr->gps_payload->altitude);
	void *ptr_gps = &(zhdr->gps_payload->altitude);

	// Converts altitude from fathoms to meters.
	float alt_convert_m = *(float *)ptr_gps * 1.8288;
	zhdr->gps_payload->altitude_float = alt_convert_m;

	zhdr->gps_payload->bearing = ntohl(zhdr->gps_payload->bearing);
	ptr_gps = &(zhdr->gps_payload->bearing);

	zhdr->gps_payload->speed = ntohl(zhdr->gps_payload->speed);
	ptr_gps = &(zhdr->gps_payload->speed);

	zhdr->gps_payload->accuracy = ntohl(zhdr->gps_payload->accuracy);
	ptr_gps = &(zhdr->gps_payload->accuracy);

	// Creates Zerg point and inserts Zerg data into tree.
	point *zerg = create_zerg_point(zhdr);
	if (qt_insert(qt, zerg)) {
	}
	// If graph not contain Zerg ID, add vertex for Zerg.
	if (!graph_contains(g, zhdr->srcid)) {
		graph_add_vertex(g, zhdr);
	} else {
		// If the ID is in there try to update the coordinates.
		if (!graph_update_coord(g, zhdr->srcid, zhdr)) {
			printf("Two conflicting positions for the same Zerg");
			exit(EXIT_SUCCESS);
		}
	}

	destroy_zerg(zhdr);
	return SUCCESS;

}

// Prints packet / PCAP file parsing stats.
void print_stats(struct stats *stats)
{
	printf("Zerg Psychic CAPture Statistics:\n\n");
	printf("%d PCAP file(s) processed.\n", stats->pcap_files_processed);
	printf("%d packets processed.\n", stats->total_packet_count);
	printf("%d packet(s) decoded.\n",
		stats->total_packet_count - stats->bad_packet_count);
	fprintf(stderr, "%d packet(s) unable to be decoded.\n",
		stats->bad_packet_count);
}

// Frees memory for all dynamically allocated space associated to packet
// headers.
void destroy_headers(struct pcap_packet_header *pcap_ph)
{
	if (pcap_ph->eth) {
		free(pcap_ph->eth);
		pcap_ph->eth = NULL;
	}
	if (pcap_ph->eth_802q) {
		free(pcap_ph->eth_802q);
		pcap_ph->eth_802q = NULL;
	}
	if (pcap_ph->ipv4) {
		free(pcap_ph->ipv4);
		pcap_ph->ipv4 = NULL;
	}
	if (pcap_ph->ipv6) {
		free(pcap_ph->ipv6);
		pcap_ph->ipv6 = NULL;
	}
	if (pcap_ph->udp) {
		free(pcap_ph->udp);
		pcap_ph->udp = NULL;
	}
}

// Frees memory for all dynamically allocated space associated to Zerg
// payloads.
void destroy_zerg(struct zerg_header *zhdr)
{
	// Memory freeing assistance provided by mentor Maj Ireland.

	switch (zhdr->type) {
	case STATUS:
		if (zhdr->status_payload) {
			if (zhdr->status_payload->name) {
				free(zhdr->status_payload->name);
				zhdr->status_payload->name = NULL;
			}
			free(zhdr->status_payload);
			zhdr->status_payload = NULL;
		}
		break;
	case GPS:
		if (zhdr->gps_payload) {
			free(zhdr->gps_payload);
			zhdr->gps_payload = NULL;
		}
		break;
	}
}

// Prints a usage statement for the user.
void usage()
{
	fprintf(stderr, "Usage: ./zergmap [-h]<var> <filename>\n");
}
