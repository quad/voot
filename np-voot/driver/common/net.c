/*  net.c

    $Id: net.c,v 1.2 2002/06/12 10:29:01 quad Exp $

DESCRIPTION

    Networking sub-system. I intend on handling the following protocols:

    * UDP           - Our primary data transfer protocol.
        * DHCP      - We want to be able to resolve our IP address in
                       foreign environments.
        * VOOT      - The Netplay VOOT protocol.
    * ICMP          - Pings, primarily.

TODO

    Implement DHCP.

*/

#include "vars.h"
#include "rtl8139c.h"
#include "util.h"
#include "malloc.h"
#include "bswap.h"
#include "biudp.h"
#include "voot.h"

#include "net.h"

/*
    Ethernet Subsystem
*/

bool net_transmit (ether_info_packet_t *frame_in)
{
    ether_ii_header_t  *frame_out;
    uint32              frame_out_length;
    bool                retval;

    /* STAGE: malloc() appropriate sized buffer. */

    frame_out_length    = sizeof (ether_ii_header_t) + frame_in->length;
    frame_out           = malloc (frame_out_length);

    if (!frame_out)
        return FALSE;

    /* STAGE: Setup the packet. */

    memcpy (frame_out->source, frame_in->source, ETHER_MAC_SIZE);
    memcpy (frame_out->dest, frame_in->dest, ETHER_MAC_SIZE);
    frame_out->ethertype = htons (frame_in->ethertype);

    /* STAGE: Copy the remaining buffer. */

    memcpy ((uint8 *) frame_out + sizeof (ether_ii_header_t), frame_in->data, frame_in->length);

    /* STAGE: Transmit the packet. */

    retval = rtl_tx ((uint8 *) frame_out, frame_out_length);

    /* STAGE: Free output buffer. */

    free (frame_out);

    return retval;
}

