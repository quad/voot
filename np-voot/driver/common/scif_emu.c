/*  scif_emu.c

    $Id: scif_emu.c,v 1.1 2002/06/11 23:47:03 quad Exp $

DESCRIPTION

    An emulation of the SCIF chip.

TODO

    The whole module.

*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"

#include "scif_emu.h"

void scif_emu_init (void)
{
    exception_table_entry   new;

    /* STAGE: Configure UBC Channel B for breakpoint on serial port access. */

    *UBC_R_BARB     = 0xFFE80000;
    *UBC_R_BAMRB    = UBC_BAMR_NOASID | UBC_BAMR_MASK_10;
    *UBC_R_BBRB     = UBC_BBR_RW | UBC_BBR_OPERAND;

    ubc_wait ();

    /* STAGE: Add exception handler for serial access. */

    new.type    = EXP_TYPE_GEN;
    new.code    = EXP_CODE_UBC;
    new.handler = serial_handler;

    add_exception_handler (&new);
}

static void* my_serial_handler (register_stack *stack, void *current_vector)
{
    uint16 *instr_raw;

    /* STAGE: The UBC breaks with the PC + 2 from the actual access. */

    instr_raw = (uint16 *) (spc() - 2);

    return current_vector;
}

void* serial_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: We only break on the serial (channel B) exception. */

    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        /* STAGE: Be sure to clear the proper bit. */

        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

        /* STAGE: Pass control to the actual code base. */

        return my_serial_handler (stack, current_vector);
    }
    else
    {
        return current_vector;
    }
}
