/*  net.c

    Networking sub-system. I intend on handling the following protocols:


    * UDP           - Our primary data transfer protocol.
        * DHCP      - We want to be able to resolve our IP address in
                       foreign environments.
        * VOOT      - The Netplay VOOT protocol.
    * ICMP          - Pings, primarily.
*/

#include "vars.h"
#include "rtl8139c.h"
#include "serial.h"
#include "net.h"
#include "bswap.h"

static void maybe_respond_command(uint32 maybe_command)
{
    volatile uint16 *player_a_health = (uint16 *) 0x8CCF6284;
    volatile uint16 *player_b_health = (uint16 *) 0x8CCF7402;

    switch (maybe_command)
    {
        case 1:
            ubc_serial_write_str("[UBC] Resetting player A health.\r\n");
            *player_a_health = 1200;
            break;

        case 2:
            ubc_serial_write_str("[UBC] Resetting player B health.\r\n");
            *player_b_health = 1200;
            break;

        default:
            ubc_serial_write_str("[UBC] command = 0x");
            ubc_serial_write_hex(maybe_command);
            ubc_serial_write_str("\r\n");
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
    memcpy((uint8 *) frame_out + sizeof(ether_ii_header_t), frame_in->data, frame_in->length);

    /* STAGE: Bye bye Mr. Packet */
    rtl_tx((uint8 *) frame_out, frame_in->length + sizeof(ether_ii_header_t));
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

    /* STAGE: Determine Ethernet frame type. */
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
 *  Is this feeling like a cleaner re-implementation of dc-load-ip. Anyone
 *  else?
 */

static uint16 ip_checksum_add(uint16 *buf, uint16 count_short, uint32 sum)
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

uint16 udp_checksum(ip_header_t *ip, uint16 ip_header_length)
{
    udp_header_t *udp;
    uint16 udp_length;
    udp_pseudo_header_t udp_top;
    uint16 temp_checksum, calc_checksum;

    /* STAGE: Identify the UDP header. */
    udp = (udp_header_t *) ((uint8 *) ip + ip_header_length);
    udp_length = ntohs(udp->length);

    /* STAGE: Back and clear the incoming checksum value. */
    temp_checksum = udp->checksum;
    udp->checksum = 0;

    /* STAGE: Construct the UDP psuedo-header. */
    udp_top.source_ip = ip->source;
    udp_top.dest_ip = ip->dest;
    udp_top.zero = 0;
    udp_top.protocol = ip->protocol;
    udp_top.length = udp->length;

    /* STAGE: Calculate the first half of the checksum using the UDP pseudo-header. */
    calc_checksum = ip_checksum_add((uint16 *) (&udp_top), sizeof(udp_top) / sizeof(uint16), 0);

    /* STAGE: Calculate the second half of the checksum using the actual UDP packet. */
    if (udp_length % 2)     /* Pad if we're not 16-bit aligned. */
    {
        ((uint8 *) udp)[udp_length] = 0;
        udp_length++;
    }
    calc_checksum = ip_checksum_add((uint16 *) udp, udp_length / sizeof(uint16), calc_checksum);

    /* STAGE: Restore the incoming checksum value. */
    udp->checksum = temp_checksum;

    return ~(calc_checksum & 0xffff);
}

uint16 icmp_checksum(icmp_header_t *icmp, uint16 icmp_header_length)
{
    uint16 temp_checksum, calc_checksum;

    /* STAGE: Back and clear the incoming checksum value. */
    temp_checksum = icmp->checksum;
    icmp->checksum = 0;

    /* STAGE: Calculate the checksum. */
    calc_checksum = ~(ip_checksum_add((uint16 *) icmp, icmp_header_length / 2, 0) & 0xffff);

    /* STAGE: Restore the original incoming checksum value. */
    icmp->checksum = temp_checksum;

    /* STAGE: Return our calculated value. */
    return calc_checksum;
}

uint16 ip_checksum(ip_header_t *ip, uint16 ip_header_length)
{
    uint16 temp_checksum, calc_checksum;

    /* STAGE: Back and clear the incoming checksum value. */
    temp_checksum = ip->checksum;
    ip->checksum = 0;

    /* STAGE: Calculate the checksum. */
    calc_checksum = ~(ip_checksum_add((uint16 *) ip, ip_header_length / 2, 0) & 0xffff);

    /* STAGE: Restore the original incoming checksum value. */
    ip->checksum = temp_checksum;

    /* STAGE: Return our calculated value. */
    return calc_checksum;
}

void ip_handle_packet(ether_info_packet_t *frame)
{
    ip_header_t *ip;
    uint16 ip_data_length;
    uint16 ip_header_length;

    ip = (ip_header_t *) frame->data;

    /* STAGE: Check if the packet is fragmented. We don't support it - yet. */
    if (ntohs(ip->flags_frag_offset) & 0x3fff)
        return;

    /* STAGE: Pre-calculate IP header sizes. */
    ip_header_length = IP_HEADER_SIZE(ip);
    ip_data_length = ntohs(ip->length) - ip_header_length;

    /* STAGE: IP Header checksum */
    if (ip->checksum != ip_checksum(ip, ip_header_length))
    {
        ubc_serial_write_str("[UBC] Bad IP checksum.\r\n");
        return;
    }

    /* STAGE: And handle the appropriate protocol type! */
    switch (ip->protocol)
    {
        case IP_PROTO_ICMP:
            icmp_handle_packet(frame, ip_header_length, ip_data_length);
            break;

        case IP_PROTO_UDP:
            udp_handle_packet(frame, ip_header_length, ip_data_length);
            break;

        //case IP_PROTO_TCP:
        default:    /* Yeah, we don't support this. */
            ubc_serial_write_str("[UBC] Unfamiliar packet type.\r\n");
            return;
    }
}

/*
 *
 *  UDP Sub-System
 *
 */

static void udp_echo_reply(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    char temp_mac[ETHER_MAC_SIZE];
    uint32 temp_ipaddr;
    uint16 temp_port;
    uint16 ip_header_size;
    ip_header_t *ip;

    ip = (ip_header_t *) frame->data;
    ip_header_size = IP_HEADER_SIZE(ip);

    /* STAGE: Ether - point to our origiantor. */
    memcpy(temp_mac, frame->source, ETHER_MAC_SIZE);
    memcpy(frame->source, frame->dest, ETHER_MAC_SIZE);
    memcpy(frame->dest, temp_mac, ETHER_MAC_SIZE);

    /* STAGE: IP - point to our originator. */
    temp_ipaddr = ip->source;
    ip->source = ip->dest;
    ip->dest = temp_ipaddr;

    /* STAGE: Compute IP checksum. */
    ip->checksum = ip_checksum(ip, ip_header_size);

    /* STAGE: UDP - we want to reply from our echo port to their port. */
    temp_port = udp->src;
    udp->src = udp->dest;
    udp->dest = temp_port;

    /* STAGE: Compute UDP checksum. */
    udp->checksum = udp_checksum(ip, ip_header_size);

    /* STAGE: Transmit it, god willing. */
    net_transmit(frame);

}

void udp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 udp_data_length)
{
    udp_header_t *udp;

    udp = (udp_header_t *) (frame->data + ip_header_length);

    /* STAGE: UDP header checksum */
    if (udp->checksum)
    {
        uint16 checksum;

        checksum = udp_checksum((ip_header_t *) frame->data, ip_header_length);

        if (!checksum)  /* Fix the complementation "problem" */
            checksum = 0xffff;

        if (checksum != udp->checksum)
        {
            ubc_serial_write_str("[UBC] Bad UDP checksum.\r\n");
            return;
        }
    }
    else
        ubc_serial_write_str("[UBC] UDP packet missing checksum.\r\n");

    /* STAGE: Handle UDP packets based on port. */
    switch(ntohs(udp->dest))
    {
        case 7:     /* UDP Echo */
            udp_echo_reply(frame, udp, udp_data_length);
            break;

        case 5007:  /* now the official VOOT network protocol port. */
        default:    /* Drop on the floor any network data we couldn't possibly understand. */
            break;
    }
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
    temp_ipaddr = ip->source;
    ip->source = ip->dest;
    ip->dest = temp_ipaddr;

    /* STAGE: Compute IP checksum. */
    ip->checksum = ip_checksum(ip, IP_HEADER_SIZE(ip));

    /* STAGE: Compute ICMP checksum. */
    icmp->checksum = icmp_checksum(icmp, icmp_data_length);

    /* STAGE: Transmit it, god willing. */
    net_transmit(frame);
}

void icmp_handle_packet(ether_info_packet_t *frame, uint16 ip_header_length, uint16 icmp_data_length)
{
    icmp_header_t *icmp;

    icmp = (icmp_header_t *) (frame->data + ip_header_length);

    /* STAGE: ICMP header checksum */
    if (icmp->checksum != icmp_checksum(icmp, icmp_data_length))
    {
        ubc_serial_write_str("[UBC] Bad ICMP checksum.\r\n");
        return;
    }

    /* STAGE: Handle ICMP packet types */
    switch (icmp->type)
    {
        /* Replay to these straight back with its own packet! Woo ha ha! */
        case ICMP_TYPE_ECHO_REQUEST:
            /* Check to see if we can respond to the data type - this can be
                a cheap communications method. */
            maybe_respond_command(*((uint32 *) icmp + sizeof(icmp_header_t)));
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
    switch (frame.ethertype)
    {
        /* STAGE: Handle TCP/IP type frames. */
        case 0x0800:    /* = TCP/IP in the network byte order world. */
            ip_handle_packet(&frame);
            break;

        default:        /* Ignore unknown ethertypes. */
            break;
    }
}
