/*  heartbeat.c

    Essentially the core of any code I might write from here on. Every UBC
    exception (thus around 60 times a second - we're breaking on pageflip)
    we receive a call here.
*/

#include "vars.h"
#include "exception.h"
#include "system.h"
#include "heartbeat.h"
#include "serial.h"
#include "asic.h"

void init_heartbeat(void)
{
    uint32 rcode;
    exception_table_entry new;

    new.type = EXP_TYPE_GEN;
    new.code = 0x1e0;
    new.handler = heartbeat;

    rcode = add_exception_handler(new);

#ifdef DEBUG_HEARTBEAT
    if (!rcode)
        ubc_serial_write_str("[UBC] Unable to hook heartbeat handler.\r\n");
#endif
}

void* heartbeat(register_stack *stack, void *current_vector)
{
    return current_vector;
}
