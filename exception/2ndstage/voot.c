/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "net.h"
#include "serial.h"
#include "biudp.h"
#include "voot.h"

static void maybe_respond_command(uint8 maybe_command)
{
    volatile uint16 *player_a_health = (uint16 *) 0x8CCF6284;
    volatile uint16 *player_b_health = (uint16 *) 0x8CCF7402;

    switch (maybe_command)
    {
        case '1':
            biudp_write_str("[UBC] Resetting player A health.\r\n");
            *player_a_health = 1200;
            break;

        case '2':
            biudp_write_str("[UBC] Resetting player B health.\r\n");
            *player_b_health = 1200;
            break;

        case '3':
        {
            uint32 index;
            uint16 *vram_start;

            #define UPSCALE_5_STYLE(bits)   (((bits) << 3) | ((bits) >> 2))
            #define UPSCALE_6_STYLE(bits)   (((bits) << 2) | ((bits) >> 4))

            #define RED_565_TO_INT(color)   UPSCALE_5_STYLE((color) & 0x1F)
            #define GREEN_565_TO_INT(color) UPSCALE_6_STYLE(((color) >> 5) & 0x3F)
            #define BLUE_565_TO_INT(color)  UPSCALE_5_STYLE(((color) >> 11) & 0x1F)

            biudp_write_str("[UBC] Dumping screenshot.\r\n#");
            vram_start = (uint16 *) (0xa5000000 + *((volatile unsigned int *)0xa05f8050));    /* Buffer start ? */

            #define MAP_NUM_PIXELS  (640 * 480)

            for (index = 0; index <= MAP_NUM_PIXELS; index++)
            {
                uint8 strip[300][3];
                
                strip[index % 300][0] = RED_565_TO_INT(vram_start[index]);
                strip[index % 300][1] = GREEN_565_TO_INT(vram_start[index]);
                strip[index % 300][2] = BLUE_565_TO_INT(vram_start[index]);

                if ((index % 300) == 299)
                    biudp_write_buffer((uint8 *) strip, sizeof(strip));
            }
            biudp_write_str("#\r\n[UBC] Done with screenshot.\r\n");
        }
            break;

        default:
            ubc_serial_write_str("[UBC] command = '");
            ubc_serial_write(maybe_command);
            ubc_serial_write_str("'\r\n");
            break;
    }
}

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length)
{
    uint8 *command;

    command = (uint8 *) udp + sizeof(udp_header_t);

    maybe_respond_command(*command);
}
