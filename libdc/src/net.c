/*  net.c

DESCRIPTION
    Networking sub-system. I intend on handling the following protocols:

    * UDP           - Our primary data transfer protocol.
        * DHCP      - We want to be able to resolve our IP address in
                       foreign environments.
        * VOOT      - The Netplay VOOT protocol.
    * ICMP          - Pings, primarily.

COPYING
    See "COPYING" in the root directory of the distribution.

CHANGELOG
    Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Imported, modified, and just generally added a timestamp when I
        created the libdc distribution.

*/

#include "vars.h"
#include "rtl8139c.h"
#include "bswap.h"
#include "net.h"

static void maybe_respond_command(icmp_header_t *icmp, uint16 icmp_data_length)
{
    uint8 *maybe_command;
    volatile uint16 *player_a_health = (uint16 *) 0x8CCF6284;
    volatile uint16 *player_b_health = (uint16 *) 0x8CCF7402;

    maybe_command = (uint8 *) icmp + sizeof(icmp_header_t) + 10;

    switch (*maybe_command)
    {
        case 1:
            *player_a_health = 1200;
            break;

        case 2:
            *player_b_health = 1200;
            break;

        default:
            break;
    }

}

/*
 *
 *  Ethernet Subsystem
 *
 */

static void net_transmit(ether_info_packet_t *frame_in)
{
    ether_ii_header_t *frame_out;

    /* If it was originally a II header, we're covered. If it was originally
        a 802.3 header, we're left with space to spare. :-) */
    frame_out = (ether_ii_header_t *) rtl_info.frame_out_buffer;

    /* STAGE: Setup the packet. */
    memcpy(frame_out->source, frame_in->source, ETHER_MAC_SIZE);
    memcpy(frame_out->dest, frame_in->dest, ETHER_MAC_SIZE);
    frame_out->ethertype = htons(frame_in->ethertype);

    /* STAGE: Copy the remaining buffer. */
    memcpy(frame_out_buffer + sizeof(ether_ii_header_t), frame_in->data, frame_in->length);

    /* STAGE: Bye bye Mr. Packet */
    rtl_tx(frame_out_buffer, frame_in->length + sizeof(ether_ii_header_t));
}

static ether_info_packet_t eth_discover_frame(uint8 *frame_data, uint32 frame_size)
{
    ether_8023_header_t *frame_8023;
    ether_ii_header_t *frame_ii;
    ether_info_packet_t frame;
    uint16 maybe_ethertype;

    frame_8023 = (ether_8023_header_t *) frame_data;
    frame_ii = (ether_ii_header_t *) frame_data;

    maybe_ethertype = ntohs(frame_ii->ethertype);

    if (maybe_ethertype >= 0x0600)
    /* It's an Ethernet II packet. */
    {
        memcpy(frame.dest, frame_ii->dest, sizeof(frame.dest));
        memcpy(frame.source, frame_ii->source, sizeof(frame.source));

        frame.ethertype = maybe_ethertype;

        frame.length = frame_size - sizeof(ether_ii_header_t);
        frame.data = frame_data + sizeof(ether_ii_header_t);
    }
    else if (frame_8023->dsap == 0xAA)
    /* 802.3 SNAP provides backwards compatibility to Ethernet II. */
    {
        memcpy(frame.dest, frame_8023->dest, sizeof(frame.dest));
        memcpy(frame.source, frame_8023->source, sizeof(frame.source));

        frame.ethertype = ntohs(frame_8023->ethertype);

        frame.length = ntohs(frame_8023->length);
        frame.data = frame_data + sizeof(ether_8023_header_t);
    }
    else
    /* We couldn't determine the frame type. */
    {
        frame.ethertype = 0x0;
    }

    /* Couldn't find an ethertype, thus it can't possibly have IP in it. */
    return frame;
}

/*
 *
 *  IP Processor
 *
 *  With all thanks to AndrewK.
 *
 *  Is this feeling like a cleaner re-implementation of dc-load-ip to anyone
 *  else?
 */

static uint16 ip_checksum(uint16 *buf, uint16 count_short)
{
    uint32 sum;

    sum = 0;

    while (count_short--)
    {
	    sum += *buf++;

	    if (sum & 0xffff0000)
	    {
	        sum &= 0xffff;
    	    sum++;
    	}
    }

    return ~(sum & 0xffff);
}

