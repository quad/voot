/*  net.c

    $Id: net.c,v 1.8 2002/06/24 06:11:15 quad Exp $

DESCRIPTION

    IP, ICMP, and basic UDP services layer.

TODO

    Implement ARP and DHCP.

*/

#include "vars.h"
#include "ether.h"
#include "util.h"
#include "malloc.h"
#include "bswap.h"
#include "biudp.h"
#include "voot.h"

#include "net.h"

/*
    NOTE: IP LAYER processor.

    CREDIT: With all thanks to AndrewK and "Internetworking with TCP/IP: Volume I".

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

static bool ip_packet_ok (ether_info_packet_t *frame, uint16 ip_header_length, uint16 ip_data_length)
{
    ip_header_t *ip;

    ip = (ip_header_t *) frame->data;

    /* STAGE: Check IP header version. */

    if ((ip->version_ihl & 0xf0) != 0x40)
        return FALSE;

    /* TODO: Check if length matches frame size. */

    /* STAGE: IP Header checksum */

    if (ip->checksum != ip_checksum (ip, ip_header_length))
        return FALSE;

    /* STAGE: Check if the packet is fragmented. We don't support it - yet. */

    if (ntohs (ip->flags_frag_offset) & 0x3fff)
        return FALSE;

    return TRUE;
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

    if (!ip_packet_ok (frame, ip_header_length, ip_data_length))
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

/*
    NOTE: ICMP LAYER processor.
*/

static uint16 icmp_checksum (icmp_header_t *icmp, uint16 icmp_header_length)
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

    return ether_transmit (frame);
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
    NOTE: UDP LAYER processor.
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

    return ether_transmit (frame);
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

        case UDP_PORT_ECHO :
            udp_echo_reply (frame, udp, udp_data_length);
            break;

        /* STAGE: VOOT protocol. */

        case UDP_PORT_VOOT :
        {
            /* STAGE / TODO: This really shouldn't be here, but I can't think of a better location. */
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
