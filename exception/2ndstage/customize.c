/*  customize.c

DESCRIPTION

    Customization reverse engineering helper code.

TODO

    VMU Loading.

*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "util.h"
#include "voot.h"
#include "gamedata.h"
#include "printf.h"
#include "controller.h"
#include "vmu.h"

#include "customize.h"

static const uint8 osd_func_key[] = { 0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f, 0xb6, 0x2f, 0xa6, 0x2f, 0x96, 0x2f, 0x86, 0x2f, 0xfb, 0xff, 0x22, 0x4f, 0xfc, 0x7f, 0x43, 0x6d, 0x00, 0xe4, 0x43, 0x6b };
/* e6 2f 53 6e d6 2f ef 63 c6 2f 38 23 */
static const uint8 custom_func_key[] = { 0xe6, 0x2f, 0x53, 0x6e, 0xd6, 0x2f, 0xef, 0x63, 0xc6, 0x2f, 0x38, 0x23 };
static char module_save[20];
static uint32 custom_func;

static customize_data* colors[2][13]; 

void customize_init(void)
{
    exception_table_entry new;

    /* STAGE: Break on the secondary animation vector. */
    *UBC_R_BARA = 0x8ccf022a;
    *UBC_R_BAMRA = UBC_BAMR_NOASID;
    *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_OPERAND;
    //*UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_INSTRUCT;

    ubc_wait();

    /* STAGE: Add exception handler for serial access. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = customize_handler;

    add_exception_handler(&new);
}

static void* my_customize_handler(register_stack *stack, void *current_vector)
{
    voot_debug("customize (VR %x, PLAYER %u, CUSTOMIZE %x, CMAP %x)", stack->r4, stack->r5, stack->r6, stack->r7);

    if (colors[stack->r5][stack->r4])
        stack->r6 = (uint32) colors[stack->r5][stack->r4]; 

    return current_vector;
}

static void* my_anim_handler(register_stack *stack, void *current_vector)
{
    static uint32 play_vector = 0;
    static uint32 osd_vector = 0;
    static uint16 save_mode_a = 0;
    static uint16 save_mode_b = 0;
    uint16 *anim_mode_a = (uint16 *) 0x8ccf0228;
    uint16 *anim_mode_b = (uint16 *) 0x8ccf022a;

/*
    8c389468
    customize (VR %u, PLR %u, CUST %x, CMAP %x)
*/

    /* STAGE: This code occurs are every animation changeover. (game code latch) */
    if ((save_mode_a != *anim_mode_a || save_mode_b != *anim_mode_b) && spc() > 0x8c270000)
    {
        /* STAGE: Detect module changeovers and search for the customization function. */
        if (memcmp((uint8 *) custom_func, custom_func_key, sizeof(custom_func_key)) && strcmp(module_save, (const char *) VOOT_MODULE_NAME))
        {
            strncpy(module_save, (const char *) VOOT_MODULE_NAME, sizeof(module_save));
            custom_func = (uint32) search_sysmem_at(custom_func_key, sizeof(custom_func_key), GAME_MEM_START, SYS_MEM_END);

            voot_debug("custom_func = %x", custom_func);

            /* STAGE: Place the breakpoint on the customization function, if we found it. */
            if (custom_func)
            {
                *UBC_R_BARB = custom_func;
                *UBC_R_BAMRB = UBC_BAMR_NOASID;
                *UBC_R_BBRB = UBC_BBR_READ | UBC_BBR_INSTRUCT;
            }
            else
                *UBC_R_BBRB = 0;

            ubc_wait();
        }

        /* STAGE: Make sure we don't catch next time around. */
        save_mode_a = *anim_mode_a;
        save_mode_b = *anim_mode_b;
    }

    /* STAGE: Health OSD segment - only triggers in game mode. */
    if (*anim_mode_b == 0xf && *anim_mode_a == 0x3)
    {
        if (!play_vector || play_vector == spc())
        {
            uint16 *p1_health_a = (uint16 *) 0x8CCF6284;
            uint16 *p1_health_b = (uint16 *) 0x8CCF6286;

            uint16 *p1_varmour_mod = (uint16 *) 0x8CCF63ec;
            uint16 *p1_varmour_base = (uint16 *) 0x8CCF63ee;

            uint16 *p2_health_a = (uint16 *) 0x8CCF7400;
            uint16 *p2_health_b = (uint16 *) 0x8CCF7402;

            uint16 *p2_varmour_mod = (uint16 *) 0x8CCF7568;
            uint16 *p2_varmour_base = (uint16 *) 0x8CCF756a;

            char cbuffer[40];

            /* STAGE: So we only latch on a single animation vector. */
            play_vector = spc();

            /* STAGE: Locate the OSD vector. */
            if (!osd_vector)
                osd_vector = (uint32) search_sysmem_at(osd_func_key, sizeof(osd_func_key), GAME_MEM_START, SYS_MEM_END);

            /* STAGE: If we found it, display the health OSD. */
            if (osd_vector)
            {
                snprintf(cbuffer, sizeof(cbuffer), "Health 1 [%u -> %u]", *p1_health_a, *p1_health_b);
                (*(void (*)()) osd_vector)(5, 100, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "V.Armour 1 [%u -> %u]", *p1_varmour_base, *p1_varmour_mod);
                (*(void (*)()) osd_vector)(5, 115, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "Health 2 [%u -> %u]", *p2_health_a, *p2_health_b);
                (*(void (*)()) osd_vector)(5, 200, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "V.Armour 2 [%u -> %u]", *p2_varmour_base, *p2_varmour_mod);
                (*(void (*)()) osd_vector)(5, 215, cbuffer);
            }
        }
    }
    else
    {
        play_vector = 0;
        osd_vector = 0;
    }

    return current_vector;
}

