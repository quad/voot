/*  heartbeat.c

DESCRIPTION

    Every pageflip we receive a call here. It's a great timer.

*/

#include "vars.h"
#include "system.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "asic.h"
#include "voot.h"
#include "gamedata.h"
#include "customize.h"

#include "heartbeat.h"

void init_heartbeat(void)
{
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch on TA_DONE ASIC interrupts. */
    new_irq.irq = EXP_CODE_INT9;
    new_irq.mask0 = ASIC_MASK0_VSYNC;
    new_irq.handler = ta_handler;

    add_asic_handler(&new_irq);
}

static void* my_heartbeat(register_stack *stack, void *current_vector)
{
    uint16 *anim_mode_a = (uint16 *) 0x8ccf0228;
    uint16 *anim_mode_b = (uint16 *) 0x8ccf022a;

    /* STAGE: Enable the various codes. */
    gamedata_enable_debug();

    /* STAGE: See if we need to load customization information. */
    if ((*anim_mode_a == 0x0 && *anim_mode_b == 0x2) ||     /* Training Mode select. */
        (*anim_mode_a == 0x0 && *anim_mode_b == 0x5) ||     /* Single Player 3d select. */
        (*anim_mode_a == 0x2 && *anim_mode_b == 0x9) ||     /* Single Player quick select. */
        (*anim_mode_a == 0x5 && *anim_mode_b == 0x2))       /* Versus select. */
        maybe_load_customize();

    return current_vector;
}

void* ta_handler(void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    return my_heartbeat(stack, current_vector);
}
