/*  exception.c

    Handle incoming exceptions in readable C-code.
*/

#include "vars.h"
#include "system.h"
#include "serial.h"
#include "util.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "rtl8139c.h"
#include "heartbeat.h"

exception_table exp_table;

/* The VBR Buffer - we better find out how large VO's actually is */
uint8 vbr_buffer[VO_VBR_SIZE];
uint8 *vbr_buffer_katana;

static void init_vbr_table(void)
{
    /* STAGE: Be evil to VOOT VBR */
    memcpy(VBR_INT(vbr_buffer) - (interrupt_sub_handler_base - interrupt_sub_handler),
            interrupt_sub_handler,
            interrupt_sub_handler_end - interrupt_sub_handler);

    /* STAGE: Relocate the Katana VBR index */
    vbr_buffer_katana = vbr_buffer + (4 * 2);

    /* STAGE: Flush cache after modifying application memory */
    flush_cache();

    /* STAGE: Change the actual VBR */
    exp_table.vbr_switched = TRUE;
}

static bool is_vbr_switch_time(void)
{
    return exp_table.ubc_exception_count >= 5;
}

uint32 add_exception_handler(exception_table_entry new_entry)
{
    uint32  index;

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if(!(exp_table.table[index].type))
        {
            exp_table.table[index] = new_entry;
            return index + 1;
        }
    }

    return 0;
}

void* exception_handler(register_stack *stack)
{
    uint32 exception_code, index;
    void *back_vector;

    /* STAGE: Increase our counters and set the proper back_vectors */
    switch (stack->exception_type)
    {
        case EXP_TYPE_GEN:
            exp_table.general_exception_count++;
            exception_code = *REG_EXPEVT;
            if (exception_code == 0x1e0)    /* Never pass on UBC interrupts to the game */
            {
                exp_table.ubc_exception_count++;
                back_vector = my_exception_finish;
            }
            else
                back_vector = VBR_GEN(vbr_buffer_katana);
            break; 

        case EXP_TYPE_CACHE:
            exp_table.cache_exception_count++;
            exception_code = *REG_EXPEVT;
            back_vector = VBR_CACHE(vbr_buffer_katana);
            break;

        case EXP_TYPE_INT:
            exp_table.interrupt_exception_count++;
            exception_code = *REG_INTEVT;
            back_vector = VBR_INT(vbr_buffer_katana);
            break;

        default:
            exp_table.odd_exception_count++;
            exception_code = EXP_CODE_BAD;
            back_vector = my_exception_finish;
            break;
    }

    /* STAGE: Handle the first initialization */
    if (!exp_table.vbr_switched && is_vbr_switch_time())
    {
#ifdef DEBUG
        /* STAGE: Initialize the serial port */
        ubc_serial_init(57600);
#endif

        /* ***** PLACE OTHER INITIALIZATION TIME CODE HERE ***** */
        /* STAGE: Initialize the BBA. */
#ifdef DEBUG
        ubc_serial_write_str("[UBC] BBA Init:");
#endif
        if (pci_detect())
        {
#ifdef DEBUG
            ubc_serial_write_str(" found");
#endif
            if (pci_bb_init())
            {
#ifdef DEBUG
                ubc_serial_write_str(" pci_bb");
#endif
                if(rtl_init())
                {
#ifdef DEBUG
                    ubc_serial_write_str(" rtl");
#endif
                    /* STAGE: Handle ASIC exceptions */
                    init_asic_handler();
                }
            }
        }

        /* STAGE: Grab a UBC timer. */
        init_heartbeat();

        /* STAGE: Initialize the new VBR */
        init_vbr_table();
    }

    /* STAGE: Handle exception table */
    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if (exp_table.table[index].code == exception_code &&
            exp_table.table[index].type == stack->exception_type)
        {
            back_vector = exp_table.table[index].handler(stack, back_vector);
        }
    }

    /* STAGE: RETURN */
    return back_vector;
}
