/*  voot.c

    $Id: voot.c,v 1.7 2002/10/18 19:52:19 quad Exp $

DESCRIPTION

    VOOT netplay protocol debug implementation.

TODO

    Remove biudp dependence and implement airhook layer.

    Write a better packet handler chain function. Something more general and
    portable to other handlers.

*/

#include "vars.h"
#include "biudp.h"
#include "util.h"
#include "malloc.h"
#include "printf.h"

#include "voot.h"

static voot_packet_handler_f    voot_packet_handler_chain = voot_packet_handle_default;

/* NOTE: This is guaranteed to be last in its chain. */

bool voot_packet_handle_default (voot_packet *packet)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND :
        {
            /* STAGE: Ensure there is actually a command. */

            if (!(packet->header.size))
                break;

            /* STAGE: Handle the version command. */

            if (packet->buffer[0] == VOOT_COMMAND_TYPE_VERSION)
            {
                uint32  freesize;
                uint32  max_freesize;

                malloc_stat (&freesize, &max_freesize);

                voot_debug ("VOX common, PRE-RELEASE [mem: %u block: %u]", freesize, max_freesize);
            }

            break;
        }

        default :
            break;
    }

    return FALSE;
}

void* voot_add_packet_chain (voot_packet_handler_f function)
{
    voot_packet_handler_f old_function;

    /* STAGE: Switch out the functions. */

    old_function = voot_packet_handler_chain;
    voot_packet_handler_chain = function;

    /* STAGE: Give them the old one so they can properly handle it. */

    return old_function;
}

bool voot_handle_packet (ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    voot_packet *packet;

    packet = (voot_packet *) ((uint8 *) udp + sizeof (udp_header_t));

    /* STAGE: Fix the size byte order. */

    packet->header.size = ntohs (packet->header.size);

    /* STAGE: Ensure the packet size is correct (or at least manageable.) */

    if ((udp_data_length < sizeof (packet->header)) || (udp_data_length < (sizeof (packet->header) + packet->header.size)))
        return FALSE;

    /* STAGE: Pass on to the first packet handler. */

    if (voot_packet_handler_chain)
        return voot_packet_handler_chain (packet);
    else
        return FALSE;
}

bool voot_send_packet (uint8 type, const uint8 *data, uint32 data_size)
{
	voot_packet    *netout;
	bool            retval;

    /*
        STAGE: Make sure the dimensions are legit and we have enough space
        for data_size + NULL.
    */

    if (data_size > sizeof (netout->buffer))
        return FALSE;

    /* STAGE: Malloc the full-sized voot_packet. */

    netout = malloc (sizeof (voot_packet));

    if (!netout)
        return FALSE;

    /* STAGE: Set the packet header information, including the NULL. */

    netout->header.type = type;
    netout->header.size = htons (data_size + 1);

    /* STAGE: Copy over the input buffer data and append NULL. */

    memcpy (netout->buffer, data, data_size);
    netout->buffer[data_size] = 0x0;

    /* STAGE: Transmit the packet. */

    retval = biudp_write_buffer ((const uint8 *) netout, sizeof (voot_packet_header) + data_size + 1);

    /* STAGE: Free the buffer and return. */

    free (netout);

    return retval;
}

bool voot_send_command (uint8 type)
{
    /* STAGE: Return with a true boolean. */

    return !!voot_printf (VOOT_PACKET_TYPE_COMMAND, "%c", type);
}

int32 voot_aprintf (uint8 type, const char *fmt, va_list args)
{
	int32           i;
	voot_packet    *packet_size_check;
	char           *printf_buffer;

    /* STAGE: Allocate the largest possible buffer for the printf. */

    printf_buffer = malloc (sizeof (packet_size_check->buffer));

    if (!printf_buffer)
        return 0;

    /* STAGE: Actually perform the printf. */

	i = vsnprintf (printf_buffer, sizeof (packet_size_check->buffer), fmt, args);

    /* STAGE: Send the packet, if we need to, and maintain correctness. */

    if (i && !voot_send_packet (type, printf_buffer, i))
        i = 0;

    /* STAGE: Keep our memory clean. */

    free (printf_buffer);

	return i;  
}

int32 voot_printf (uint8 type, const char *fmt, ...)
{
	va_list args;
	int32   i;

	va_start (args, fmt);
	i = voot_aprintf (type, fmt, args);
	va_end (args);

	return i;
}
