/*  biudp.h

    $Id: biudp.h,v 1.1 2002/06/11 20:32:53 quad Exp $

*/

#ifndef __COMMON_BIUDP_H__
#define __COMMON_BIUDP_H__

#include "vars.h"
#include "net.h"

#define BIUDP_SEGMENT_SIZE  1024

typedef struct
{
    uint8   dest_mac[ETHER_MAC_SIZE];
    uint32  source_ip;
    uint32  dest_ip;
    uint16  port;
    bool    initialized;
} biudp_control_t;

void    biudp_init          (const biudp_control_t *in_control);
bool    biudp_write_buffer  (const uint8 *in_data, uint32 in_data_length);
bool    biudp_write         (uint8 in);
bool    biudp_write_str     (const uint8 *in_string);

#endif
