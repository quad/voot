/*  customize.c

    $Id: customize.c,v 1.9 2002/11/14 22:35:04 quad Exp $

DESCRIPTION

    Customization core.

TODO

    Versus mode needs to be reversed.

    Menu select side should be checked instead of game side in Cable Versus mode.

    Re-reverse the entire system to have data transmitted across the line.
    Customized heads are *for certain* not transmitted.

    Finish documenting the player gamedata structure and switch it its
    usage.

*/

#include <vars.h>
#include <ubc.h>
#include <exception.h>
#include <util.h>
#include <malloc.h>
#include <searchmem.h>
#include <gamedata.h>
#include <printf.h>
#include <controller.h>
#include <vmu.h>
#include <anim.h>

#include "customize.h"

static exception_handler_f  old_ubc_handler;
static anim_render_chain_f  old_anim_handler;

static const uint8  osd_func_key[]      = {
                                            0xe6, 0x2f, 0xd6, 0x2f, 0xc6,
                                            0x2f, 0xb6, 0x2f, 0xa6, 0x2f,
                                            0x96, 0x2f, 0x86, 0x2f, 0xfb,
                                            0xff, 0x22, 0x4f, 0xfc, 0x7f,
                                            0x43, 0x6d, 0x00, 0xe4, 0x43,
                                            0x6b
                                          };

static const uint8  custom_func_key[]   = {
                                            0xe6, 0x2f, 0x53, 0x6e, 0xd6,
                                            0x2f, 0xef, 0x63, 0xc6, 0x2f,
                                            0x38, 0x23
                                          };
static uint32       custom_func;

static customize_ipc    ipc         = C_IPC_START;
static uint8            player      = 0;
static uint8            side        = 0;
static uint8            file_number = 0;
static uint8           *file_buffer = NULL;
static customize_data*  colors[2][VR_SENTINEL]; 

/* NOTE: References into the gamedata structure. */

static uint8           *p1_vr_loc       = (uint8 *)     0x8ccf6236;
static uint16          *p1_health_real  = (uint16 *)    0x8ccf6284;
static uint16          *p1_health_stat  = (uint16 *)    0x8ccf6286;
static uint16          *p1_varmour_mod  = (uint16 *)    0x8ccf63ec;
static uint16          *p1_varmour_base = (uint16 *)    0x8ccf63ee;

static uint8           *p2_vr_loc       = (uint8 *)     0x8ccf73b2;
static uint16          *p2_health_real  = (uint16 *)    0x8ccf7400;
static uint16          *p2_health_stat  = (uint16 *)    0x8ccf7402;
static uint16          *p2_varmour_mod  = (uint16 *)    0x8ccf7568;
static uint16          *p2_varmour_base = (uint16 *)    0x8ccf756a;

static void customize_clear_player (uint32 player, bool do_head)
{
    uint32  vr;

    /* STAGE: Free and null out all the customization for a particular player. */

    for (vr = 0; vr < VR_SENTINEL; vr++)
    {
        if (colors[player][vr])
        {
            free (colors[player][vr]);

            colors[player][vr] = NULL;
        }
    }

    if (do_head)
        GAMEDATA_OPT->cust_head.index[player] = 0x0;
}

static void* my_customize_handler (register_stack *stack, void *current_vector)
{
    voot_vr_id  vr;
    uint32      player;
    uint8      *palette;

    vr      = stack->r4;
    player  = stack->r5;
    palette = (uint8 *) stack->r6;

    /* STAGE: Ensure we've been given sane values. */

    if ((vr >= 0 && vr < VR_SENTINEL) && (player == 0 || player == 1))
    {
        if (colors[player][vr])
        {
            memcpy (palette, colors[player][vr]->palette, sizeof (customize_data));

            GAMEDATA_OPT->cust_emb.index[player]    = TRUE;
            GAMEDATA_OPT->cust_head.index[player]   = colors[player][vr]->head;
        }

        customize_clear_player (player, FALSE);
    }

    return current_vector;
}

