/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "exception.h"
#include "trap.h"
#include "biudp.h"
#include "util.h"
#include "voot.h"

#include "rtl8139c.h"

#ifdef DEPRECATED_VOOT_NET

static void dump_framebuffer_udp(void)
{
    uint32 index;
    uint16 *vram_start;

    #define UPSCALE_5_STYLE(bits)   (((bits) << 3) | ((bits) >> 2))
    #define UPSCALE_6_STYLE(bits)   (((bits) << 2) | ((bits) >> 4))

    #define RED_565_TO_INT(color)   UPSCALE_5_STYLE((color) & 0x1F)
    #define GREEN_565_TO_INT(color) UPSCALE_6_STYLE(((color) >> 5) & 0x3F)
    #define BLUE_565_TO_INT(color)  UPSCALE_5_STYLE(((color) >> 11) & 0x1F)

    vram_start = (uint16 *) (0xa5000000 + *((volatile unsigned int *)0xa05f8050));    /* Buffer start ? */

    #define MAP_NUM_PIXELS  (640 * 480)
    #define STRIP_SIZE      300

    /* STAGE: Release the data back in sectioned strips. */
    for (index = 0; index <= MAP_NUM_PIXELS; index++)
    {
        uint8 strip[STRIP_SIZE][3];
                
        strip[index % STRIP_SIZE][0] = RED_565_TO_INT(vram_start[index]);
        strip[index % STRIP_SIZE][1] = GREEN_565_TO_INT(vram_start[index]);
        strip[index % STRIP_SIZE][2] = BLUE_565_TO_INT(vram_start[index]);

        if ((index % STRIP_SIZE) == STRIP_SIZE - 1)
            biudp_write_buffer((uint8 *) strip, sizeof(strip));
    }
}

#endif

static void maybe_handle_command(uint8 command, udp_header_t *udp, uint16 udp_data_length)
{
    switch(command)
    {
#ifdef DEPRECATED_VOOT_NET
        /* STAGE: Gamedata upload logic. */
        case '1':
            biudp_write_buffer((const uint8 *) VOOT_MEM_START, VOOT_MEM_END - VOOT_MEM_START);
            break;

        /* STAGE: Gamedata download logic. */
        case '2':
            biudp_write_str("[UBC] Uploading game data.\r\n");
            memcpy((uint8 *) VOOT_MEM_START, (uint8 *) udp + sizeof(udp_header_t) + 1, VOOT_MEM_END - VOOT_MEM_START);
            biudp_write_str("[UBC] Uploaded game data.\r\n");
            break;

        /* STAGE: Do we take a screenshot? */
        case 's':
            dump_framebuffer_udp();
            break;

        /* STAGE: Dump 16mb of system memory. */
        case 'S':
            biudp_write_buffer((const uint8 *) SYS_MEM_START, SYS_MEM_END - SYS_MEM_START);
            break;

#endif
        case 'i':
        {
            char test_string[] = "SuperJoe";
            trap_inject_data(test_string, sizeof(test_string));
        }
            break;

        case 'm':
        {
            uint8 *ba, *bb, *bc;

            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "malloc_fail_count == %u\n", malloc_fail_count);

            ba = malloc(NET_MAX_PACKET);
            bb = malloc(NET_MAX_PACKET);
            bc = malloc(NET_MAX_PACKET);

            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "malloc sequence %x, %x, and %x\n", ba, bb, bc);

            free(bc);
            free(bb);
            free(ba);

            ba = malloc(NET_MAX_PACKET);

            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "fresh malloc %x\n", ba);

            free(ba);
        }
            break;

        case 'n':
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "rtl_max_wait_count == %u\n", rtl_max_wait_count);
            break;

        case 'p':
        {
            volatile uint16 *p1_health = (uint16 *) 0x8CCF6284;
            volatile uint16 *p2_health = (uint16 *) 0x8CCF7402;

            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "p1_health = %u p2_health = %u\n", *p1_health, *p2_health);
        }
            break;

        case 't':
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "%u\n", time());
            break;

        case 'v':
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Netplay VOOT Extensions, BETA\n");
            break;

        default:
            break;
    }
}

static void maybe_handle_voot(voot_packet *packet, udp_header_t *udp, uint16 udp_data_length)
{
    /* STAGE: Fix the size byte order. */
    packet->size = ntohs(packet->size);

    switch (packet->type)
    {
        case VOOT_PACKET_TYPE_COMMAND:
            maybe_handle_command(packet->buffer[0], udp, udp_data_length);
            break;

        case VOOT_PACKET_TYPE_DATA:
            trap_inject_data(packet->buffer, packet->size);
            break;

        default:
            break;
    }
}

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    voot_packet *packet;

#ifndef HARDCODE_IP
    /* STAGE: Use the information from the UDP packet to fill out our biudp
        information. */
    {
        biudp_control_t control;
        ip_header_t *ip;

        ip = (ip_header_t *) frame->data;
        memcpy(control.dest_mac, frame->source, ETHER_MAC_SIZE);
        IP_ADDR_COPY(control.source_ip, ip->dest);
        IP_ADDR_COPY(control.dest_ip, ip->source);
        control.port = udp->src;

        biudp_init(&control);
    }
#endif

    packet = (voot_packet *) ((uint8 *) udp + sizeof(udp_header_t));
    maybe_handle_voot(packet, udp, udp_data_length);
}
