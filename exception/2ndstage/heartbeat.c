/*  heartbeat.c

    Essentially the core of any code I might write from here on. Every UBC
    exception (thus around 60 times a second - we're breaking on pageflip)
    we receive a call here.
*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "system.h"
#include "asic.h"
#include "biudp.h"
#include "heartbeat.h"

#ifdef DEBUG
    #include "serial.h"
#endif

void init_heartbeat(void)
{
    uint32 rcode;
    exception_table_entry new;

    new.type = EXP_TYPE_GEN;
    new.code = 0x1e0;
    new.handler = heartbeat;

    rcode = add_exception_handler(new);

#ifdef DEBUG
    /* There needs to be a better method of notification. */
    if (!rcode)
        ubc_serial_write_str("[UBC] Unable to hook heartbeat handler.\r\n");
#endif
}

void* heartbeat(register_stack *stack, void *current_vector)
{
/*
    volatile uint16 *proto_on = (volatile uint16 *) (0x8CCF9ECC + 0x6A);
*/
    volatile uint8 *enemy_shoot = (volatile uint8 *) (0x8CCF9ECC + 0x31);
    volatile uint16 *arcade = (volatile uint16 *) (0x8CCF9ECC + 0x1A);
    volatile uint16 *proto_ok = (volatile uint16 *) (0x8CCF9ECC + 0x86);
    volatile uint16 *menus = (volatile uint16 *) (0x8CCF9ECC + 0x8C);
    static bool done_once = FALSE;

    if (!done_once)
    {
        /* STAGE: !!! Check the timer chip. See if VOOT is using it. */

        done_once = TRUE;
    }

    *enemy_shoot = 0x01;
    *arcade = 0xFFFF;
    *proto_ok = 0x0101;
    *menus = 0x0101;

    return current_vector;
}