void maybe_load_customize()
{
    static customize_ipc ipc = C_IPC_START;
    static uint32 player = 0;
    static uint32 side = 0;
    vmu_port *data_port = (vmu_port *) (0x8ccf9f06);
    static uint8 file_number = 0;
    static uint8 *file_buffer = NULL;

    /* NOTE: This is a rather complex piece of IPC. If it works, I'll be suprised. */

    /* STAGE: [Step 1-P1] First player requests customization load. */
    if ((check_controller_press(CONTROLLER_PORT_A0) & CONTROLLER_MASK_BUTTON_Y) && !ipc)
    {
        /* STAGE: Clear out the entire customization structure. */
        for (side = 0; side < 2; side++)
        {
            for (player = 0; player < 13; player++)
            {
                free(colors[side][player]);
                colors[side][player] = NULL;
            }
        }
        
        /* STAGE: Right here is where we would perform the huge logic check
            to find out which VR (1 or 2) the owner of this controller is. 
            However, since this is debugging code, we'll assume it's player
            1. */

        player = side = 0;

        voot_debug("Beginning search for customization data.");

        /* STAGE: Begin the mount scan for a VMS. */
        *data_port = VMU_PORT_A1;
        ipc = C_IPC_MOUNT_SCAN_1;
        file_number = 0;

        vmu_mount(*data_port);
    }
    /* STAGE: [Step 2] If we're involved in either step of the mount scan. */
    else if ((ipc == C_IPC_MOUNT_SCAN_1 || ipc == C_IPC_MOUNT_SCAN_2) && !vmu_status(*data_port))
    {
        uint32 retval;
        char filename[13];

        /* STAGE: Determine if we're mounted by searching for a customization file. */
        for (retval = 1; file_number < 100; file_number++)
        {
            snprintf(filename, sizeof(filename), "VOORATAN.C%02u", file_number);

            retval = vmu_exists_file(*data_port, filename);

            if (!retval)
                break;
        }

        /* STAGE: We found the file! Now lets start accessing it... */
        if (!retval)
        {
            voot_debug("Found '%s' on port %u and loading.", filename, *data_port);

            /* STAGE: Give us a 10 block temporary file buffer. */
            file_buffer = malloc(512 * 10);

            /* STAGE: Make sure we actually obtained the file buffer. */
            if (file_buffer)
            {
                vmu_load_file(*data_port, filename, file_buffer, 10);

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
            voot_debug("No customization on port %u, trying next port...", *data_port);

            (*data_port)++;
            file_number = 0;

            vmu_mount(*data_port);

            ipc = C_IPC_MOUNT_SCAN_2;
        }
        /* STAGE: Apparently it wasn't found on either port. Abort. */
        else if (ipc == C_IPC_MOUNT_SCAN_2)
        {
            voot_debug("No customization data on port %u, giving up.", *data_port);

            *data_port = VMU_PORT_NONE;

            ipc = C_IPC_START;
        }
    }
    /* STAGE: [Step 3] Loaded customization file. */
    else if (ipc == C_IPC_LOAD && !vmu_status(*data_port))
    {
        voot_vr_id vr;

        voot_debug("Customization data loaded. [%x]", file_buffer[0]);

        vr = file_buffer[0x2A0];

        switch (vr)
        {
            case VR_DORDRAY:
                voot_debug("VR: DORDRAY");
                break;

            case VR_BALSERIES:
                voot_debug("VR: BALSERIES");
                break;

            case VR_CYPHER:
                voot_debug("VR: CYPHER");
                break;

            case VR_GRYSVOK:
                voot_debug("VR: GRYSVOK");
                break;

            case VR_APHARMDB:
                voot_debug("VR: APHARMDB");
                break;

            case VR_APHARMDB_B:
                voot_debug("VR: APHARMDB_B");
                break;

            case VR_RAIDEN:
                voot_debug("VR: RAIDEN");
                break;

            case VR_TEMJIN:
                voot_debug("VR: TEMJIN");
                break;

            case VR_FEIYEN:
                voot_debug("VR: FEIYEN");
                break;

            case VR_ANGELAN:
                voot_debug("VR: ANGELAN");
                break;

            case VR_SPECINEFF:
                voot_debug("VR: SPECINEFF");
                break;

            case VR_APHARMDS:
                voot_debug("VR: APHARMDS");
                break;

            case VR_AJIM:
                voot_debug("VR: AJIM");
                break;

            default:
                voot_debug("VR: Unknown [%x]", vr);
                break;
        }

        /* STAGE: If it is one of the VR types we're storing, copy the data over. */
        if (vr < 13 && !colors[side][vr])
        {
            customize_data temp;

            /* STAGE: This whole trick is to keep memory from becoming very
                fragmented. With only 64k, it's good to keep this in mind. */

            memcpy(&temp, file_buffer + 0x2b0 + (side * 0x200), sizeof(customize_data));

            free(file_buffer);
            colors[side][vr] = malloc(sizeof(customize_data));

            /* STAGE: Make sure we actually obtained the memory for the customization. */
            if (colors[side][vr])
                memcpy(colors[side][vr], &temp, sizeof(customize_data));
        }
        else
            free(file_buffer);

        file_number++;
        ipc = C_IPC_MOUNT_SCAN_2;
    }
}

void* customize_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: In the case of the secondary animation mode (channel A) exception. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */
        current_vector = my_anim_handler(stack, current_vector);
    }

    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

        /* STAGE: Pass control to the actual code base. */
        current_vector = my_customize_handler(stack, current_vector);
    }

    return current_vector;
}
