#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"

#define VOOT_PACKET_TYPE_DEBUG      'd'
#define VOOT_PACKET_TYPE_DATA       '>'
#define VOOT_PACKET_TYPE_HUD        'h'
#define VOOT_PACKET_TYPE_GD_ULOAD   'g'
#define VOOT_PACKET_TYPE_GD_DLOAD   'G'
#define VOOT_PACKET_TYPE_COMMAND    'c'

#define BIUDP_SEGMENT_SIZE  1024

typedef struct
{
    uint8   type __attribute__ ((packed));
    uint16  size __attribute__ ((packed));
} voot_packet_header;

typedef struct
{
    voot_packet_header  header __attribute__ ((packed));
    uint8               buffer[BIUDP_SEGMENT_SIZE - sizeof(voot_packet_header)] __attribute__ ((packed));
} voot_packet;

voot_packet* voot_parse_buffer(uint8 *buffer, uint32 buffer_size);
voot_packet* voot_parse_socket(int32 socket);

#endif __VOOT_H__
