/*  controller.c

DESCRIPTION

    A controller access module. This uses the reverse engineered sections of
    VOOT's (Katana's?) controller read routines, thus saving us a UBC break.

*/

#include "vars.h"
#include "util.h"

#include "controller.h"

static uint8 *controller_root;
static const uint8 controller_key[] = { 0x34, 0xe0, 0x04, 0xd3, 0x07, 0x04, 0x1a, 0x00 };

void controller_init (void)
{
    if (!controller_root)
        controller_root = search_sysmem(controller_key, sizeof(controller_key));
}

controller_status* check_controller_info (controller_port port)
{
    if (controller_root)
        return (*(controller_status* (*)()) controller_root)(port);
    else
        return NULL;
}

uint32 check_controller_press (controller_port port)
{
    controller_status *info;

    info = check_controller_info(port);

    if (info)
        return info->button_pressed;
    else
        return 0;
}
