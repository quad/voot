/*  trap.c

    Serial trapping, tapping, and injection logic.

TODO

    In the case of an overflow, the serial and FIFO rings should be flushed and re-syncronized. */

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

scif_fifo_t fifo;

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
 
static uint32 fifo_add(const uint8 *data, uint32 size, dir_e direction)
{
}

static dir_e fifo_get(uint8 *data)
{
}

uint32 trap_inject_data(const uint8 *data, uint32 size)
{
    uint32 serial_index, timeout_count;
    uint32 ring_index;

    /* STAGE: Add incoming data to serial ring. */
    serial_index = 0;
    while((serial_index < size) && (*SCIF_R_FS & SCIF_FS_TDFE))
    {
        *SCIF_R_FTG = data[serial_index];

        serial_index++;

        *SCIF_R_FS &= ~(SCIF_FS_TDFE | SCIF_FS_TEND);
    }

    timeout_count = 0;
    while((timeout_count < SCIF_TIMEOUT) && !(*SCIF_R_FS & SCIF_FS_TEND))
        timeout_count++;

    if (timeout_count == SCIF_TIMEOUT)
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "SCIF timeout during injection.\n");

    /* STAGE: Add injected data to FIFO ring with IN designation. */
    ring_index = fifo_add(data, size, IN);
    if (ring_index < serial_index)
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "FIFO ring overflow! %u characters dropped.\n", serial_index - ring_index);
    else if (serial_index < ring_index)
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Serial ring overflow! %u characters dropped.\n", ring_index - serial_index);

    return data_index;
}

void* rxi_handler(register_stack *stack, void *current_vector)
{
    uint8 fifo_data, serial_data;
    dir_e dir;
    bool pass_data;
    void *return_vector;

    /* STAGE: If something horrible happens, we don't want VOOT freaking out about it. */
    return_vector = my_exception_finish;

    /* STAGE: Drop all OUT data, pass along the first IN. If no data, drop
        the whole interrupt on the floor. */
    pass_data = FALSE;
    while (!pass_data)
    {
        /* STAGE: Obtain the data from each FIFO ring. */
        dir = fifo_get(&fifo_data);
        serial_data = *SCIF_R_FRD;

        /* STAGE: Ring syncronization check. */
        if (fifo_data != serial_data)
        {
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "FIFO and serial ring desyncronization!\n");
            break;
        }

        /* STAGE: If the ring data is not "IN", then drop it on the floor. */
        switch (dir)
        {
            /* STAGE: IN data is what we want to pass along. Change the
                vector so that VOOT receives the interrupt and go! */
            case IN:
                return_vector = current_vector;
                pass_data = TRUE;
                break;

            /* STAGE: Otherwise, flush the data from the SCIF and process
                the next piece of data. */
            case OUT:
            default:
                *SCIF_R_FS &= ~(SCIF_FS_RDF | SCIF_FS_DR);
                break;
        }
    }

    return return_vector;
}

static void* my_serial_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: Reconfigure serial port for testing. */
    serial_set_baudrate(57600);
    *SCIF_R_FC |= SCIF_FC_LOOP;

    /* STAGE: SPC based trap checker. */
    switch (spc())
    {
        /* STAGE: Trapped transmission. */
        case 0x8c0397f4:
            /* DEBUG: Immediate mode dumping of serial data. */
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, ">%c", stack->r2);

            /* TODO: Add outgoing data to per-frame dump buffer. */

            /* STAGE: Add outgoing data to FIFO ring with OUT designation. */
            if(!fifo_add(&(stack->r2), sizeof(stack->r2), IN))
                biudp_printf(VOOT_PACKET_TYPE_DEBUG, "FIFO ring overflow in transmission!\n");

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
