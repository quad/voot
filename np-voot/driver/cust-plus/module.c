/*  module.c

    $Id: module.c,v 1.2 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.

*/

#include <vars.h>
#include <vmu.h>
#include <controller.h>
#include "customize.h"

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