static ether_info_packet_t eth_discover_frame (uint8 *frame_data, uint32 frame_size)
{
    ether_8023_header_t    *frame_8023;
    ether_ii_header_t      *frame_ii;
    ether_info_packet_t     frame;
    uint16                  maybe_ethertype;

    frame_8023  = (ether_8023_header_t *) frame_data;
    frame_ii    = (ether_ii_header_t *) frame_data;

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

/*
    IP Processor

    CREDIT: With all thanks to AndrewK.

    Is this feeling like a cleaner re-implementation of dc-load-ip. Anyone
    else?
*/

static uint16 ip_checksum_add (const uint16 *buf, uint16 count_short, uint32 sum)
{
    while (count_short--)
    {
	    sum += *buf++;

	    if (sum & 0xffff0000)
	    {
	        sum &= 0xffff;
    	    sum++;
    	}
    }

    return sum;
}

static bool ip_packet_ok (ether_info_packet_t *frame, uint16 ip_header_length, uint16 ip_data_length)
{
    ip_header_t *ip;

    ip = (ip_header_t *) frame->data;

    /* TODO: Check IP header version. */

    /* TODO: Check if length matches frame size. */

    /* STAGE: IP Header checksum */

    if (ip->checksum != ip_checksum (ip, ip_header_length))
        return TRUE;

    /* STAGE: Check if the packet is fragmented. We don't support it - yet. */

    if (ntohs (ip->flags_frag_offset) & 0x3fff)
        return TRUE;

    return FALSE;
}

uint16 ip_checksum (ip_header_t *ip, uint16 ip_header_length)
{
    uint16  temp_checksum;
    uint16  calc_checksum;

    /* STAGE: Backup and clear the incoming checksum value. */

    temp_checksum   = ip->checksum;
    ip->checksum    = 0;

    /* STAGE: Calculate the checksum. */

    calc_checksum   = ~(ip_checksum_add ((uint16 *) ip, ip_header_length / 2, 0) & 0xffff);

    /* STAGE: Restore the original incoming checksum value. */

    ip->checksum    = temp_checksum;

    /* STAGE: Return our calculated value. */

    return calc_checksum;
}

bool ip_handle_packet (ether_info_packet_t *frame)
{
    ip_header_t    *ip;
    uint16          ip_data_length;
    uint16          ip_header_length;

    ip = (ip_header_t *) frame->data;

    /* STAGE: Pre-calculate IP header sizes. */

    ip_header_length    = IP_HEADER_SIZE(ip);
    ip_data_length      = ntohs (ip->length) - ip_header_length;

    /* STAGE: Sanity checks on the IP packet. */

    if (ip_packet_ok (frame, ip_header_length, ip_data_length))
        return FALSE;

    /* STAGE: ... and handle the appropriate protocol type! */

    switch (ip->protocol)
    {
        case IP_PROTO_ICMP :
            return icmp_handle_packet (frame, ip_header_length, ip_data_length);

        case IP_PROTO_UDP :
            return udp_handle_packet (frame, ip_header_length, ip_data_length);

        default :
            return FALSE;
    }
}

static void ip_reverse_packet (ether_info_packet_t *frame)
{
    char            temp_mac[ETHER_MAC_SIZE];
    uint32          temp_ipaddr;
    ip_header_t    *ip;

    ip = (ip_header_t *) frame->data;

    /* STAGE: Ether - point to our origiantor. */

    memcpy (temp_mac, frame->source, ETHER_MAC_SIZE);
    memcpy (frame->source, frame->dest, ETHER_MAC_SIZE);
    memcpy (frame->dest, temp_mac, ETHER_MAC_SIZE);

    /* STAGE: IP - point to our originator. */

    SAFE_UINT32_COPY (temp_ipaddr, ip->source);
    SAFE_UINT32_COPY (ip->source, ip->dest);
    SAFE_UINT32_COPY (ip->dest, temp_ipaddr);

    /* STAGE: Compute IP checksum. */

    ip->checksum = ip_checksum (ip, IP_HEADER_SIZE(ip));
}

/*
    ICMP Sub-System
*/

uint16 icmp_checksum (icmp_header_t *icmp, uint16 icmp_header_length)
{
    uint16  temp_checksum;
    uint16  calc_checksum;

    /* STAGE: Back and clear the incoming checksum value. */

    temp_checksum   = icmp->checksum;
    icmp->checksum  = 0;

    /* STAGE: Calculate the checksum. */

    calc_checksum   = ~(ip_checksum_add ((uint16 *) icmp, icmp_header_length / 2, 0) & 0xffff);

    /* STAGE: Restore the original incoming checksum value. */

    icmp->checksum  = temp_checksum;

    /* STAGE: Return our calculated value. */

    return calc_checksum;
}

static bool icmp_echo_reply (ether_info_packet_t *frame, icmp_header_t *icmp, uint16 icmp_data_length)
{
    ip_header_t    *ip;

    ip = (ip_header_t *) frame->data;

    /* STAGE: We want to reply to a ping. */

    icmp->type = ICMP_TYPE_ECHO_REPLY;

    /* STAGE: Reverse the packet's direction. */

    ip_reverse_packet (frame);

    /* STAGE: Compute ICMP checksum. */

    icmp->checksum = icmp_checksum (icmp, icmp_data_length);

    /* STAGE: Transmit it, god willing. */

    return net_transmit (frame);
}

bool icmp_handle_packet (ether_info_packet_t *frame, uint16 ip_header_length, uint16 icmp_data_length)
{
    icmp_header_t  *icmp;

    icmp = (icmp_header_t *) (frame->data + ip_header_length);

    /* STAGE: ICMP header checksum. */

    if (icmp->checksum != icmp_checksum (icmp, icmp_data_length))
        return FALSE;

    /* STAGE: Handle ICMP packet types. */

    switch (icmp->type)
    {
        /* STAGE: Replay to these straight back with its own packet! Woo ha ha! */

        case ICMP_TYPE_ECHO_REQUEST :
            icmp_echo_reply (frame, icmp, icmp_data_length);
            break;

        default :
            break;
    }

    return FALSE;
}

/*
    UDP Sub-System
*/

uint16 udp_checksum (ip_header_t *ip, uint16 ip_header_length)
{
    udp_header_t           *udp;
    uint16                  udp_length;
    udp_pseudo_header_t     udp_top;
    uint16                  temp_checksum;
    uint16                  calc_checksum;

    /* STAGE: Identify the UDP header. */

    udp         = (udp_header_t *) ((uint8 *) ip + ip_header_length);
    udp_length  = ntohs (udp->length);

    /* STAGE: Back and clear the incoming checksum value. */

    temp_checksum = udp->checksum;
    udp->checksum = 0;

    /* STAGE: Construct the UDP psuedo-header. */

    SAFE_UINT32_COPY(udp_top.source_ip, ip->source);
    SAFE_UINT32_COPY(udp_top.dest_ip, ip->dest);

    udp_top.zero        = 0;
    udp_top.protocol    = ip->protocol;
    udp_top.length      = udp->length;

    /* STAGE: Calculate the first half of the checksum using the UDP pseudo-header. */

    calc_checksum = ip_checksum_add ((uint16 *) (&udp_top), sizeof (udp_top) / sizeof (uint16), 0);

    /* STAGE: Pad the UDP length for alignment. */

    if (udp_length % 2)
    {
        ((uint8 *) udp)[udp_length] = 0;
        udp_length++;
    }

    /* STAGE: Calculate the second half of the checksum using the actual UDP packet. */

    calc_checksum = ip_checksum_add ((uint16 *) udp, udp_length / sizeof (uint16), calc_checksum);

    /* STAGE: Restore the incoming checksum value. */

    udp->checksum = temp_checksum;

    return ~(calc_checksum & 0xffff);
}

static bool udp_echo_reply (ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    uint16          temp_port;
    uint16          ip_header_size;
    ip_header_t    *ip;

    ip              = (ip_header_t *) frame->data;
    ip_header_size  = IP_HEADER_SIZE(ip);

    /* STAGE: Reverse the packet's direction. */

    ip_reverse_packet (frame);

    /* STAGE: UDP - we want to reply from our echo port to their port. */

    temp_port   = udp->src;
    udp->src    = udp->dest;
    udp->dest   = temp_port;

    /* STAGE: Compute UDP checksum. */

    udp->checksum = udp_checksum (ip, ip_header_size);

    /* STAGE: Transmit it, god willing. */

    return net_transmit (frame);
}

bool udp_handle_packet (ether_info_packet_t *frame, uint16 ip_header_length, uint16 udp_data_length)
{
    udp_header_t   *udp;

    udp = (udp_header_t *) (frame->data + ip_header_length);

    /* STAGE: UDP header checksum. */

    if (udp->checksum)
    {
        uint16 checksum;

        checksum = udp_checksum ((ip_header_t *) frame->data, ip_header_length);

        /* STAGE: Fix the complementation "problem." */

        if (!checksum)
            checksum = 0xffff;

        if (checksum != udp->checksum)
            return FALSE;
    }

    /* STAGE: Handle UDP packets based on port. */

    switch (ntohs(udp->dest))
    {
        /* STAGE: UDP Echo. */

        case NET_UDP_PORT_ECHO :
            udp_echo_reply (frame, udp, udp_data_length);
            break;

        /* STAGE: VOOT protocol. */

        case NET_UDP_PORT_VOOT :
        {
            /* STAGE: This really shouldn't be here, but I can't think of a better location. */
            {
                biudp_control_t control;
                ip_header_t    *ip;

                ip = (ip_header_t *) frame->data;
                memcpy (control.dest_mac, frame->source, ETHER_MAC_SIZE);
                SAFE_UINT32_COPY(control.source_ip, ip->dest);
                SAFE_UINT32_COPY(control.dest_ip, ip->source);
                control.port = udp->src;

                biudp_init (&control);
            }

            return voot_handle_packet (frame, udp, udp_data_length);
        }

        default :
            break;
    }

    return FALSE;
}

/*
    Top level networking
*/

bool net_handle_frame (uint8 *frame_data, uint32 frame_size)
{
    ether_info_packet_t frame;

    frame = eth_discover_frame (frame_data, frame_size);
    switch (frame.ethertype)
    {
        /* STAGE: Handle TCP/IP type frames. */

        case 0x0800 :    /* NOTE: TCP/IP in the network byte order world. */
            return ip_handle_packet (&frame);

        default :
            return FALSE;
    }
}
