/*  anim.c

    $Id: anim.c,v 1.3 2002/07/06 14:18:15 quad Exp $

DESCRIPTION

    A module which provides a function hook chain on the VOOT render-cycle.

TODO

    Handle the case of reinitialization. (we've overwritten the two
    instructions of the matched functions.)

    Do not allow the module to be accessed unless initialized.

*/

#include "vars.h"
#include "util.h"
#include "searchmem.h"
#include "gamedata.h"
#include "exception.h"
#include "ubc.h"

#include "anim.h"

/*
    NOTE: This is one of the few fixed locations common between all versions
    of VOOT.
*/

anim_printf_debug_f __anim_printf_debug = (anim_printf_debug_f) 0x8c0153c4;

static bool         inited;
static uint16       *anim_render_root;
static const uint8  anim_render_key[] = {
                                            0xdd, 0x04, 0x33, 0x34, 0x34, 0x8b,
                                            0x14, 0xe1, 0x17, 0x34
                                        };

static anim_render_chain_f  anim_render_chain;
static exception_handler_f  old_trap_handler;

static void* anim_trap_handler (register_stack *stack, void *current_vector)
{
    if (ubc_trap_number () == TRAP_CODE_ANIM)
    {
        /* STAGE: If this is our trap, ensure the original code is emulated. */

        stack->r4 = *GAMEDATA_ANIM_MODE_B;

        if (anim_render_chain)
            anim_render_chain (*GAMEDATA_ANIM_MODE_A, *GAMEDATA_ANIM_MODE_B);
    }

    /* STAGE: Pass back to a further trap handler, if one exists. */

    if (old_trap_handler)
        return old_trap_handler (stack, current_vector);
    else
        return current_vector;
}

void anim_init (void)
{
    uint16                  anim_trap;
    exception_table_entry   new_exception;

    /* STAGE: Generate our trap code for comparison testing... */

    anim_trap = ubc_generate_trap (TRAP_CODE_ANIM);

    /* STAGE: Locate the animation trappoint, if necessary. */

    if (!anim_render_root || memcmp (anim_render_root, &anim_trap, sizeof (anim_trap)))
        anim_render_root = search_gamemem (anim_render_key, sizeof (anim_render_key));

    /* STAGE: Configure an exception handler for traps. */

    new_exception.type      = EXP_TYPE_GEN;
    new_exception.code      = EXP_CODE_TRAP;
    new_exception.handler   = anim_trap_handler;

    if (!inited)
        inited = exception_add_handler (&new_exception, &old_trap_handler);

    /* STAGE: Add the trappoint, if we're ready for it. */

    if (anim_render_root && inited)
        anim_render_root[0] = anim_trap;
}

bool anim_add_render_chain (anim_render_chain_f new_function, anim_render_chain_f *old_function)
{
    if (anim_render_root)
    {
        /*
            STAGE: Swap around the functions, and give them back the
            original for passthrough.
        */

        *old_function = anim_render_chain;
        anim_render_chain = new_function;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
