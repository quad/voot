/*  dumpio.c

    $Id: dumpio.c,v 1.7 2002/06/29 12:57:04 quad Exp $

DESCRIPTION

    Dump IO protocol handling logic.

*/

#include "vars.h"
#include "voot.h"
#include "gamedata.h"
#include "util.h"
#include "system.h"
#include "video.h"

#include "dumpio.h"

static dump_control_t           control;
static voot_packet_handler_f    old_voot_packet_handler;

static bool dump_packet_handler (voot_packet *packet)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND :
        {
            uint32  option;

            /* STAGE: Ensure there is actually a command. */

            if (!(packet->header.size))
                break;

            /* STAGE: Determine the command option, if specified. */

            option = 0;

            if (packet->header.size >= 8)
            {
                uint8  *buffer_index;

                /* STAGE: The option isn't byte aligned, so we need to carefully copy it out. */

                buffer_index = (uint8 *) (((uint32 *) packet->buffer) + 1);

                ((uint8 *) (&option))[0] = buffer_index[0];
                ((uint8 *) (&option))[1] = buffer_index[1];
                ((uint8 *) (&option))[2] = buffer_index[2];
                ((uint8 *) (&option))[3] = buffer_index[3];
            }

            /* STAGE: Handle the DUMPIO commands. */

            switch (packet->buffer[0])
            {
                case VOOT_COMMAND_TYPE_DUMPON :
                {
                    dump_start (option);

                    break;
                }

                case VOOT_COMMAND_TYPE_DUMPOFF :
                {
                    uint32  bytes;

                    bytes = dump_stop ();

                    break;
                }

                /*
                    NOTE: After taking a certain number of screenshots, it appears
                    to crash the system.

                    Maybe the dump_framebuffer() call should be moved into the
                    heartbeat logic and some simple IPC be implemented?
                */


                case VOOT_COMMAND_TYPE_SCREEN :
                    dump_framebuffer ();
                    break;

                case VOOT_COMMAND_TYPE_DUMPMEM :
                    dump_buffer ((const uint8 *) SYS_MEM_START, SYS_MEM_END - SYS_MEM_START);
                    break;

                case VOOT_COMMAND_TYPE_DUMPGAME :
                    dump_buffer ((const uint8 *) VOOT_MEM_START, VOOT_MEM_END - VOOT_MEM_START);
                    break;

                case VOOT_COMMAND_TYPE_DUMPSELECT :
                    dump_buffer ((const uint8 *) option, 1024);
                    break;
            }
            
            break;
        }
            
        case VOOT_PACKET_TYPE_DUMP :
            dump_add (packet->buffer, packet->header.size);
            break;
    }

    return old_voot_packet_handler (packet);
}

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

void dump_buffer (const uint8 *in_data, uint32 in_data_length)
{
    uint32          index;
    uint32          remain;
    uint32          segment_size;
    voot_packet    *sizer;

    segment_size = sizeof (sizer->buffer) - 1;

    /* STAGE: Notify the client we're transmitting a dump buffer. */

    voot_send_command (VOOT_COMMAND_TYPE_DUMPON);

    /* STAGE: Dump the specified data. */

    for (index = 0; index < (in_data_length / segment_size); index++)
    {
        const uint8    *in_data_segment;

        in_data_segment = in_data + (segment_size * index);

        if (!voot_send_packet (VOOT_PACKET_TYPE_DUMP, in_data_segment, segment_size))
            break;

        video_waitvbl ();
    }

    /* STAGE: Transmit any remaining trailing data... */

    remain = in_data_length % segment_size;

    if (remain)
        voot_send_packet (VOOT_PACKET_TYPE_DUMP, (in_data + in_data_length) - remain, remain);

    /* STAGE: Notify the client we're done transmitting a dump buffer. */

    voot_send_command (VOOT_COMMAND_TYPE_DUMPOFF);
}

void dump_add (const uint8 *in_data, uint32 in_data_size)
{
    /* STAGE: Ensure we're dumping someplace... */

    if (!control.target)
        return;

    /* STAGE: Copy as much of incoming data in as possible. */

    memcpy ((uint8 *) (control.target + control.index), in_data, in_data_size);

    /* STAGE: Update the control structure of our distance into the buffer. */

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

    /* STAGE: Save the number of bytes dumped. */

    num_bytes_dumped = control.index;

    /* STAGE: Clear the dump control structure. */

    control.target = control.index = 0;

    /* STAGE: Return the number of bytes dumped. */

    return num_bytes_dumped;
}

void dump_init (void)
{
    /* STAGE: Add ourselves to the VOOT packet handling chain. */

    old_voot_packet_handler = voot_add_packet_chain (dump_packet_handler);
}
