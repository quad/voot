/*  ether.h

    $Id: ether.h,v 1.3 2002/06/24 00:19:17 quad Exp $

*/

#ifndef __COMMON_ETHER_H__
#define __COMMON_ETHER_H__

#define ETHER_MAC_SIZE          6
#define ETHER_TYPE_IP           0x0800

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE]    __attribute__ ((packed));
    uint8   source[ETHER_MAC_SIZE]  __attribute__ ((packed));
    uint16  ethertype               __attribute__ ((packed));
} ether_ii_header_t;

typedef struct
{
    uint8   dest[ETHER_MAC_SIZE]    __attribute__ ((packed));
    uint8   source[ETHER_MAC_SIZE]  __attribute__ ((packed));
    uint16  length                  __attribute__ ((packed));
    uint8   dsap                    __attribute__ ((packed));
    uint8   ssap                    __attribute__ ((packed));
    uint8   control                 __attribute__ ((packed));
    uint8   vendor_code[3]          __attribute__ ((packed));
    uint16  ethertype               __attribute__ ((packed));
} ether_8023_header_t;

typedef struct
{
    uint8           dest[ETHER_MAC_SIZE];
    uint8           source[ETHER_MAC_SIZE];

    uint16          ethertype;
    uint32          length;

    const uint8    *data;
    const uint8    *raw;

    uint16          crc;
} ether_info_packet_t;

/* NOTE: Module definitions. */

bool    ether_init          (void);
bool    ether_transmit      (ether_info_packet_t *frame_in);
uint8 * ether_mac           (void);
bool    ether_handle_frame  (const uint8* data, uint32 data_size);

#endif
