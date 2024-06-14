
#ifndef ZERGMAP_FUNC_H
#define ZERGMAP_FUNC_H

#include "graph.h"
#include "zergmap.h"
#include "quadTree.h"
#include <stdint.h>

// Populates appropriate PCAP file header struct fields with values read in
// from .PCAP file.
int file_hdr_parse(struct stats *stats, struct file *file,
		struct pcap_file_header *pcap_fh);

// Iterates through every packet and calls follow on header parsing functions
// to populate corresponding header struct fields.
int packet_hdr_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph,
			struct zerg_header *zhdr,
			struct pcap_file_header *pcap_fh, graph * g, qt * qt);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet struct fields.
int eth_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet 802Q struct fields.
int eth_802q_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// IPv4 struct fields.
int ipv4_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// ethernet IPv6 struct fields.
int ipv6_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// UDP struct fields.
int udp_header_parse(struct stats *stats, struct file *file,
			struct pcap_packet_header *pcap_ph);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg header struct fields.
int zerg_header_parse(struct stats *stats, struct file *file,
			struct zerg_header *zhdr,
			struct pcap_packet_header *pcap_ph, graph * g, qt * qt);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg GPS struct fields.
int zerg_gps_parse(struct file *file, struct zerg_header *zhdr, graph * g,
		qt * qt);

// Parses .PCAP file, converts values from Big to Little Endian and populates
// Zerg status struct fields.
int zerg_status_parse(struct stats *stats, struct file *file,
			struct zerg_header *zhdr, graph * g);

// Frees memory for all dynamically allocated space associated to Zerg
// payloads.
void destroy_zerg(struct zerg_header *zhdr);

// Frees memory for all dynamically allocated space associated to packet
// headers.
void destroy_headers(struct pcap_packet_header *pcap_ph);

// Prints packet / PCAP file parsing stats.
void print_stats(struct stats *stats);

// Prints a usage statement for the user.
void usage();

#endif
