/*  voot.h

    $Id: voot.h,v 1.1 2002/06/11 20:33:48 quad Exp $

*/ 

#ifndef __COMMON_VOOT_H__
#define __COMMON_VOOT_H__

/* NOTE: We broke our no-standard header rule. */

#include <stdarg.h>

#include "vars.h"
#include "biudp.h"
#include "net.h"

#define VOOT_PACKET_HEADER_SIZE         3
#define VOOT_PACKET_BUFFER_SIZE         (BIUDP_SEGMENT_SIZE - sizeof(voot_packet_header))

#define VOOT_PACKET_TYPE_DEBUG          'd'
#define VOOT_PACKET_TYPE_DATA           '>'
#define VOOT_PACKET_TYPE_HUD            'h'
#define VOOT_PACKET_TYPE_COMMAND        'c'
#define VOOT_PACKET_TYPE_DUMP           'D'

#define VOOT_COMMAND_TYPE_DEBUG         '?'
#define VOOT_COMMAND_TYPE_TIME          't'
#define VOOT_COMMAND_TYPE_VERSION       'v'
#define VOOT_COMMAND_TYPE_DUMPON        'D'
#define VOOT_COMMAND_TYPE_DUMPOFF       'd'
#define VOOT_COMMAND_TYPE_SCREEN        's'
#define VOOT_COMMAND_TYPE_DUMPMEM       'm'
#define VOOT_COMMAND_TYPE_DUMPGAME      'g'
#define VOOT_COMMAND_TYPE_DUMPSELECT    'S'

typedef struct
{
    uint8   type    __attribute__ ((packed));
    uint16  size    __attribute__ ((packed));
} voot_packet_header;

typedef struct
{
    voot_packet_header  header                          __attribute__ ((packed));
    uint8               buffer[VOOT_PACKET_BUFFER_SIZE] __attribute__ ((packed));
} voot_packet;

bool    voot_handle_packet  (ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length);
bool    voot_send_packet    (uint8 type, const uint8 *data, uint32 data_size);
bool    voot_send_command   (uint8 type);
void    voot_dump_buffer    (const uint8 *in_data, uint32 in_data_length);
int32   voot_aprintf        (uint8 type, const char *fmt, va_list args);
int32   voot_printf         (uint8 type, const char *fmt, ...);
int32   __voot_debug        (const char *fmt, ...);

#ifdef DEBUG
    #define voot_debug(args...)     __voot_debug(args)
#else
    #define voot_debug(args...)     ;
#endif

#endif
