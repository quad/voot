/*  module.c

    $Id: module.c,v 1.1 2002/06/12 00:40:01 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <rtl8139c.h>
#include <voot.h>
#include <heartbeat.h>
#include <vmu.h>
#include <controller.h>

#include "module.h"

void module_initialize (void)
{
    /* NOTE: We don't need to initialize anything special. */
}

void* module_heartbeat (register_stack *stack, void *current_vector)
{
    uint8 *debug_566 = (uint8 *) 0x8ccf9f68;
    vmu_port *data_port = (vmu_port *) (0x8ccf9f06);

    *debug_566 = 0x01;

    if (check_controller_press(*data_port) & CONTROLLER_MASK_BUTTON_Y)
    {
        uint8 *vrcust_action = (uint8 *) 0x8c275224;

        *vrcust_action = 0x1a;
    }

    return current_vector;
}

void module_configure (void)
{
    heartbeat_init ();
    controller_init ();
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
