
#ifndef ZERGMAP_H
#define ZERGMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// These defines are the Little and Big Endian file type IDs for PCAP files.
// A define is used instead of enum, as ISO C restricts enumerator values to
// the range of an int. PCAP File Types Ref: //https://wiki.wireshark.org/
// Development/LibpcapFileFormat.
#define PCAP_FILE_TYPE_LE 2712847316
#define PCAP_FILE_TYPE_BE 3569595041

// PCAP file header is 24 bytes in length as per RFC.
enum { PCAP_FHEADER_SIZE = 24 };
struct pcap_file_header {

	uint32_t file_type;
	uint16_t major_version;
	uint16_t minor_version;
	uint32_t gmt_offset;
	uint32_t accuracy_delta;
	uint32_t max_cap_len;
	uint32_t link_layer_type;

};

// PCAP packet header is 16 bytes in length as per RFC.
enum { PCAP_PHEADER_SIZE = 16 };
struct pcap_packet_header {

	uint32_t unix_epoch;
	uint32_t from_epoch;
	uint32_t capture_len;
	uint32_t packet_length;
	struct ethernet_header *eth;
	struct eth_802q *eth_802q;
	struct ipv4_header *ipv4;
	struct ipv6_header *ipv6;
	struct udp_header *udp;

};

// Ethernet header is 14 bytes in length as per RFC.
enum { ETH_HEADER_SIZE = 14 };
struct ethernet_header {

	uint8_t dst_mac[6];
	uint8_t src_mac[6];
	uint16_t ethernet_type;

};

// Ethernet 802Q header is 18 bytes in length as per RFC.
enum { ETH_802Q_HEADER_SIZE = 18 };
struct eth_802q {

	uint8_t dst_mac[6];
	uint8_t src_mac[6];
	uint32_t vlan_tag;
	uint16_t ethernet_type;

};

// IPv4 header is 20 bytes in length as per RFC.
enum { IPV4_HEADER_SIZE = 20 };
struct ipv4_header {

	uint8_t ip_ihl:4;
	uint8_t ip_version:4;
	uint8_t dscp:6;
	uint8_t ecn:2;
	uint16_t total_len;
	uint16_t identification;
	uint16_t flags:3;
	uint16_t fragment_offset:13;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t header_checksum;
	uint32_t src_ip;
	uint32_t dst_ip;

};

// IPv6 header is 40 bytes in length as per RFC.
enum { IPV6_HEADER_SIZE = 40 };

struct ipv6_header {

	uint8_t traffic_class;
	uint8_t version:4;
	uint32_t flow_label:20;
	uint16_t payload_length:16;
	uint8_t next_header:8;
	uint8_t hop_limit:8;
	uint32_t src_addr[4];
	uint32_t dst_addr[4];

};

// UDP header is 8 bytes in length as per RFC.
enum { UDP_HEADER_SIZE = 8 };
struct udp_header {

	uint16_t src_prt;
	uint16_t dst_prt;
	uint16_t udp_len;
	uint16_t udp_checksum;

};

// Zerg header is 12 bytes in length as per DICE ruberic.
enum { ZERG_HEADER_SIZE = 12 };
struct zerg_header {

	uint8_t type:4;
	uint8_t version:4;
	uint32_t len:24;
	uint16_t srcid;
	uint16_t dstid;
	uint32_t sequence_id;
	struct zerg_status *status_payload;
	struct zerg_gps *gps_payload;
	size_t payload_sz;
	bool health_flag;
	bool low_health;
	double health_threshold;

};

// Zerg status is 12 bytes in length as per DICE ruberic.
enum { ZERG_STATUS_SIZE = 12 };
struct zerg_status {

	int32_t hit_points:24;
	uint8_t armor;
	uint32_t max_hit_points:24;
	uint8_t type;
	uint32_t speed;
	char *name;
	double percent_health;

};

// Establishes structure for Zerg GPS payloads.
struct zerg_gps {

	union {
		uint64_t longitude;
		double longitude_double;
	};
	union {
		uint64_t latitude;
		double latitude_double;
	};

	union {
		uint32_t altitude;
		float altitude_float;
	};

	uint32_t bearing;
	uint32_t speed;
	uint32_t accuracy;

};

// Establishes file struct.
struct file {
	size_t count;
	size_t total_files;
	FILE *pcap_data;
};

// Establishes stats struct.
struct stats {
	int packet_len;
	long bytes_index;
	long bytes_read;
	int packet_count;
	int bad_packet_count;
	int total_packet_count;
	int pcap_files_processed;
};

#endif
