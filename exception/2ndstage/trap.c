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

#define PHY_FIFO_SIZE   16

struct
{
    struct
    {
        uint8   data;
        dir_e   direction;
    } data[PHY_FIFO_SIZE];

    uint32      start;
    uint32      size;
} phy_fifo;

struct
{
    char        data[64];

    uint32      start;
    uint32      end;    
} net_fifo;

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


static bool phy_fifo_add(uint8 in_data, dir_e dir)
{
    uint32 fifo_index;

    /* STAGE: Do we have space in the physical fifo? */
    if (phy_fifo.size >= PHY_FIFO_SIZE)
    {
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical fifo desynchronized!\n");
        return FALSE;
    }
        
    /* STAGE: Add the byte to the end of the physical fifo. */
    fifo_index = (phy_fifo.start + phy_fifo.size) % PHY_FIFO_SIZE;
    phy_fifo.data[fifo_index].data = in_data;
    phy_fifo.data[fifo_index].direction = dir;
    phy_fifo.size++;

    return TRUE;
}

static bool phy_fifo_del(uint8 check_data)
{
    /* STAGE: Do we even have data in the physical fifo? Also, check if the
        first byte in the ring equals the passed byte. (check_data) */
    if (!phy_fifo.size || (phy_fifo.data[phy_fifo.start].data != check_data))
    {
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical fifo desynchronized!\n");
        return FALSE;
    }

    /* STAGE: Flush the current byte in the SCIF. */
    *SCIF_R_FS &= ~(SCIF_FS_RDF | SCIF_FS_DR);

    /* STAGE: Flush the current byte in the physical fifo. */
    phy_fifo.start = ++phy_fifo.start % PHY_FIFO_SIZE;
    phy_fifo.size--;

    return TRUE;
}

static uint32 phy_size(void)
{
    /* STAGE: Return with number of bytes within physical fifo. */
    return phy_fifo.size;
}

static uint32 phy_sync_fifo(uint8 *temp_buffer)
{
    uint32 work_index, temp_index;

    /* DEBUG: Notification of physical fifo flush. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Saving and flushing physical fifo...\n");

    /* STAGE: Flush all data not marked IN in the physical fifo. */
    temp_index = 0;
    for (work_index = 0; work_index < PHY_FIFO_SIZE; work_index++)
    {
        uint32 fifo_index;

        fifo_index = (phy_fifo.start + work_index) % PHY_FIFO_SIZE;

        if (phy_fifo.data[fifo_index].direction == IN)
        {
            temp_buffer[temp_index] = phy_fifo.data[fifo_index].data;
            temp_index++;
        }
    }
    
    /* STAGE: Reset the physical fifo. */
    phy_fifo.start = phy_fifo.size = 0;

    /* DEBUG: Notification of physical completion. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical fifo clear!\n");

    /* DEBUG: Notification of SCIF flush. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Flushing the SCIF fifo...\n");

    /* STAGE: Flush the SCIF fifo. */ 
    while (*SCIF_R_FS & SCIF_FS_DR)
    {
        uint8 serial_data;

        /* DEBUG: Obtain the serial data? */
        serial_data = *SCIF_R_FRD;

        /* STAGE: Drop a single character from the SCIF fifo. */
        *SCIF_R_FS &= ~(SCIF_FS_RDF | SCIF_FS_DR);

        /* DEBUG: Notify for debugging. Make sure this crap is even working. */
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "...flush SCIF...\n");
    }

    /* DEBUG: Notification of SCIF completion. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "SCIF FIFO clear!\n");

    return temp_index;
}

static void phy_sync(void)
{
    uint8 temp_buffer[PHY_FIFO_SIZE];
    uint32 work_index, temp_index;

    temp_index = 0;
    if (phy_size())
        temp_index = phy_sync_fifo(temp_buffer);

    /* TODO: Transfer data from the net fifo to the physical fifo - marker IN */

    /* DEBUG: Notification of resynchronization. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Resynchronizing the physical and SCIF fifos:\n");

    /* STAGE: Inject the temporary buffer data in both the SCIF and physical fifos. */
    for (work_index = 0; work_index < temp_index; work_index++)
    {
        uint32 timeout_count;

        /* STAGE: Check if we can transmit on the SCIF. */
        if (!(*SCIF_R_FS & SCIF_FS_TDFE))
        {
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "SCIF fifo overflow during resynchronization!\n");
            break;
        }
        
        /* STAGE: Attempt to transmit the byte. */
        *SCIF_R_FTG = temp_buffer[work_index];
        *SCIF_R_FS &= ~(SCIF_FS_TDFE | SCIF_FS_TEND);

        /* STAGE: Wait for the timeout, if necessary. */
        timeout_count = 0;
        while((timeout_count < SCIF_TIMEOUT) && !(*SCIF_R_FS & SCIF_FS_TEND))
            timeout_count++;

        if (timeout_count == SCIF_TIMEOUT)
        {
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "SCIF timeout during injection.\n");
            break;
        }

        /* STAGE: Handle the physical fifo. */
        phy_fifo_add(temp_buffer[work_index], IN);        
    }

    /* DEBUG: Notification of resynchronization completion. */
    biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical and SCIF fifos resynchronized. %u bytes retained.\n", temp_index);
}
 
uint32 trap_inject_data(const uint8 *data, uint32 size)
{
    uint32 data_processed;

#ifdef ACTUALLY_DO_INJECT_DATA
    /* STAGE: Add incoming data to net fifo. */
    data_processed = net_fifo_add(data, size);
#else
    /* DEBUG: Fake the data injection. */
    data_processed = 0;
#endif

    /* STAGE: Resynchronize net and physical fifos. */
    phy_sync();

    return data_processed;
}

void* rxi_handler(register_stack *stack, void *current_vector)
{
    void *return_vector;

    /* STAGE: If something horrible happens, we don't want VOOT freaking out about it. */
    return_vector = my_exception_finish;

    /* STAGE: Synchronize the physical fifo. */
    phy_sync();

    /* STAGE: If there is any remaining data in the physical fifo, pass on the interrupt. */
    if (phy_size())
        return_vector = current_vector;

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
            /* DEBUG: Immediate mode dumping of outgoing serial data. */
            biudp_printf(VOOT_PACKET_TYPE_DATA, "%c", stack->r2);

            /* TODO (future): Add outgoing data to per-frame dump buffer. */

            /* STAGE: Add outgoing data to physical fifo. */
            if (!phy_fifo_add(stack->r2, OUT))
                biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical fifo overflow in transmission!\n");

            break;        

        /* STAGE: Trapped reception. */
        case 0x8c039b58:
            /* DEBUG: Immediate mode dumping of incoming serial data. */
            biudp_printf(VOOT_PACKET_TYPE_DEBUG, "<%c\n", stack->r3);

            /* STAGE: Remove incoming data from physical fifo. */
            if (!phy_fifo_del(stack->r3))
                biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Physical fifo overflow in reception!\n");

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