static void maybe_start_load_customize (void)
{
    bool    go;

    go = FALSE;

    /* STAGE: [Step 1-P1] First player requests customization load. */

    if (ipc == C_IPC_START && (check_controller_press (CONTROLLER_PORT_A0) & CONTROLLER_MASK_BUTTON_Y))
    {
        /*
            STAGE: Determine which colorization scheme to load.
            
            If the control port is 0 (A), our side is equal to the DNA/RNA
            select.
            
            If the control port is 1 (B), our side is equal to the reverse
            of the DNA/RNA select.
            
            Our player is the controlling port.
        */

        if (GAMEDATA_OPT->control_port)
            side = !(*GAMEDATA_GAME_SIDE);
        else
            side = *GAMEDATA_GAME_SIDE;

        player = GAMEDATA_OPT->control_port;

        /* STAGE: Begin the mount scan for a VMS. */

        GAMEDATA_OPT->data_port = VMU_PORT_A1;
        go                      = TRUE;
    }
    /* STAGE: [Step 1-P2] Second player requests customization load. */
    else if (ipc == C_IPC_START && (check_controller_press (CONTROLLER_PORT_B0) & CONTROLLER_MASK_BUTTON_Y))
    {
        /*
            STAGE: Determine which colorization scheme to load.

            If the control port is 0 (A), our side is equal to the reverse
            of the DNA/RNA select.

            If the control port is 1 (B), our side is equal to DNA/RNA
            select.

            Our player is the reverse of the controlling port.
        */
                 
        if (!(GAMEDATA_OPT->control_port))
            side = !(*GAMEDATA_GAME_SIDE);
        else
            side = *GAMEDATA_GAME_SIDE;

        player = !(GAMEDATA_OPT->control_port);

        /* STAGE: Begin the mount scan for a VMS. */

        GAMEDATA_OPT->data_port = VMU_PORT_B1;
        go                      = TRUE;
    }

    if (go)
    {
        /* STAGE: Make sure the colors for this player are cleared out. */

        customize_clear_player (player, TRUE); 

        /* STAGE: Finish initializing the IPC for the mount scan. */

        ipc         = C_IPC_MOUNT_SCAN_1;
        file_number = 0;

        /* STAGE: Send the mount request. */

        vmu_mount (GAMEDATA_OPT->data_port);
    }
}

static void maybe_do_load_customize (void)
{
    /* STAGE: [Step 2] If we're involved in either step of the mount scan. */

    if ((ipc == C_IPC_MOUNT_SCAN_1 || ipc == C_IPC_MOUNT_SCAN_2) && !vmu_status (GAMEDATA_OPT->data_port))
    {
        uint32  retval;
        char    filename[VMU_MAX_FILENAME];

        /* STAGE: Determine if we're mounted by searching for a customization file. */

        for (retval = 1; file_number < 100; file_number++)
        {
            snprintf (filename, sizeof (filename), "VOORATAN.C%02u", file_number);

            retval = vmu_exists_file (GAMEDATA_OPT->data_port, filename);

            if (!retval)
                break;
        }

        /* STAGE: We found the file! Now lets start accessing it... */

        if (!retval)
        {
            /* STAGE: Give us a 10 block temporary file buffer. */

            file_buffer = malloc (512 * CUSTOMIZE_VMU_SIZE);

            /* STAGE: Make sure we actually obtained the file buffer. */

            if (file_buffer)
            {
                vmu_load_file (GAMEDATA_OPT->data_port, filename, file_buffer, CUSTOMIZE_VMU_SIZE);

                ipc = C_IPC_LOAD;
            }
            else
            {
                ipc = C_IPC_START;
            }
        }
        /* STAGE: Didn't find a customization file on this VMU, so check the next port. */
        else if (ipc == C_IPC_MOUNT_SCAN_1)
        {
            (GAMEDATA_OPT->data_port)++;
            file_number = 0;

            vmu_mount (GAMEDATA_OPT->data_port);

            ipc = C_IPC_MOUNT_SCAN_2;
        }
        /* STAGE: Apparently it wasn't found on either port. Abort. */
        else if (ipc == C_IPC_MOUNT_SCAN_2)
        {
            GAMEDATA_OPT->data_port = VMU_PORT_NONE;

            ipc = C_IPC_START;
        }
    }
    /* STAGE: [Step 3] Loaded customization file. */
    else if (ipc == C_IPC_LOAD && !vmu_status (GAMEDATA_OPT->data_port))
    {
        voot_vr_id  vr;

        vr = file_buffer[CUSTOMIZE_VMU_VR_IDX];

        /*
            STAGE: If it is one of the VR types we're storing, copy the data
            over.
            
            If we already stored one, skip it.
        */
        if (vr < VR_SENTINEL && !colors[player][vr])
        {
            customize_data  temp_color;
            uint8           temp_head;

            /*
                STAGE: This whole trick is to keep memory from becoming very
                fragmented.
                
                With only 64k, it's good to keep this in mind.
            */

            memcpy (temp_color.palette, file_buffer + CUSTOMIZE_VMU_COLOR_IDX + (side * CUSTOMIZE_PALETTE_SIZE), CUSTOMIZE_PALETTE_SIZE);
            temp_head = file_buffer[CUSTOMIZE_VMU_HEAD_IDX];

            free (file_buffer);

            /* STAGE: Make sure we actually obtained the memory for the customization. */

            colors[player][vr] = malloc (sizeof (customize_data));

            if (colors[player][vr])
            {
                memcpy (colors[player][vr]->palette, temp_color.palette, CUSTOMIZE_PALETTE_SIZE);
                colors[player][vr]->head = temp_head;
            }
        }
        else
        {
            free (file_buffer);
        }

        file_number++;
        ipc = C_IPC_MOUNT_SCAN_2;
    }
}

