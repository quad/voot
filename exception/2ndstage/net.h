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

#define IP_ADDR_COPY(trgt, src)     {                           \
    *(((uint16 *) &(trgt))    ) = *(((uint16 *) &(src))    );   \
    *(((uint16 *) &(trgt)) + 1) = *(((uint16 *) &(src)) + 1);   \
                                    }

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
} ether_ii_header_t __attribute__ ((packed));

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
} ether_8023_header_t __attribute__ ((packed));

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE];
    uint8   source[ETHER_MAC_SIZE];
    uint16  ethertype;

    uint32  length;
    uint8   *data;
    uint16  crc;
} ether_info_packet_t __attribute__ ((packed));

typedef struct
{
    uint8   version_ihl;
    uint8   tos;
    uint16  length;
    uint16  packet_id;
    uint16  flags_frag_offset;
    uint8   ttl;
    uint8   protocol;
    uint16  checksum;
    uint32  source;
    uint32  dest;
} ip_header_t __attribute__ ((packed));

typedef struct
{
    uint8   type;
    uint8   code;
    uint16  checksum;
    uint32  misc;
} icmp_header_t __attribute__ ((packed));

typedef struct {
    uint16  src;
    uint16  dest;
    uint16  length;
    uint16  checksum;
} udp_header_t __attribute__ ((packed));

typedef struct {
    uint32  source_ip;
    uint32  dest_ip;
    uint8   zero;
    uint8   protocol;
    uint16  length;
} udp_pseudo_header_t __attribute__ ((packed));

void net_transmit(ether_info_packet_t *frame_in);
uint16 udp_checksum(ip_header_t *ip, uint16 ip_header_length);
uint16 icmp_checksum(icmp_header_t *icmp, uint16 icmp_header_length);
uint16 ip_checksum(ip_header_t *ip, uint16 ip_header_length);
void net_handle_frame(uint8 *frame_data, uint32 frame_size);

#endif
