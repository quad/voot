/*  customize.c

DESCRIPTION

    Customization reverse engineering helper code.

TODO

    Nirvana

*/

#include "vars.h"
#include "assert.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "util.h"
#include "voot.h"

#include "customize.h"

static const uint8 custom_func_key[] = { 0xe6, 0x2f, 0x53, 0x6e, 0xd6, 0x2f, 0xef, 0x63, 0xc6, 0x2f, 0x38, 0x23 };

void customize_init(void)
{
    exception_table_entry new;

    customize_reinit();

    /* STAGE: Configure UBC Channel A for breakpoint */
    *UBC_R_BAMRA = UBC_BAMR_NOASID;
    *UBC_R_BBRA = UBC_BBR_READ | UBC_BBR_INSTRUCT;

    ubc_wait();

    /* STAGE: Add exception handler for serial access. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = customize_handler;

    add_exception_handler(&new);
}

void customize_reinit(void)
{
    static uint32 custom_func = 0x0;

    /* STAGE: Check if the current custom_func is valid. */
    if (!memcmp((const char *) custom_func, custom_func_key, sizeof(custom_func_key)))
        return;

    /* STAGE: Locate the customization function beginning. */
    custom_func = (uint32) search_sysmem_at(custom_func_key, sizeof(custom_func_key), GAME_MEM_START, SYS_MEM_END);
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "Found customization function @ %x", custom_func);

    /* STAGE: Configure UBC Channel A for breakpoint */
    *UBC_R_BARA = custom_func;

    ubc_wait();
}

static void* my_customize_handler(register_stack *stack, void *current_vector)
{
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "customize break hit @ %x", spc());
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "(%x, %x, %x, %x)", stack->r4, stack->r5, stack->r6, stack->r7);

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
