/*  module.c

    $Id: module.c,v 1.6 2002/11/24 14:56:46 quad Exp $

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
    /* STAGE: And now start-up the customization core. */

    customize_init ();
}

void module_reconfigure (void)
{
    /* TODO: Add in necessary reconfiguration for customization to last past Drimaga menu. */
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
