/*  trap.c

    Serial trapping, tapping, and injection logic.
*/

#include "vars.h"
#include "voot.h"
#include "exception-lowlevel.h"
#include "exception.h"
#include "serial.h"
#include "trap.h"

/*
    SCIF Write:
        WRITE on 0xFFE8000C in R2   (PC: 8c0397f4)
        Each write occurs within a single pageflip.

    SCIF Read:
        READ  on 0xFFE80014 in R3   (PC: 8c039b58)
*/

struct
{
    bool data_in_fifo;
    uint8 data;
} scif_fifo_status;

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

uint32 trap_inject_data(uint8 *data, uint32 size)
{
    uint32 data_index, timeout_count;

    /* STAGE: Write the bytes to the FIFO, if possible. */
    data_index = 0;
    while((data_index < size) && (*SCIF_R_FS & SCIF_FS_TDFE))
    {
        *SCIF_R_FTG = data[data_index];

        data_index++;

        *SCIF_R_FS &= ~(SCIF_FS_TDFE | SCIF_FS_TEND);
    }

    timeout_count = 0;
    while((timeout_count < SCIF_TIMEOUT) && !(*SCIF_R_FS & SCIF_FS_TEND))
        timeout_count++;

    if (timeout_count == SCIF_TIMEOUT)
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "SCIF timeout during injection.\n");

    return data_index;
}

void* rxi_handler(register_stack *stack, void *current_vector)
{
    void *return_vector;

    /* STAGE: If the data in the buffer is loopback data, drop it on the floor. */
    if (scif_fifo_status.data_in_fifo && *SCIF_R_FRD == scif_fifo_status.data)
    {
        /* STAGE: Theoretically we should only be handling the DR bit, but
            I'm one paranoid guy. */
        *SCIF_R_FS &= ~(SCIF_FS_RDF | SCIF_FS_DR);

        scif_fifo_status.data_in_fifo = FALSE;

        return_vector = my_exception_finish;
    }
    else
        return_vector = current_vector;

    return return_vector;
}

static void* my_serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: SPC based trap checker. */
    switch (spc())
    {
        /* STAGE: Trapped transmission. */
        case 0x8c0397f4:
            biudp_printf(VOOT_PACKET_TYPE_DATA, "%c", stack->r2);

            /* STAGE: Notify our RXI handler that we should drop the loopbacked data. */
            scif_fifo_status.data_in_fifo = TRUE;
            scif_fifo_status.data = stack->r2;

            break;        

        /* STAGE: Trapped reception. */
        case 0x8c039b58:
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "<%c\n", stack->r3);
            break;
    }

    return current_vector;
}

void* serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: Reconfigure serial port for testing. */
    serial_set_baudrate(57600);
    *SCIF_R_FC |= SCIF_FC_LOOP;

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
