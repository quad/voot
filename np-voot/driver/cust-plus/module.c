/*  module.c

    $Id: module.c,v 1.9 2003/03/06 07:37:56 quad Exp $

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
    /* STAGE: Make sure it's always aware of the situation! */

    customize_init ();
}
