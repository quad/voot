/*  module.c

    $Id: module.c,v 1.3 2002/06/29 13:10:46 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.

*/

#include <vars.h>
#include <vmu.h>
#include <controller.h>
#include "customize.h"

#include <ether.h>
#include <dumpio.h>

#include "module.h"

void module_initialize (void)
{
    /* NOTE: Don't need to initialize anything further. */
}

void module_configure (void)
{
    /* STAGE: Initialize the VMU sub-system. */

    vmu_init ();

    /* STAGE: Get ourselves controller access too. */

    controller_init ();

    /* STAGE: And now start-up the customization core. */

    customize_init ();
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