static void maybe_find_customize (void)
{
    static char     module_save[VMU_MAX_FILENAME];

    /* STAGE: Detect module changeovers and search for the customization function. */

    if (memcmp ((uint8 *) custom_func, custom_func_key, sizeof (custom_func_key)) && strcmp (module_save, GAMEDATA_OPT->module_name))
    {
        /* STAGE: Make sure we aren't constantly accessing after every failure... */

        strncpy (module_save, GAMEDATA_OPT->module_name, sizeof (module_save));

        /* STAGE: Try to locate one of the customization functions. */

        custom_func = (uint32) search_memory_at (custom_func_key, sizeof (custom_func_key), (const uint8 *) OVERLAY_MEM_START, (const uint8 *) SYS_MEM_END);

        /* STAGE: Place the breakpoint on the customization function, if we found it. */

        if (custom_func)
            ubc_configure_channel (UBC_CHANNEL_B, custom_func, UBC_BBR_READ | UBC_BBR_INSTRUCT);
        else
            ubc_clear_channel (UBC_CHANNEL_B);
    }
}

static void maybe_load_customize (uint16 anim_mode_a, uint16 anim_mode_b)
{
    /* STAGE: See if we need to load customization information. */

    if ((anim_mode_a == 0x0 && anim_mode_b == 0x2)  ||    /* NOTE: Training Mode select. */
        (anim_mode_a == 0x6 && anim_mode_b == 0x4)  ||    /* NOTE: Versus Cable select. */
        (anim_mode_a == 0x0 && anim_mode_b == 0x5)  ||    /* NOTE: Single Player 3d select. */
        (anim_mode_a == 0x2 && anim_mode_b == 0x9)  ||    /* NOTE: Single Player quick select. */
        (anim_mode_a == 0x3 && anim_mode_b == 0x29) ||    /* NOTE: Single Player quick continue. */
        (anim_mode_a == 0x5 && anim_mode_b == 0x2))       /* NOTE: Versus select. */
    {
        voot_vr_id  p1_vr;
        voot_vr_id  p2_vr;

        /* STAGE: Update the OSD with whatever we're doing. */

        switch (ipc)
        {
            case C_IPC_START :
                anim_printf_debug (0.0, 0.0, "Press Y to load customized VRs.");
                break;

            case C_IPC_MOUNT_SCAN_1 :
                anim_printf_debug (0.0, 0.0, "Searching SLOT A for customized VR data...");
                break;

            case C_IPC_MOUNT_SCAN_2 :
                anim_printf_debug (0.0, 0.0, "Searching SLOT B for customized VR data...");
                break;

            case C_IPC_LOAD :
                anim_printf_debug (0.0, 0.0, "Loading customized VR data...");
                break;
        }

        /* STAGE: Handle customization load process. */

        maybe_start_load_customize ();
        maybe_do_load_customize ();

        /* STAGE: An ugly hack of an attempt at customized heads. */

        p1_vr = *p1_vr_loc;
        p2_vr = *p2_vr_loc;

        if ((p1_vr >= 0) && (p1_vr < VR_SENTINEL) && colors[0][p1_vr])
            GAMEDATA_OPT->cust_head.player.p1 = colors[0][p1_vr]->head;
        else
            GAMEDATA_OPT->cust_head.player.p1 = 0x0;

        if ((p2_vr >= 0) && (p2_vr < VR_SENTINEL) && colors[1][p2_vr])
            GAMEDATA_OPT->cust_head.player.p2 = colors[1][p2_vr]->head;
        else
            GAMEDATA_OPT->cust_head.player.p2 = 0x0;
    }
    else
    {
        /* STAGE: Handle the customization load process. (or cleanup) */

        maybe_do_load_customize ();

        /* STAGE: If we move back to the main menu, clear all the information. */

        if (anim_mode_a == 0x2 && !(anim_mode_b == 0x9 || anim_mode_b == 0xa))
        {
            customize_clear_player (0, TRUE);
            customize_clear_player (1, TRUE);
        }
    }

}

