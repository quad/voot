#ifndef __BIUDP_H__
#define __BIUDP_H__

#include "vars.h"
#include "net.h"
#include "voot.h"

#define BIUDP_SEGMENT_SIZE  1024 + VOOT_PACKET_HEADER_SIZE

/* #define HARDCODE_IP         1 */

typedef struct
{
    uint8   dest_mac[ETHER_MAC_SIZE];
    uint32  source_ip;
    uint32  dest_ip;
    uint32  port;
    bool    initialized;
} biudp_control_t;

void biudp_init(const biudp_control_t *in_control);
void biudp_write_buffer(const uint8 *in_data, uint32 in_data_length);
void biudp_write(const uint8 in);
void biudp_write_str(const uint8 *in_string);
int32 biudp_printf(uint8 type, const char *fmt, ...);

#endif
