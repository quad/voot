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

#include "customize.h"

static const uint8 custom_func_key[] = { 0xe6, 0x2f, 0x53, 0x6e, 0xd6, 0x2f, 0xef, 0x63, 0xc6, 0x2f, 0x38, 0x23 };
static const uint8 osd_func_key[] = { 0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f, 0xb6, 0x2f, 0xa6, 0x2f, 0x96, 0x2f, 0x86, 0x2f, 0xfb, 0xff, 0x22, 0x4f, 0xfc, 0x7f, 0x43, 0x6d, 0x00, 0xe4, 0x43, 0x6b };
static uint32 custom_func;
static char gamebin_c[20];
static customize_check_mode custom_status;

#define HARD_BREAK
//#define VECTOR_TRACK

static void customize_locate_func(void)
{
#ifdef HARD_BREAK
    return;
#endif

    /* STAGE: Locate the customization function beginning. */
    custom_func = (uint32) search_sysmem_at(custom_func_key, sizeof(custom_func_key), GAME_MEM_START, SYS_MEM_END);

    /* STAGE: If we found the customization function, configure UBC Channel
        A to break on it. */
    if (custom_func)
    {
        *UBC_R_BARA = custom_func;
        *UBC_R_BAMRA = UBC_BAMR_NOASID;
        *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_INSTRUCT;
    }
    /* STAGE: Otherwise set our break on Temjin's customize data. */
    else
    {
        *UBC_R_BARA = 0x8ccf9f15;
        *UBC_R_BAMRA = UBC_BAMR_NOASID;
        *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_OPERAND;
    }

    ubc_wait();
}

bool customize_reinit(void)
{
#ifdef HARD_BREAK
    //*UBC_R_BARA = 0x8c397f62;     /* Main animation loop breakpoint. */
    *UBC_R_BARA = 0x8ccf022a;
    *UBC_R_BAMRA = UBC_BAMR_NOASID;
    //*UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_INSTRUCT;
    *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_OPERAND;

    ubc_wait();

    return TRUE;
#endif

    /* STAGE: Is the current vector still valid? */
    if (custom_status == RUN && !memcmp((uint8 *) custom_func, custom_func_key, sizeof(custom_func_key)))
        return TRUE;
    /* STAGE: Is the current vector still known broken? (with no chance of having changed. */
    else if (custom_status == LOAD && !strcmp((uint8 *) VOOT_MODULE_NAME, gamebin_c))
        return FALSE;
    /* STAGE: Ignore any references from outside the game module. */
    else if (spc() < 0x8c270000)
        return FALSE;

    /* STAGE: Function reference is either invalid or we're allowed to search again. */
    customize_locate_func();

    /* STAGE: If the function was located, set our status to run but DO NOT
        allow the handler to process this iteration. */
    if (custom_func)
    {
        replace_game_text("GATE FIELD INTERVENTION",
                          "A shark on whiskey");
        replace_game_text("STATUS CRITICAL    ",
                          "is mighty risky.");
        replace_game_text("VR EYE CAMERA 1P",
                          "PUNK BITCH CAM");
        replace_game_text("(pause)",
                          "(stop)");

        custom_status = RUN;
    }
    /* STAGE: No customization function was located. Continue in LOAD mode. */
    else
    {
        custom_status = LOAD;
        strncpy(gamebin_c, (uint8 *) VOOT_MODULE_NAME, sizeof(gamebin_c));
    }

    /* STAGE: This iteration is invalid. Don't let the handler touch it. */
    return FALSE;
}

void customize_init(void)
{
    exception_table_entry new;

    /* STAGE: Notify the module we're in the LOAD process. */
    custom_status = LOAD;
    custom_func = 0x0;

    /* STAGE: Make a (futile) search for the customization function. */
    customize_reinit();

    /* STAGE: Add exception handler for serial access. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = customize_handler;

    add_exception_handler(&new);
}

static void* my_customize_handler(register_stack *stack, void *current_vector)
{
    static uint32 play_vector = 0;
    static uint32 osd_vector = 0;

    /* STAGE: Make a vague attempt at a reinitialization. */
    if (!customize_reinit())
        return current_vector;

/*
    8c389468
    customize (VR %u, PLR %u, CUST %x, CMAP %x)

    8c0201b4
    vmu_load_file(DRIVE %u, FILE %s, BUFFER %x, NUM_BLOCKS %u)

    npclient: [npc|INFO] DEBUG(slave): (0, 'VOORATAN.SYS', 8ce01c00, from 8c30b8ac
    npclient: [npc|INFO] DEBUG(slave): (1, 'VOORATAN.SYS', 8ce01c00, from 8c30b8ac
*/

    //voot_printf(VOOT_PACKET_TYPE_DEBUG, "func (%x, %x, %x, %x)", stack->r4, stack->r5, stack->r6, stack->r7);

    //voot_printf(VOOT_PACKET_TYPE_DEBUG, "(%u [%u], '%s', %x, %u) from %x", stack->r4, *((uint8 *) (0x8ccf0000 + 0x9f06)), stack->r5, stack->r6, stack->r7, stack->pr);

    if (*((uint16 *) 0x8ccf022a) == 0xf && *((uint16 *) 0x8ccf0228) == 0x3)
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
            
            uint16 *p2_stun = (uint16 *) 0x8CCF66D6;
            char cbuffer[40];

            play_vector = spc();

            if (!osd_vector)
                osd_vector = (uint32) search_sysmem_at(osd_func_key, sizeof(osd_func_key), GAME_MEM_START, SYS_MEM_END);

            if (osd_vector)
            {
                snprintf(cbuffer, sizeof(cbuffer), "Health 1 [%u -> %u]", *p1_health_b, *p1_health_a);
                (*(void (*)()) osd_vector)(5, 100, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "V.Armour 1 [%u -> %u]", *p1_varmour_base, *p1_varmour_mod);
                (*(void (*)()) osd_vector)(5, 115, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "Health 2 [%u -> %u]", *p2_health_b, *p2_health_a);
                (*(void (*)()) osd_vector)(5, 200, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "V.Armour 2 [%u -> %u]", *p2_varmour_base, *p2_varmour_mod);
                (*(void (*)()) osd_vector)(5, 215, cbuffer);

                snprintf(cbuffer, sizeof(cbuffer), "Stun 2 [%u]", *p2_stun);
                (*(void (*)()) osd_vector)(5, 230, cbuffer);
            }
        }
    }
    else
    {
        play_vector = 0;
        osd_vector = 0;
    }

#ifdef VECTOR_TRACK 
    if (spc() > 0x8c270000 && play_vector != spc())
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "spc() = %x from %x [gm %x] [?? %x]", spc(), stack->pr, *((uint16 *) 0x8ccf022a), *((uint16 *) 0x8ccf0228));

        play_vector = spc();
    }
#endif

    return current_vector;
}

void* customize_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: We only break on the serial (channel A) exception. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */
        return my_customize_handler(stack, current_vector);
    }
    else
        return current_vector;
}
