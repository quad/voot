/*  voot.h

DESCRIPTION

    The VOOT Netplay protocol parser header file.

CHANGELOG

    Tue Jan 22 17:52:46 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added this changelog entry. The header file has actually been around
        for quite some time.

    Sun Feb 24 17:13:06 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        Updated with the latest revision of the VOOT protocol. This will
        happen a lot.

*/

#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"

#define VOOT_SLAVE_PORT     5007
#define VOOT_SERVER_PORT    5008

#define VOOT_PACKET_TYPE_DEBUG      'd'
#define VOOT_PACKET_TYPE_DATA       '>'
#define VOOT_PACKET_TYPE_HUD        'h'
#define VOOT_PACKET_TYPE_COMMAND    'c'
#define VOOT_PACKET_TYPE_DUMP       'D'

#define VOOT_COMMAND_TYPE_HEALTH    'h'
#define VOOT_COMMAND_TYPE_TIME      't'
#define VOOT_COMMAND_TYPE_VERSION   'v'
#define VOOT_COMMAND_TYPE_PASVON    'P'
#define VOOT_COMMAND_TYPE_DUMPON    'D'
#define VOOT_COMMAND_TYPE_DUMPOFF   'd'
#define VOOT_COMMAND_TYPE_SCREEN    's'
#define VOOT_COMMAND_TYPE_DUMPMEM   'S'
#define VOOT_COMMAND_TYPE_DUMPGAME  'g'

#define VOOT_PACKET_HEADER_SIZE     3
#define BIUDP_SEGMENT_SIZE          1024

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
void voot_send_data(int32 socket, uint8 packet_type, uint8 *data, uint32 data_size);

#endif __VOOT_H__