void ip_handle_packet(ether_info_packet_t *frame)
{
    ip_header_t *ip;
    uint16 ip_data_length;
    uint16 ip_header_length;
    uint16 temp_checksum;

    ip = (ip_header_t *) frame->data;

    /* STAGE: Check if the packet is fragmented. We don't support it - yet. */
    if (ntohs(ip->flags_frag_offset) & 0x3fff)
        return;

    ip_header_length = IP_HEADER_SIZE(ip);
    ip_data_length = ntohs(ip->length) - ip_header_length;

    /* STAGE: IP Header checksum */
    temp_checksum = ip->checksum;
    ip->checksum = 0;
    ip->checksum = ip_checksum((uint16 *) ip, ip_header_length/2);
    if (temp_checksum != ip->checksum)
        return;

    /* STAGE: And handle the appropriate protocol type! */
    switch (ip->protocol)
    {
        case IP_PROTO_ICMP:
            icmp_handle_packet(frame, ip_header_length, ip_data_length);
            break;

        case IP_PROTO_UDP:
        //case IP_PROTO_TCP:
         default:    /* Yeah, we don't support this. */
            return;
    }
}

/*
 *
 *  UDP Sub-System
 *
 */

void udp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 udp_data_length)
{
    udp_header_t *udp;
    uint16 temp_checksum;

    udp = (udp_header_t *) (frame->data + ip_header_length);

    /* STAGE: UDP header checksum */
    temp_checksum = udp->checksum;
    udp->checksum = 0;
    udp->checksum = ip_checksum((uint16 *) udp, udp_data_length/2);
    if (temp_checksum != udp->checksum)
        return;
}

/*
 *
 *  ICMP Sub-System
 *
 */

static void icmp_echo_reply(ether_info_packet_t *frame, icmp_header_t *icmp, uint16 icmp_data_length)
{
    char temp_mac[ETHER_MAC_SIZE];
    uint32 temp_ipaddr;
    ip_header_t *ip;

    ip = (ip_header_t *) frame->data;

    /* STAGE: We want to reply to a ping. */
    icmp->type = ICMP_TYPE_ECHO_REPLY;

    /* STAGE: Ether - point to our origiantor. */
    memcpy(temp_mac, frame->source, ETHER_MAC_SIZE);
    memcpy(frame->source, frame->dest, ETHER_MAC_SIZE);
    memcpy(frame->dest, temp_mac, ETHER_MAC_SIZE);

    /* STAGE: IP - point to our originator. */
    memcpy(&temp_ipaddr, &ip->source, sizeof(temp_ipaddr));
    memcpy(&ip->source, &ip->dest, sizeof(temp_ipaddr));
    memcpy(&ip->dest, &temp_ipaddr, sizeof(temp_ipaddr));

    /* STAGE: Compute IP checksum. */
    ip->checksum = 0;
    ip->checksum = ip_checksum((uint16 *) ip, IP_HEADER_SIZE(ip)/2);

    /* STAGE: Compute ICMP checksum. */
    icmp->checksum = 0;
    icmp->checksum = ip_checksum((uint16 *) icmp, icmp_data_length/2);

    /* STAGE: Transmit it, god willing. */
    net_transmit(frame);
}

void icmp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 icmp_data_length)
{
    icmp_header_t *icmp;
    uint16 temp_checksum;

    icmp = (icmp_header_t *) (frame->data + ip_header_length);

    /* STAGE: ICMP header checksum */
    temp_checksum = icmp->checksum;
    icmp->checksum = 0;
    icmp->checksum = ip_checksum((uint16 *) icmp, icmp_data_length/2);
    if (temp_checksum != icmp->checksum)
        return;

    /* STAGE: Handle ICMP packet types */
    switch (icmp->type)
    {
        /* Replay to these straight back with its own packet! Woo ha ha! */
        case ICMP_TYPE_ECHO_REQUEST:
            /* Check to see if we can respond to the data type - this can be
                a cheap communications method. */
            maybe_respond_command(icmp, icmp_data_length);
            icmp_echo_reply(frame, icmp, icmp_data_length);
            break;

        default:    /* Whatever it is, we don't support it yet. */
            break;
    }
}

/*
 *
 *  Top level networking
 *
 */

void net_handle_frame(uint8 *frame_data, uint32 frame_size)
{
    ether_info_packet_t frame;

    frame = eth_discover_frame (frame_data, frame_size);
    if (!frame.ethertype)
#ifdef DEBUG_NET
        ubc_serial_write_str("[UBC] Received malformed frame.\r\n");
#else
        return;
#endif
    else if(frame.ethertype == 0x0800)  /* TCP/IP */
    {
        ip_handle_packet(&frame);
    }       
}
