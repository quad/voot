/*  dumpio.c

    $Id: dumpio.c,v 1.1 2002/06/11 23:53:36 quad Exp $

DESCRIPTION

    Dump IO protocol handling logic.

*/

#include "vars.h"
#include "assert.h"
#include "voot.h"
#include "util.h"
#include "system.h"

#include "dumpio.h"

static dump_control_t   control;

void dump_framebuffer (void)
{
    uint32  index;
    uint16 *vram_start;

    /* STAGE: Inform the client of the outgoing data dump. */

    voot_send_command (VOOT_COMMAND_TYPE_DUMPON);

    /* STAGE: Format into strips that fit under the 1k packet limit. */

    #define MAP_NUM_PIXELS  (640 * 480)
    #define STRIP_SIZE      (VOOT_PACKET_BUFFER_SIZE / 3)

    /* STAGE: Release the data back in sectioned strips. */

    vram_start = VIDEO_VRAM_START;

    for (index = 0; index <= MAP_NUM_PIXELS; index++)
    {
        uint8 strip[STRIP_SIZE][3];
                
        strip[index % STRIP_SIZE][0] = RED_565_TO_INT(vram_start[index]);
        strip[index % STRIP_SIZE][1] = GREEN_565_TO_INT(vram_start[index]);
        strip[index % STRIP_SIZE][2] = BLUE_565_TO_INT(vram_start[index]);

        if ((index % STRIP_SIZE) == STRIP_SIZE - 1)
            voot_send_packet (VOOT_PACKET_TYPE_DUMP, (uint8 *) strip, sizeof (strip));
    }

    /* STAGE: Data dump is finished. */

    voot_send_command (VOOT_COMMAND_TYPE_DUMPOFF);
}

void dump_add (const uint8 *in_data, uint32 in_data_size)
{
    if (!control.target)
        return;

    memcpy ((uint8 *) (control.target + control.index), in_data, in_data_size);
    assert (!memcmp((uint8 *) (control.target + control.index), in_data, in_data_size));

    control.index += in_data_size;
}

void dump_start (uint32 target_loc)
{
    control.target  = target_loc;
    control.index   = 0;
}

uint32 dump_stop (void)
{
    uint32  num_bytes_dumped;

    num_bytes_dumped = control.index;

    control.target = control.index = 0;

    return num_bytes_dumped;
}
