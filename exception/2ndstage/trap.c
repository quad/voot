/*  trap.c

    Serial trapping, tapping, and injection logic.
*/

#include "vars.h"
#include "biudp.h"
#include "exception-lowlevel.h"
#include "exception.h"
#include "serial.h"
#include "heartbeat.h"
#include "trap.h"

/*
    SCIF Write:
        WRITE on 0xFFE8000C in R2   (PC: 8c0397f4)
        Each write occurs within a single pageflip.

    SCIF Read:
        READ  on 0xFFE80014 in R3   (PC: 8c039b58)
*/

void init_ubc_b_serial(void)
{
    exception_table_entry new;

    /* STAGE: Configure UBC Channel B for breakpoint on serial port access */
    *UBC_R_BARB = 0xFFE80000;
    *UBC_R_BAMRB = UBC_BAMR_NOASID | UBC_BAMR_MASK_10;
    *UBC_R_BBRB = UBC_BBR_RW | UBC_BBR_OPERAND;

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

void trap_inject_data(uint8 *data, uint32 size)
{
    volatile uint16 *scif_sr = REGISTER(uint16) 0xFFE80010;
    volatile uint16 *scif_cr = REGISTER(uint16) 0xFFe80018;
    volatile uint8 *scif_tdr = REGISTER(uint8) 0xFFE8000C;
    uint32 data_index, timeout_count;

    #define SCIF_SR_TDFE    (1<<5)
    #define SCIF_SR_TEND    (1<<6)
    #define SCIF_CR_LOOP    (1)
    #define SCIF_TIMEOUT    10000

    biudp_write_str("i>");

    *scif_cr |= SCIF_CR_LOOP;

    /* STAGE: !!! switch the serial port into loopback mode and write some bytes. */
    data_index = 0;
    while((data_index < size) && (*scif_sr & SCIF_SR_TDFE))
    {
        *scif_tdr = data[data_index];

        biudp_write(data[data_index]);

        data_index++;

        *scif_sr &= ~(SCIF_SR_TDFE | SCIF_SR_TEND);
    }

    biudp_write_str("#\r\n");

    timeout_count = 0;
    while((timeout_count < SCIF_TIMEOUT) && !(*scif_sr & SCIF_SR_TEND))
        timeout_count++;

    if (timeout_count == SCIF_TIMEOUT)
        biudp_write_str("[UBC] SCIF timeout during injection.\r\n");

    *scif_cr &= ~(SCIF_CR_LOOP);
}

void* rxi_handler(register_stack *stack, void *current_vector)
{
    biudp_write_str("[UBC] Received RXI interrupt.\r\n");

    return current_vector;
}

static void* my_serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: SPC based trap checker. */
    switch (spc())
    {
        /* STAGE: Trapped transmission. */
        case 0x8c0397f4:
            biudp_write('>');
            biudp_write(stack->r2);
            biudp_write_str("\r\n");
            break;        

        /* STAGE: Trapped reception. */
        case 0x8c039b58:
            biudp_write('<');
            biudp_write(stack->r3);
            biudp_write_str("\r\n");
            break;
    }

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
