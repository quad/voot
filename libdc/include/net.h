#ifndef __NET_H__
#define __NET_H__

#include "vars.h"
#include "bswap.h"

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

#define ntohl bswap32
#define htonl bswap32
#define ntohs bswap16
#define htons bswap16

typedef struct
{
    uint8 dest[ETHER_MAC_SIZE];
    uint8 source[ETHER_MAC_SIZE];
    uint16 ethertype;
} ether_ii_header_t;

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE];
    uint8   source[ETHER_MAC_SIZE];
    uint16  length;
    uint8   dsap;
    uint8   ssap;
    uint8   control;
    uint8   vendor_code[3];
    uint16  ethertype;
} ether_8023_header_t;

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE];
    uint8   source[ETHER_MAC_SIZE];
    uint16  ethertype;

    uint32  length;
    uint8   *data;
    uint16  crc;
} ether_info_packet_t;

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
} ip_header_t;

typedef struct
{
    uint8   type __attribute__ ((packed));
    uint8   code __attribute__ ((packed));
    uint16  checksum __attribute__ ((packed));
    uint32  misc __attribute__ ((packed));
} icmp_header_t;

typedef struct {
  uint16    src __attribute__ ((packed));
  uint16    dest __attribute__ ((packed));
  uint16    length __attribute__ ((packed));
  uint16    checksum __attribute__ ((packed));
  uint8     data[1] __attribute__ ((packed));
} udp_header_t;

extern uint8 frame_out_buffer[NET_MAX_PACKET];

void ip_handle_packet(ether_info_packet_t *frame);
void net_handle_frame(uint8 *frame_data, uint32 frame_size);
void icmp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 icmp_data_length);

#endif