static void maybe_display_stats (uint16 anim_mode_a, uint16 anim_mode_b)
{
    static void    *osd_vector          = 0;
    static int32    p1_health_save[2]   = {-1, -1};
    static int32    p2_health_save[2]   = {-1, -1};

    /*
        STAGE: Health OSD segment.
        
        NOTE: Only triggers in game mode.
    */

    if (anim_mode_b == 0xf && anim_mode_a == 0x3 && (!strcmp ("training54.bin", GAMEDATA_OPT->module_name) || !strcmp ("training.bin", GAMEDATA_OPT->module_name)))
    {
        char    cbuffer[40];

        /* STAGE: Locate the OSD vector. */

        if (!osd_vector || memcmp (osd_vector, osd_func_key, sizeof (osd_func_key)))
            osd_vector = search_memory_at (osd_func_key, sizeof (osd_func_key), (const uint8 *) OVERLAY_MEM_START, (const uint8 *) SYS_MEM_END);

        /* STAGE: If we found it, display the health OSD. */

        if (osd_vector)
        {
            snprintf (cbuffer, sizeof (cbuffer), "Dam 1 [%u > %u | %d]", *p1_health_real, *p1_health_stat, p1_health_save[1]);
            (*(void (*)()) osd_vector) (5, 340, cbuffer);

            snprintf (cbuffer, sizeof (cbuffer), "V.A 1 [%u > %u]", *p1_varmour_base, *p1_varmour_mod);
            (*(void (*)()) osd_vector) (5, 355, cbuffer);

            snprintf (cbuffer, sizeof (cbuffer), "Dam 2 [%u > %u | %d]", *p2_health_real, *p2_health_stat, p2_health_save[1]);
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
    else
    {
        p1_health_save[0]   = p2_health_save[0] = -1;
        p1_health_save[1]   = p2_health_save[1] = 0;
    }

}

static void my_anim_handler (uint16 anim_mode_a, uint16 anim_mode_b)
{
    maybe_find_customize ();
    maybe_load_customize (anim_mode_a, anim_mode_b);
    maybe_display_stats (anim_mode_a, anim_mode_b);

    /*
        STAGE: Check if we're in a VR Customization module.
        
        If so, enable Button Y to start VR Test. The module value only
        applies for 5.66.
    */

    if (anim_mode_a == 0x0 && anim_mode_b == 0x0 && !strcmp ("vrcust.bin", (const char *) 0x8ccf9edc))
    {
        if (check_controller_press (GAMEDATA_OPT->data_port) & CONTROLLER_MASK_BUTTON_Y)
        {
            uint8  *vrcust_action = (uint8 *) 0x8c275224;

            *vrcust_action = 0x1a;
        }
    }

    if (old_anim_handler)
        return old_anim_handler (anim_mode_a, anim_mode_b);
}

static void* customize_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: In the case of the customize function (channel B) exception. */

    if (ubc_is_channel_break (UBC_CHANNEL_B))
        current_vector = my_customize_handler (stack, current_vector);

    if (old_ubc_handler)
        return old_ubc_handler (stack, current_vector);
    else
        return current_vector;
}

void customize_init (void)
{
    static bool exp_inited;
    static bool anim_inited;

    if (!exp_inited)
    {
        exception_table_entry   new;

        /* STAGE: Ensure we're hooked on the UBC as well. */

        new.type    = EXP_TYPE_GEN;
        new.code    = EXP_CODE_UBC;
        new.handler = customize_handler;

        exp_inited = exception_add_handler (&new, &old_ubc_handler);
    }

    if (exp_inited && !anim_inited)
    {
        /* STAGE: Initialize and configure the animation render hook. */

        anim_init ();

        anim_inited = anim_add_render_chain (my_anim_handler, &old_anim_handler);
    }

    /* STAGE: Initialize the VMU sub-system. */

    vmu_init ();

    /* STAGE: Get ourselves controller access too. */

    controller_init ();
}
