#ifndef __NET_H__
#define __NET_H__

#include "vars.h"
#include "bswap.h"

#define VOOT_UDP_PORT       5007

/* This cannot be greater than 0x800 (2048d) - the Tx descriptor size. */
#define NET_MAX_PACKET      1518

#define ETHER_MAC_SIZE      6

#define IP_PROTO_ICMP       0x1
#define IP_PROTO_UDP        0x11

#define IP_HEADER_SIZE(hdr) ((hdr->version_ihl & 0x0f) * 4)

/* Straight from "Internetworking with TCP/IP" and just about any TCP/IP
    stack in existance. */
#define ICMP_TYPE_ECHO_REQUEST  8
#define ICMP_TYPE_ECHO_REPLY    0

/* Virtual definitions for our byte-swapping functions. */
#define ntohl bswap32
#define htonl bswap32
#define ntohs bswap16
#define htons bswap16

typedef struct
{
    uint8 dest[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint8 source[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint16 ethertype __attribute__ ((packed));
} ether_ii_header_t __attribute__ ((packed));

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint8   source[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint16  length __attribute__ ((packed));
    uint8   dsap __attribute__ ((packed));
    uint8   ssap __attribute__ ((packed));
    uint8   control __attribute__ ((packed));
    uint8   vendor_code[3] __attribute__ ((packed));
    uint16  ethertype __attribute__ ((packed));
} ether_8023_header_t __attribute__ ((packed));

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint8   source[ETHER_MAC_SIZE] __attribute__ ((packed));
    uint16  ethertype __attribute__ ((packed));

    uint32  length __attribute__ ((packed));
    uint8   *data __attribute__ ((packed));
    uint16  crc __attribute__ ((packed));
} ether_info_packet_t __attribute__ ((packed));

typedef struct
{
    uint8   version_ihl __attribute__ ((packed));
    uint8   tos __attribute__ ((packed));
    uint16  length __attribute__ ((packed));
    uint16  packet_id __attribute__ ((packed));
    uint16  flags_frag_offset __attribute__ ((packed));
    uint8   ttl __attribute__ ((packed));
    uint8   protocol __attribute__ ((packed));
    uint16  checksum __attribute__ ((packed));
    uint32  source __attribute__ ((packed));
    uint32  dest __attribute__ ((packed));
} ip_header_t __attribute__ ((packed));

typedef struct
{
    uint8   type __attribute__ ((packed));
    uint8   code __attribute__ ((packed));
    uint16  checksum __attribute__ ((packed));
    uint32  misc __attribute__ ((packed));
} icmp_header_t __attribute__ ((packed));

typedef struct
{
    uint16  src __attribute__ ((packed));
    uint16  dest __attribute__ ((packed));
    uint16  length __attribute__ ((packed));
    uint16  checksum __attribute__ ((packed));
} udp_header_t __attribute__ ((packed));

typedef struct
{
    uint32  source_ip __attribute__ ((packed));
    uint32  dest_ip __attribute__ ((packed));
    uint8   zero __attribute__ ((packed));
    uint8   protocol __attribute__ ((packed));
    uint16  length __attribute__ ((packed));
} udp_pseudo_header_t __attribute__ ((packed));

bool net_transmit(ether_info_packet_t *frame_in);

uint16 ip_checksum(ip_header_t *ip, uint16 ip_header_length);
bool ip_handle_packet(ether_info_packet_t *frame);

uint16 icmp_checksum(icmp_header_t *icmp, uint16 icmp_header_length);
bool icmp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 icmp_data_length);

uint16 udp_checksum(ip_header_t *ip, uint16 ip_header_length);
bool udp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 udp_data_length);

bool net_handle_frame(uint8 *frame_data, uint32 frame_size);

#endif
