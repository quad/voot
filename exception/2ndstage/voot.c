/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "trap.h"
#include "util.h"
#include "printf.h"
#include "dumpio.h"
#include "gamedata.h"

#include "voot.h"

static bool maybe_handle_command(uint8 command, voot_packet *packet)
{
    switch(command)
    {
        case VOOT_COMMAND_TYPE_HEALTH:
        {
            volatile uint16 *p1_health = (uint16 *) 0x8CCF6284;
            volatile uint16 *p2_health = (uint16 *) 0x8CCF7402;

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "p1_health = %u p2_health = %u", *p1_health, *p2_health);

            break;
        }

        case VOOT_COMMAND_TYPE_TIME:
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "%u", time());
            break;

        case VOOT_COMMAND_TYPE_VERSION:
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Netplay VOOT Extensions, BETA");
            break;

        case VOOT_COMMAND_TYPE_PASVON:
            trap_set_passive(TRUE);
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Passive monitoring ON!");
            break;

        case VOOT_COMMAND_TYPE_DUMPON:
        {
            uint32 address;

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Received DUMPON command...");

            address = ((uint32 *) packet->buffer)[1];

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Receiving dump to address %u", address);

#if 0
            dump_start(address);
#endif

            break;
        }

        case VOOT_COMMAND_TYPE_DUMPOFF:
            dump_stop();
            break;

        /* TODO: After taking a certain number of screenshots, it appears to
            crash the system. Maybe the dump_framebuffer() call should be
            moved into the heartbeat logic and some simple IPC be
            implemented? */

        case VOOT_COMMAND_TYPE_SCREEN:
            dump_framebuffer();
            break;

        case VOOT_COMMAND_TYPE_DUMPMEM:
            voot_dump_buffer((const uint8 *) SYS_MEM_START, SYS_MEM_END - SYS_MEM_START);
            break;

        case VOOT_COMMAND_TYPE_DUMPGAME:
            voot_dump_buffer((uint8 *) VOOT_MEM_START, VOOT_MEM_END - VOOT_MEM_START);
            break;

        default:
            break;
    }

    return FALSE;
}

static bool maybe_handle_voot(voot_packet *packet, udp_header_t *udp, uint16 udp_data_length)
{
    /* STAGE: Fix the size byte order. */
    packet->header.size = ntohs(packet->header.size);

    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND:
            return maybe_handle_command(packet->buffer[0], packet);

        case VOOT_PACKET_TYPE_DATA:
            trap_inject_data(packet->buffer, packet->header.size - 1);
            break;

        case VOOT_PACKET_TYPE_DUMP:
            dump_add(packet->buffer, packet->header.size - 1);
            break;

        default:
            break;
    }

    return FALSE;
}

bool voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    voot_packet *packet;

    packet = (voot_packet *) ((uint8 *) udp + sizeof(udp_header_t));
    return maybe_handle_voot(packet, udp, udp_data_length);
}

bool voot_send_packet(uint8 type, const uint8 *data, uint32 data_size)
{
	voot_packet *netout;

    /* STAGE: Make sure the dimensions are legit and we have enough space
        for data_size + NULL */
    if ((data_size >= sizeof(netout->buffer)) || !data_size)
        return FALSE;

    /* STAGE: Malloc the full-sized voot_packet. */
    netout = malloc(sizeof(voot_packet));
    if (!netout)
        return FALSE;   /* We didn't send any data. */

    /* STAGE: Set the packet header information, including the NULL */
    netout->header.type = type;
    netout->header.size = htons(data_size + 1);

    /* STAGE: Copy over the input buffer data and append NULL. */
    memcpy(netout->buffer, data, data_size);
    netout->buffer[data_size] = 0x0;

    /* STAGE: Transmit the packet. */
    biudp_write_buffer((const uint8 *) netout, VOOT_PACKET_HEADER_SIZE + data_size + 1);

    /* STAGE: Free the buffer and return. */
    free(netout);

    return TRUE;
}

bool voot_send_command(uint8 type)
{
    return voot_printf(VOOT_PACKET_TYPE_COMMAND, "%c", type);
}

void voot_dump_buffer(const uint8 *in_data, uint32 in_data_length)
{
    uint32 index, remain, segment_size;
    voot_packet *sizer;

    segment_size = sizeof(sizer->buffer) - 1;

    voot_send_command(VOOT_COMMAND_TYPE_DUMPON);

    for (index = 0; index < (in_data_length / segment_size); index++)
    {
        const uint8 *in_data_segment;

        in_data_segment = in_data + (segment_size * index);

        if (!voot_send_packet(VOOT_PACKET_TYPE_DUMP, in_data_segment, segment_size))
        {
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Error sending packet in main dump loop! Abort!");
            break;
        }

        /* STAGE: Delay so we don't flood the receiving system. */
        vid_waitvbl();
    }

    remain = in_data_length % segment_size;
    if (remain)
        voot_send_packet(VOOT_PACKET_TYPE_DUMP, (in_data + in_data_length) - remain, remain);

    voot_send_command(VOOT_COMMAND_TYPE_DUMPOFF);
}

int32 voot_printf(uint8 type, const char *fmt, ...)
{
	va_list args;
	int32 i;
	voot_packet *packet_size_check;
	char *printf_buffer;

    printf_buffer = malloc(sizeof(packet_size_check->buffer));
    if(!printf_buffer)
        return 0;

	va_start(args, fmt);
	i = vsnprintf(printf_buffer, sizeof(packet_size_check->buffer), fmt, args);
	va_end(args);

    voot_send_packet(type, printf_buffer, i);

    free(printf_buffer);

	return i;
}
