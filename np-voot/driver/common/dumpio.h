/*  dumpio.h

    $Id: dumpio.h,v 1.3 2002/06/20 10:20:04 quad Exp $

*/

#ifndef __COMMON_DUMPIO_H__
#define __COMMON_DUMPIO_H__

#include "voot.h"

#define UPSCALE_5_STYLE(bits)   (((bits) << 3) | ((bits) >> 2))
#define UPSCALE_6_STYLE(bits)   (((bits) << 2) | ((bits) >> 4))

#define RED_565_TO_INT(color)   UPSCALE_5_STYLE(((color) >> 11) & 0x1F)
#define GREEN_565_TO_INT(color) UPSCALE_6_STYLE(((color) >> 5) & 0x3F)
#define BLUE_565_TO_INT(color)  UPSCALE_5_STYLE((color) & 0x1F)

typedef struct
{
    uint32 target;
    uint32 index;
} dump_control_t;

/* NOTE: Module definitions. */

void    dump_framebuffer    (void);
void    dump_buffer         (const uint8 *in_data, uint32 in_data_length);
void    dump_add            (const uint8 *in_data, uint32 in_data_size);
void    dump_start          (uint32 target_loc);
uint32  dump_stop           (void);
bool    dump_packet_handler (voot_packet *packet);
void    dump_init           (void);

#endif
