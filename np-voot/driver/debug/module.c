/*  module.c

    $Id: module.c,v 1.3 2002/06/12 09:33:53 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <exception.h>
#include <util.h>
#include <printf.h>
#include <rtl8139c.h>
#include <dumpio.h>

#include "module.h"

static uint8           *level_select    = (uint8 *)     0x8c2d13ac;

static uint16          *anim_mode_a     = (uint16 *)    0x8ccf0228;
static uint16          *anim_mode_b     = (uint16 *)    0x8ccf022a;

static uint16          *p1_health_real  = (uint16 *)    0x8ccf6284;
static uint16          *p1_health_stat  = (uint16 *)    0x8ccf6286;
static uint16          *p1_varmour_mod  = (uint16 *)    0x8ccf63ec;
static uint16          *p1_varmour_base = (uint16 *)    0x8ccf63ee;

static uint16          *p2_health_real  = (uint16 *)    0x8ccf7400;
static uint16          *p2_health_stat  = (uint16 *)    0x8ccf7402;
static uint16          *p2_varmour_mod  = (uint16 *)    0x8ccf7568;
static uint16          *p2_varmour_base = (uint16 *)    0x8ccf756a;

static const uint8  osd_func_key[]      = {
                                            0xe6, 0x2f, 0xd6, 0x2f, 0xc6,
                                            0x2f, 0xb6, 0x2f, 0xa6, 0x2f,
                                            0x96, 0x2f, 0x86, 0x2f, 0xfb,
                                            0xff, 0x22, 0x4f, 0xfc, 0x7f,
                                            0x43, 0x6d, 0x00, 0xe4, 0x43,
                                            0x6b
                                          };

void module_initialize (void)
{
    /* NOTE: We don't need to initialize anything special. */
}

void module_configure (void)
{
    exception_table_entry   new;

    /* STAGE: Configure the UBC channels for the animation and level select breakpoints. */

    ubc_configure_channel (UBC_CHANNEL_A, (uint32) anim_mode_b, UBC_BBR_READ | UBC_BBR_OPERAND);
    ubc_configure_channel (UBC_CHANNEL_B, (uint32) level_select, UBC_BBR_READ | UBC_BBR_OPERAND);

    /* STAGE: Add our handler to the queue. */

    new.type    = EXP_TYPE_GEN;
    new.code    = EXP_CODE_UBC;
    new.handler = debug_handler;

    add_exception_handler (&new);

    /* STAGE: Configure the networking. */

    if (pci_detect ())
    {
        if (pci_bb_init ())
        {
            if (rtl_init ())
            {
            }
        }
    }
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}

static void* my_anim_handler (register_stack *stack, void *current_vector)
{
    static uint32   play_vector         = 0;
    static uint32   osd_vector          = 0;
    static int32    p1_health_save[2]   = {-1, -1};
    static int32    p2_health_save[2]   = {-1, -1};

    /*
        STAGE: Health OSD segment.
        
        Only triggers in game mode.
    */

    if (*anim_mode_b == 0xf && *anim_mode_a == 0x3)
    {
        if ((!play_vector || play_vector == spc ()))
        {
            char    cbuffer[40];

            /* STAGE: So we only latch on a single animation vector. */

            play_vector = spc ();

            /* STAGE: Locate the OSD vector. */

            if (!osd_vector)
                osd_vector = (uint32) search_sysmem_at (osd_func_key, sizeof (osd_func_key), GAME_MEM_START, SYS_MEM_END);

            /* STAGE: If we found it, display the health OSD. */

            if (osd_vector)
            {
                snprintf (cbuffer, sizeof (cbuffer), "Dam 1 [%u > %u | %d]", *p1_health_real, *p1_health_stat, p1_health_save[1]);
                (*(void (*)()) osd_vector) (5, 340, cbuffer);

                snprintf (cbuffer, sizeof (cbuffer), "V.A 1 [%u > %u]", *p1_varmour_base, *p1_varmour_mod);
                (*(void (*)()) osd_vector) (5, 355, cbuffer);

                snprintf (cbuffer, sizeof(cbuffer), "Dam 2 [%u > %u | %d]", *p2_health_real, *p2_health_stat, p2_health_save[1]);
                (*(void (*)()) osd_vector) (5, 400, cbuffer);

                snprintf (cbuffer, sizeof (cbuffer), "V.A 2 [%u > %u]", *p2_varmour_base, *p2_varmour_mod);
                (*(void (*)()) osd_vector) (5, 415, cbuffer);
            }

            /* STAGE: If the health values have changed, update our status. */

            if (*p1_health_real != p1_health_save[0])
            {
                if (p1_health_save[0] >= 0)
                    p1_health_save[1] = p1_health_save[0] - *p1_health_real;

                p1_health_save[0] = *p1_health_real;
            }

            if (*p2_health_real != p2_health_save[0])
            {
                if (p2_health_save[0] >= 0)
                    p2_health_save[1] = p2_health_save[0] - *p2_health_real;

                p2_health_save[0] = *p2_health_real;
            }
        }
    }
    else
    {
        play_vector         = osd_vector        = 0;
        p1_health_save[0]   = p2_health_save[0] = -1;
        p1_health_save[1]   = p2_health_save[1] = 0;
    }

    return current_vector;
}

static void* my_debug_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: Forces our level to be Tangram's stage. */    

    *level_select = 0x14;

    return current_vector;
}

void* debug_handler (register_stack *stack, void *current_vector)
{
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */
        current_vector = my_anim_handler (stack, current_vector);
    }

    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

        /* STAGE: Pass control to the actual code base. */
        current_vector = my_debug_handler (stack, current_vector);
    }

    return current_vector;
}
