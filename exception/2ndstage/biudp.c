/*  biudp.c

    Bi-directional UDP communications code.
*/

#include "vars.h"
#include "voot.h"
#include "net.h"
#include "rtl8139c.h"
#include "util.h"
#include <stdarg.h>
#include "printf.h"
#include "biudp.h"

biudp_control_t control;
voot_packet netout;

void biudp_init(const biudp_control_t *in_control)
{
    /* STAGE: Copy the input control into global control structure. */
    memcpy(&control, in_control, sizeof(biudp_control_t));

    control.initialized = 1;

    /* TODO (!!!): I really need to write some sort of function which will
        search the 192.? reserved IP area for an open IP. Use a ping scan or
        something. */
}

/* This entire section is essentially a duplicate of the net_transmit code,
    except its formatted differently. I wish I could combine them somehow. */
static void biudp_write_segment(const uint8 *in_data, uint32 in_data_length)
{
    ether_ii_header_t *frame_out;
    ip_header_t *ip;
    udp_header_t *udp;
    uint16 total_length, ip_length, ip_header_length;

    /* STAGE: Use Ethernet II. Everyone MUST support it. */
    ip_length = sizeof(ip_header_t) + sizeof(udp_header_t) + in_data_length;
    total_length = sizeof(ether_ii_header_t) + ip_length;

    /* STAGE: malloc() the proper size output buffer. */
    frame_out = malloc(total_length);
    if (!frame_out)     /* !!! I need some OOB method of warning here. */
        return;

    ip = (ip_header_t *) ((uint8 *) frame_out + sizeof(ether_ii_header_t)); 

    /* STAGE: Setup the frame layer. */
    memcpy(frame_out->source, rtl_info.mac, ETHER_MAC_SIZE);
    memcpy(frame_out->dest, control.dest_mac, ETHER_MAC_SIZE);
    frame_out->ethertype = htons(0x0800);

    /* STAGE: Setup the IP layer. */
    ip->version_ihl = 0x45;     /* 4 is the IP version, 5 is the size of the header in 32-bit segments. No options and no padding. */
    ip->tos = 0x14;     /* Normal precedence, low delay, high reliability - this is all we want but we'll never get. */
    ip->length = htons(ip_length);
    ip->packet_id = 0;  /* TODO (!!!): Add some sort of ID system later. */
    ip->flags_frag_offset = htons(0x4000);  /* One order of IP packet, hold the fragmentation. */
    ip->ttl = 0x20;     /* 32 seems like plenty - it gets me to Kirk. */
    ip->protocol = IP_PROTO_UDP;
    ip->checksum = 0;

    IP_ADDR_COPY(ip->source, control.source_ip);
    IP_ADDR_COPY(ip->dest, control.dest_ip);

    /* STAGE: Calculate the IP checksum last. */
    ip_header_length = IP_HEADER_SIZE(ip);
    ip->checksum = ip_checksum(ip, ip_header_length);

    /* STAGE: Identify the UDP layer. */
    udp = (udp_header_t *) ((uint8 *) ip + ip_header_length);

    /* STAGE: Setup the UDP layer. */
    udp->src = htons(VOOT_UDP_PORT);
    udp->dest = control.port;
    udp->length = htons(sizeof(udp_header_t) + in_data_length);

    /* STAGE: So, how about that data? */
    memcpy((uint8 *) udp + sizeof(udp_header_t), in_data, in_data_length);

    /* STAGE: Calculate the UDP checksum last. */
    udp->checksum = udp_checksum(ip, ip_header_length);

    /* STAGE: ... and transmit it, god willing. */
    rtl_tx((uint8 *) frame_out, sizeof(ether_ii_header_t) + ip_length);

    /* STAGE: Free the output buffer. */
    free(frame_out);
}

void biudp_write_buffer(const uint8 *in_data, uint32 in_data_length)
{
    uint32 index, remain;

    if (!control.initialized)
        return;

    /* STAGE: Split the incoming into BIUDP_SEGMENT_SIZE byte chunks and
        feed those out. */
    for (index = 0; index < (in_data_length / BIUDP_SEGMENT_SIZE); index++)
    {
        const uint8 *in_data_segment;

        in_data_segment = in_data + (BIUDP_SEGMENT_SIZE * index);

        biudp_write_segment(in_data_segment, BIUDP_SEGMENT_SIZE);

        /* STAGE: Delay so we don't flood the receiving system. */
        vid_waitvbl();
    }

    remain = in_data_length % BIUDP_SEGMENT_SIZE;
    if (remain)
        biudp_write_segment((in_data + in_data_length) - remain, remain);
}

void biudp_write(const uint8 in)
{
    biudp_write_buffer(&in, sizeof(uint8));
}

void biudp_write_str(const uint8 *in_string)
{
    biudp_write_buffer(in_string, strlen(in_string));
}

int32 biudp_printf(uint8 type, const char *fmt, ...)
{
	va_list args;
	int32 i;

    netout.type = type;

	va_start(args, fmt);
	i = vsnprintf(netout.buffer, sizeof(netout.buffer), fmt, args);
	va_end(args);

    netout.size = htons(i);

    biudp_write_buffer((uint8 *) &netout, VOOT_PACKET_HEADER_SIZE + i);

	return i;
}
