/*  voot.c

    $Id: voot.c,v 1.2 2002/11/14 20:56:08 quad Exp $

DESCRIPTION

    VOOT netplay protocol debug implementation.

TODO

    Remove biudp dependence and implement airhook layer.

    Write a better packet handler chain function. Something more general and
    portable to other handlers.

    Handle initialization cleaner!

*/

#include <vars.h>
#include <util.h>
#include <malloc.h>
#include <printf.h>

#include "lwip/udp.h"

#include "voot.h"

static voot_packet_handler_f    voot_packet_handler_chain = voot_packet_handle_default;
static struct udp_pcb          *voot_pcb;

/* NOTE: This is guaranteed to be last in its chain. */

static void voot_handle_packet (void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, uint16 port)
{
    voot_packet *packet;

    packet = p->payload;

    /* STAGE: Ensure the packet size is enough for a basic header. */

    if (p->len < sizeof (packet->header))
    {
        pbuf_free (p);
        return;
    }

    /* STAGE: Fix the size byte order. */

    packet->header.size = ntohs (packet->header.size);

    /* STAGE: Ensure the packet size is correct (or at least manageable.) */

    if (p->len < (sizeof (packet->header) + packet->header.size))
    {
        pbuf_free (p);
        return;
    }

    /* STAGE: Connect the UDP endpoint to whoever sent to us. */

    udp_connect (upcb, addr, port);

    /* STAGE: Pass on to the first packet handler. */

    if (voot_packet_handler_chain)
    {
        /*
            STAGE: If someone in the chain wants to retain the packet, they
            simply return TRUE.
        */

        if (!voot_packet_handler_chain (packet, p))
            pbuf_free (p);
    }
}

bool voot_packet_handle_default (voot_packet *packet, void *ref)
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

                voot_printf (VOOT_PACKET_TYPE_DEBUG, "VOX common, PRE-RELEASE [mem: %u block: %u]", freesize, max_freesize);
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


bool voot_send_packet (uint8 type, const uint8 *data, uint32 data_size)
{
	voot_packet_header *netout;
	struct pbuf        *p;
	bool                retval;

    if (!voot_pcb)
        return FALSE;

    /*
        STAGE: Make sure the dimensions are legit and we have enough space
        for data_size + NULL.
    */

    if (data_size > VOOT_PACKET_BUFFER_SIZE)
        return FALSE;

    /* STAGE: Malloc the full-sized voot_packet. */

    p = pbuf_alloc (PBUF_TRANSPORT, sizeof (voot_packet_header) + data_size + 1, PBUF_RAM);

    if (!p)
        return FALSE;

    /* STAGE: Set the packet header information, including the NULL. */

    netout          = p->payload;
    netout->type    = type;
    netout->size    = htons (data_size + 1);

    /* STAGE: Copy over the input buffer data and append NULL. */

    memcpy (p->payload + sizeof (voot_packet_header), data, data_size);
    ((uint8 *) p->payload)[sizeof (voot_packet_header) + data_size] = 0x0;

    /* STAGE: Transmit the packet. */

    retval = !udp_send (voot_pcb, p);

    /* STAGE: Free the buffer and return. */

    pbuf_free (p);

    return retval;
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

bool voot_send_command (uint8 type)
{
    /* STAGE: Return with a true boolean. */

    return !!voot_printf (VOOT_PACKET_TYPE_COMMAND, "%c", type);
}

void voot_init (void)
{
    voot_pcb = udp_new ();

    if (voot_pcb)
    {
        udp_bind (voot_pcb, IP_ADDR_ANY, VOOT_UDP_PORT);
        udp_recv (voot_pcb, voot_handle_packet, NULL);
    }
}
