/*  biudp.h

    $Id: biudp.h,v 1.2 2002/06/20 10:20:04 quad Exp $

*/

#ifndef __COMMON_BIUDP_H__
#define __COMMON_BIUDP_H__

#include "vars.h"
#include "net.h"

#define BIUDP_SEGMENT_SIZE  (NET_MAX_PACKET - sizeof (ether_ii_header_t) - (sizeof (ip_header_t) + sizeof (udp_header_t)))

typedef struct
{
    uint8   dest_mac[ETHER_MAC_SIZE];
    uint32  source_ip;
    uint32  dest_ip;
    uint16  port;
    bool    initialized;
} biudp_control_t;

/* NOTE: Module definitions. */

void    biudp_init          (const biudp_control_t *in_control);
bool    biudp_write_buffer  (const uint8 *in_data, uint32 in_data_length);
bool    biudp_write         (uint8 in);
bool    biudp_write_str     (const uint8 *in_string);

#endif
