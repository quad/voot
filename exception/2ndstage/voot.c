/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "trap.h"
#include "rtl8139c.h"
#include "util.h"
#include "printf.h"

#include "voot.h"

static void dump_framebuffer(void)
{
    uint32 index;
    uint16 *vram_start;

    voot_send_command(VOOT_COMMAND_TYPE_DUMPON);

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
            voot_send_packet(VOOT_PACKET_TYPE_DUMP, (uint8 *) strip, sizeof(strip));
    }

    voot_send_command(VOOT_COMMAND_TYPE_DUMPOFF);
}

static bool maybe_handle_command(uint8 command)
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


        /* STAGE: Dump 16mb of system memory. */
        case 'S':
            biudp_write_buffer((const uint8 *) SYS_MEM_START, SYS_MEM_END - SYS_MEM_START);
            break;

#endif

        case VOOT_COMMAND_TYPE_SCREEN:
        {
            dump_framebuffer();
            break;
        }

        case VOOT_COMMAND_TYPE_INJECTTST:
        {
            char test_string[] = "SuperJoe";
            trap_inject_data(test_string, sizeof(test_string));
        }
            break;

        case VOOT_COMMAND_TYPE_PRINTFTST:
        {
            char test_string[] = "SuperJoe";
            int32 i;

            i = voot_printf(VOOT_PACKET_TYPE_DEBUG, "%s", test_string);
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "i = %d", i);
            break;
        }

        case VOOT_COMMAND_TYPE_MALLOCTST:
        {
            uint8 *ba, *bb, *bc;

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "malloc_fail_count == %u", malloc_fail_count);

            ba = malloc(NET_MAX_PACKET);
            bb = malloc(NET_MAX_PACKET);
            bc = malloc(NET_MAX_PACKET);

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "malloc sequence %x, %x, and %x", ba, bb, bc);

            free(bc);
            free(bb);
            free(ba);

            ba = malloc(NET_MAX_PACKET);

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "fresh malloc %x", ba);

            free(ba);
        }
            break;

        case VOOT_COMMAND_TYPE_NETSTAT:
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "rtl_max_wait_count == %u", rtl_max_wait_count);
            break;

        case VOOT_COMMAND_TYPE_HEALTH:
        {
            volatile uint16 *p1_health = (uint16 *) 0x8CCF6284;
            volatile uint16 *p2_health = (uint16 *) 0x8CCF7402;

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "p1_health = %u p2_health = %u", *p1_health, *p2_health);
        }
            break;

        case VOOT_COMMAND_TYPE_TIME:
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "%u", time());
            break;

        case VOOT_COMMAND_TYPE_PASVON:
            trap_set_passive(TRUE);
            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Passive monitoring ON!");
            break;

        case VOOT_COMMAND_TYPE_VERSION:
        {


            voot_printf(VOOT_PACKET_TYPE_DEBUG, "Netplay VOOT Extensions, BETA");
            break;
        }

        default:
            break;
    }

    return FALSE;
}

static bool maybe_handle_voot(voot_packet *packet, udp_header_t *udp, uint16 udp_data_length)
{
    /* STAGE: Fix the size byte order. */
    packet->header.size = ntohs(packet->header.size);

    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND:
            return maybe_handle_command(packet->buffer[0]);

        case VOOT_PACKET_TYPE_DATA:
            trap_inject_data(packet->buffer, packet->header.size - 1);
            break;

        default:
            break;
    }

    return FALSE;
}

bool voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    voot_packet *packet;

    packet = (voot_packet *) ((uint8 *) udp + sizeof(udp_header_t));
    return maybe_handle_voot(packet, udp, udp_data_length);
}

bool voot_send_packet(uint8 type, const uint8 *data, uint32 data_size)
{
	voot_packet *netout;

    /* STAGE: Make sure the dimensions are legit and we have enough space
        for data_size + NULL */
    if ((data_size >= sizeof(netout->buffer)) || !data_size)
        return FALSE;

    /* STAGE: Malloc the full-sized voot_packet. */
    netout = malloc(sizeof(voot_packet));
    if (!netout)
        return FALSE;   /* We didn't send any data. */

    /* STAGE: Set the packet header information, including the NULL */
    netout->header.type = type;
    netout->header.size = htons(data_size + 1);

    /* STAGE: Copy over the input buffer data and append NULL. */
    memcpy(netout->buffer, data, data_size);
    netout->buffer[data_size] = 0x0;

    /* STAGE: Transmit the packet. */
    biudp_write_buffer((const uint8 *) netout, VOOT_PACKET_HEADER_SIZE + data_size + 1);

    /* STAGE: Free the buffer and return. */
    free(netout);

    return TRUE;
}

bool voot_send_command(uint8 type)
{
    return voot_printf(VOOT_PACKET_TYPE_COMMAND, "%c", type);
}

int32 voot_printf(uint8 type, const char *fmt, ...)
{
	va_list args;
	int32 i;
	voot_packet *packet_size_check;
	char *printf_buffer;

    printf_buffer = malloc(sizeof(packet_size_check->buffer));
    if(!printf_buffer)
        return 0;

	va_start(args, fmt);
	i = vsnprintf(printf_buffer, sizeof(packet_size_check->buffer), fmt, args);
	va_end(args);

    voot_send_packet(type, printf_buffer, i);

    free(printf_buffer);

	return i;
}
