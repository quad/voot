#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"

#define VOOT_SLAVE_PORT     5007
#define VOOT_SERVER_PORT    5008

#define VOOT_PACKET_TYPE_DEBUG      'd'
#define VOOT_PACKET_TYPE_DATA       '>'
#define VOOT_PACKET_TYPE_HUD        'h'
#define VOOT_PACKET_TYPE_GD_ULOAD   'g'
#define VOOT_PACKET_TYPE_GD_DLOAD   'G'
#define VOOT_PACKET_TYPE_COMMAND    'c'

#define VOOT_COMMAND_TYPE_INJECTTST 'i'
#define VOOT_COMMAND_TYPE_MALLOCTST 'm'
#define VOOT_COMMAND_TYPE_NETSTAT   'n'
#define VOOT_COMMAND_TYPE_HEALTH    'p'
#define VOOT_COMMAND_TYPE_TIME      't'
#define VOOT_COMMAND_TYPE_VERSION   'v'

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

uint32 voot_check_packet_advsize(voot_packet *packet, uint32 size);
voot_packet* voot_parse_buffer(uint8 *buffer, uint32 buffer_size);
voot_packet* voot_parse_socket(int32 socket);
int32 voot_send_packet(int32 socket, voot_packet *packet, uint32 size);
int32 voot_send_command(int32 socket, uint8 command);

#endif __VOOT_H__
