/*  trap.c

    Serial trapping, tapping, and injection logic.
*/

#include "vars.h"
#include "biudp.h"
#include "exception-lowlevel.h"
#include "exception.h"
#include "serial.h"
#include "trap.h"

/*
    SCIF Write:
        WRITE on 0xFFE8000C in R2   (PC: 8c0397f4)

    SCIF Read:
        READ  on 0xFFE80014 in R3   (PC: 8c039b58)
*/

void init_ubc_b_serial(void)
{
    exception_table_entry new;

    /* STAGE: Configure UBC Channel B for breakpoint on serial port access */
    *UBC_R_BARB = 0xFFE80014;
    *UBC_R_BAMRB = UBC_BAMR_NOASID;
    *UBC_R_BBRB = UBC_BBR_READ | UBC_BBR_OPERAND;

    ubc_wait();

    /* STAGE: Add exception handler for serial access. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = serial_handler;

    add_exception_handler(&new);

    /* STAGE: Add exception handler for RXI interrupts. */
    new.type = EXP_TYPE_INT;
    new.code = EXP_CODE_RXI;
    new.handler = rxi_handler;

    add_exception_handler(&new);
} 

void* rxi_handler(register_stack *stack, void *current_vector)
{
    biudp_write_str("#Received RXI interrupt.\r\n");

    return current_vector;
}

static void* my_serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: !!! implement a SPC based trap checker. */
    switch (spc())
    {
    }

    biudp_write('#');
    biudp_write(stack->r3);
    biudp_write('#');
    biudp_write_hex(spc());

    return current_vector;
}

void* serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: !!! Reconfigure serial port for testing. */
    ubc_serial_set_baudrate(57600);

    /* STAGE: We only break on the serial (channel B) exception. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

        /* STAGE: Pass control to the actual code base. */
        return my_serial_handler(stack, current_vector);
    }
    else
        return current_vector;
}
