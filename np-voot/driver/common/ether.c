/*  ether.c

    $Id: ether.c,v 1.1 2002/06/23 23:18:05 quad Exp $

DESCRIPTION

    Ethernet control module.

    Acts as a gateway between the NETWORK LAYER and the HARDWARE LAYER.

*/

#include "vars.h"
#include "rtl8139c.h"
#include "util.h"
#include "net.h"

#include "ether.h"

/* NOTE: Miscellaneous utility functions. No logic occurs within these. */

/*
    NOTE: Recognize between an Ethernet II and 802.3 SNAP frame, and
    populate a ether_info_packet_t structure appropriately.
*/

static ether_info_packet_t ether_discover_frame (const uint8 *frame_data, uint32 frame_size)
{
    ether_8023_header_t    *frame_8023;
    ether_ii_header_t      *frame_ii;
    ether_info_packet_t     frame;
    uint16                  maybe_ethertype;

    frame_8023  = (ether_8023_header_t *) frame_data;
    frame_ii    = (ether_ii_header_t *) frame_data;
    frame.raw   = frame_data;

    maybe_ethertype = ntohs (frame_ii->ethertype);

    /* STAGE: Determine Ethernet frame type. */

    if (maybe_ethertype >= 0x0600)
    {
        /* STAGE: It's an Ethernet II packet. */

        memcpy (frame.dest, frame_ii->dest, sizeof (frame.dest));
        memcpy (frame.source, frame_ii->source, sizeof (frame.source));

        frame.ethertype = maybe_ethertype;

        frame.length    = frame_size - sizeof (ether_ii_header_t);
        frame.data      = frame_data + sizeof (ether_ii_header_t);
    }
    else if (frame_8023->dsap == 0xAA)
    {
        /* STAGE: 802.3 SNAP provides backwards compatibility to Ethernet II. */

        memcpy (frame.dest, frame_8023->dest, sizeof (frame.dest));
        memcpy (frame.source, frame_8023->source, sizeof (frame.source));

        frame.ethertype = ntohs (frame_8023->ethertype);

        frame.length    = ntohs (frame_8023->length);
        frame.data      = frame_data + sizeof (ether_8023_header_t);
    }
    else
    {
        /* STAGE: We couldn't determine the frame type. */

        frame.ethertype = 0x0;
    }

    return frame;
}

/* NOTE: Exposed interface, communicates with NETWORK LAYER and HARDWARE LAYER. */

bool ether_init (void)
{
    /* TODO: Dummy function directly accessing the RTL. */

    return rtl_init ();
}

bool ether_tx (const uint8 *frame, uint32 frame_size)
{
    /* TODO: Dummy function directly accessing the RTL. */

    return rtl_tx (frame, frame_size);
}

uint8* ether_mac (void)
{
    /* TODO: Dummy function directly accessing the RTL. */

    return rtl_mac ();    
}

bool ether_handle_frame (const uint8* frame_data, uint32 frame_size)
{
    ether_info_packet_t frame;

    /*
        STAGE: Determine the type of ethernet frame, and fill in the info
        structure.
    */

    frame = ether_discover_frame (frame_data, frame_size);

    /*
        STAGE: Handle the specified ethertype.

        TODO: If we end up requiring multiple ethertypes, change this into a
        handler chain.
    */

    switch (frame.ethertype)
    {
        /* STAGE: Handle IP ethertype frames. */

        case 0x0800 :
            return ip_handle_packet (&frame);

        default :
            return FALSE;
    }
}
