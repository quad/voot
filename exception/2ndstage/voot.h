#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"
#include "biudp.h"
#include "net.h"

#define VOOT_PACKET_HEADER_SIZE     3

#define VOOT_PACKET_TYPE_DEBUG      'd'
#define VOOT_PACKET_TYPE_DATA       '>'
#define VOOT_PACKET_TYPE_HUD        'h'
#define VOOT_PACKET_TYPE_GD_ULOAD   'g'
#define VOOT_PACKET_TYPE_GD_DLOAD   'G'
#define VOOT_PACKET_TYPE_COMMAND    'c'
#define VOOT_PACKET_TYPE_DUMP       'D'

#define VOOT_COMMAND_TYPE_INJECTTST 'i'
#define VOOT_COMMAND_TYPE_NETSTAT   'n'
#define VOOT_COMMAND_TYPE_HEALTH    'h'
#define VOOT_COMMAND_TYPE_TIME      't'
#define VOOT_COMMAND_TYPE_VERSION   'v'
#define VOOT_COMMAND_TYPE_PASVON    'P'
#define VOOT_COMMAND_TYPE_DUMPON    'D'
#define VOOT_COMMAND_TYPE_DUMPOFF   'd'
#define VOOT_COMMAND_TYPE_SCREEN    's'
#define VOOT_COMMAND_TYPE_DUMPMEM   'S'
#define VOOT_COMMAND_TYPE_DUMPGAME  'g'

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

bool voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length);
bool voot_send_packet(uint8 type, const uint8 *data, uint32 data_size);
bool voot_send_command(uint8 type);
void voot_dump_buffer(const uint8 *in_data, uint32 in_data_length);
int32 voot_printf(uint8 type, const char *fmt, ...);

#endif
