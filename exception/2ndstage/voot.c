/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "biudp.h"
#include "exception.h"
#include "search.h"
#include "voot.h"

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

static void maybe_respond_command(uint8 maybe_command, udp_header_t *udp, uint16 udp_data_length)
{
/*
    volatile uint16 *player_a_health = (uint16 *) 0x8CCF6284;
    volatile uint16 *player_b_health = (uint16 *) 0x8CCF7402;
*/

    switch (maybe_command)
    {
        /* STAGE: Dump VOOT system memory. */
        case '1':
            biudp_write_buffer((const uint8 *) VOOT_MEM_START, VOOT_MEM_END - VOOT_MEM_START);
            break;

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

        case 'v':
            biudp_write_str("[UBC] Netplay VOOT Extensions, BETA - compiled at " __TIME__ " on " __DATE__ "\r\n");
            break;

        default:
            break;
    }
}

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    uint8 *command;

    /* STAGE: Use the information from the ICMP echo to fill out our
        biudp information. */
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

    command = (uint8 *) udp + sizeof(udp_header_t);
    maybe_respond_command(*command, udp, udp_data_length);
}
