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

typedef struct
{
    uint8   type __attribute__ ((packed));
    uint16  size __attribute__ ((packed));
    uint8   buffer[BIUDP_SEGMENT_SIZE - VOOT_PACKET_HEADER_SIZE] __attribute__ ((packed));
} voot_packet __attribute__ ((packed));

/*

Total Guesstimated System Memory:
    8CCF9C00 - 8CCFAFFF (0x13ff / 5119d)

Initial Focus:
    8CCF9EE0 - 8CCF9EFF

*/

#define VOOT_MEM_START      0x8CCF9ECC
#define VOOT_MEM_END        0x8CCFA2CC

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length);

#endif
