/*  biudp.c

    $Id: biudp.c,v 1.9 2002/06/30 09:15:06 quad Exp $

DESCRIPTION

    Bi-directional UDP communications code.

TODO

    Remove and replace with airhook protocol layer.

*/

#include "vars.h"
#include "ether.h"
#include "util.h"
#include "malloc.h"
#include "video.h"

#include "biudp.h"

static biudp_control_t  control;

void biudp_init (const biudp_control_t *in_control)
{
    /* STAGE: Copy the input control into global control structure. */

    memcpy (&control, in_control, sizeof (biudp_control_t));

    control.initialized = 1;
}

static bool biudp_write_segment (const uint8 *in_data, uint32 in_data_length)
{
    ether_info_packet_t frame_out;
    ip_header_t        *ip;
    udp_header_t       *udp;
    uint16              ip_length;
    uint16              ip_header_length;
    bool                retval;

    /* STAGE: Ensure we're not oversize. */

    if (in_data_length > BIUDP_SEGMENT_SIZE)
        return FALSE;

    /* STAGE: Use Ethernet II. Everyone MUST support it. */

    ip_length = sizeof (ip_header_t) + sizeof (udp_header_t) + in_data_length;

    /* STAGE: malloc() the proper size output buffer. */

    ip = malloc (ip_length);

    if (!ip)
        return FALSE;

    /* STAGE: Setup the frame layer. */

    memcpy (frame_out.source, ether_mac (), ETHER_MAC_SIZE);
    memcpy (frame_out.dest, control.dest_mac, ETHER_MAC_SIZE);

    frame_out.ethertype = ETHER_TYPE_IP;
    frame_out.length    = ip_length;
    frame_out.data      = (uint8 *) ip;

    /* STAGE: Setup the IP layer. */

    ip->version_ihl         = 0x45;                 /* 4 is the IP version, 5 is the size of the header in 32-bit segments. No options and no padding. */
    ip->tos                 = 0x14;                 /* Normal precedence, low delay, high reliability - this is all we want but we'll never get. */
    ip->length              = htons (ip_length);
    ip->packet_id           = 0;
    ip->flags_frag_offset   = htons (0x4000);       /* One order of IP packet, hold the fragmentation. */
    ip->ttl                 = 0x20;                 /* 32 seems like plenty - it gets me to Kirk. */
    ip->protocol            = IP_PROTO_UDP;
    ip->checksum            = 0;

    SAFE_UINT32_COPY(ip->source, control.source_ip);
    SAFE_UINT32_COPY(ip->dest, control.dest_ip);

    /* STAGE: Calculate the IP checksum last. */

    ip_header_length    = IP_HEADER_SIZE(ip);
    ip->checksum        = ip_checksum (ip, ip_header_length);

    /* STAGE: Identify the UDP layer. */

    udp = (udp_header_t *) ((uint8 *) ip + ip_header_length);

    /* STAGE: Setup the UDP layer. */

    udp->src    = htons (UDP_PORT_VOOT);
    udp->dest   = control.port;
    udp->length = htons (sizeof (udp_header_t) + in_data_length);

    /* STAGE: So, how about that data? */

    memcpy ((uint8 *) udp + sizeof (udp_header_t), in_data, in_data_length);

    /* STAGE: Calculate the UDP checksum last. */

    udp->checksum = udp_checksum (ip, ip_header_length);

    /* STAGE: ... and transmit it, god willing. */

    retval = ether_transmit (&frame_out);

    /* STAGE: Free the output buffer. */

    free (ip);

    return retval;
}

bool biudp_write_buffer (const uint8 *in_data, uint32 in_data_length)
{
    uint32  index;
    uint32  remain;

    if (!control.initialized)
        return FALSE;

    /*
        STAGE: Split the incoming data into BIUDP_SEGMENT_SIZE byte chunks
        and feed those out.
    */

    for (index = 0; index < (in_data_length / BIUDP_SEGMENT_SIZE); index++)
    {
        const uint8    *in_data_segment;

        in_data_segment = in_data + (BIUDP_SEGMENT_SIZE * index);

        if (!biudp_write_segment (in_data_segment, BIUDP_SEGMENT_SIZE))
            return FALSE;

        video_waitvbl ();
    }

    /* STAGE: Handle any remaining data... */

    remain = in_data_length % BIUDP_SEGMENT_SIZE;

    if (remain)
        return biudp_write_segment ((in_data + in_data_length) - remain, remain);
    else
        return TRUE;
}

bool biudp_write (uint8 in)
{
    return biudp_write_buffer (&in, sizeof(uint8));
}

bool biudp_write_str (const uint8 *in_string)
{
    return biudp_write_buffer (in_string, strlen(in_string));
}
