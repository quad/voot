/*  voot.h

    $Id: voot.h,v 1.2 2002/12/16 07:51:00 quad Exp $

*/ 

#ifndef __COMMON_VOOT_H__
#define __COMMON_VOOT_H__

/* NOTE: We broke our no-standard header rule. */

#include <stdarg.h>

#define VOOT_PACKET_BUFFER_SIZE         1024
#define VOOT_UDP_PORT                   5007

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
    uint8               padding                         __attribute__ ((packed));
} voot_packet;

typedef bool (* voot_packet_handler_f)  (voot_packet *, void *);

/* NOTE: Module definitions. */

#ifdef DEBUG
    #define voot_debug(args...)     voot_printf(VOOT_PACKET_TYPE_DEBUG , ## args)
#else
    #define voot_debug(args...)     ;
#endif

void *  voot_add_packet_chain       (voot_packet_handler_f function);
bool    voot_send_packet            (uint8 type, const uint8 *data, uint32 data_size);
int32   voot_aprintf                (uint8 type, const char *fmt, va_list args);
int32   voot_printf                 (uint8 type, const char *fmt, ...);
bool    voot_send_command           (uint8 type);
void    voot_init                   (void);

#endif
