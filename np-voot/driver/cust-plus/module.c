/*  module.c

    $Id: module.c,v 1.7 2002/12/16 07:51:00 quad Exp $

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
    /* NOTE: We don't need to reinitialize anything. */
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
