/*  module.c

    $Id: module.c,v 1.4 2002/08/04 05:48:05 quad Exp $

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
