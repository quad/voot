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
#include "biosfont.h"

#include "customize.h"

static const uint8 custom_func_key[] = { 0xe6, 0x2f, 0x53, 0x6e, 0xd6, 0x2f, 0xef, 0x63, 0xc6, 0x2f, 0x38, 0x23 };
static const uint8 osd_func_key[] = { 0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f, 0xb6, 0x2f, 0xa6, 0x2f, 0x96, 0x2f, 0x86, 0x2f, 0xfb, 0xff, 0x22, 0x4f, 0xfc, 0x7f, 0x43, 0x6d, 0x00, 0xe4, 0x43, 0x6b };
static uint32 custom_func;

void customize_init(void)
{
    exception_table_entry new;

    /* STAGE: Break on the secondary animation vector. */
    *UBC_R_BARA = 0x8ccf022a;
    *UBC_R_BAMRA = UBC_BAMR_NOASID;
    *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_OPERAND;

    ubc_wait();

    /* STAGE: Add exception handler for serial access. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = customize_handler;

    add_exception_handler(&new);
}

static void* my_anim_handler(register_stack *stack, void *current_vector)
{
    static uint32 play_vector = 0;
    static uint32 osd_vector = 0;
    uint16 *anim_mode_a = (uint16 *) 0x8ccf0228;
    uint16 *anim_mode_b = (uint16 *) 0x8ccf022a;

/*
    8c389468
    customize (VR %u, PLR %u, CUST %x, CMAP %x)
*/

    /* STAGE: Health OSD segment. */
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

            /* STAGE: Test biosfont OSD. */
            bfont_draw_str(VRAM_START, 640, "XYZZY");
        }
    }
    else
    {
        play_vector = 0;
        osd_vector = 0;
    }

    return current_vector;
}

void* customize_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: In the case of the secondary animation mode (channel A) exception. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */
        return my_anim_handler(stack, current_vector);
    }
    else
        return current_vector;
}
