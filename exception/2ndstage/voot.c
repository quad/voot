/*  voot.c

    VOOT netplay protocol debug implementation.

*/

#include "vars.h"
#include "net.h"
#include "serial.h"
#include "voot.h"

static void maybe_respond_command(uint8 maybe_command)
{
    volatile uint16 *player_a_health = (uint16 *) 0x8CCF6284;
    volatile uint16 *player_b_health = (uint16 *) 0x8CCF7402;

    switch (maybe_command)
    {
        case '1':
            ubc_serial_write_str("[UBC] Resetting player A health.\r\n");
            *player_a_health = 1200;
            break;

        case '2':
            ubc_serial_write_str("[UBC] Resetting player B health.\r\n");
            *player_b_health = 1200;
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
