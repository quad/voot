/*  exception.c

    Handle incoming exceptions in readable C-code.
*/

#include "vars.h"
#include "system.h"
#include "exception-lowlevel.h"
#include "util.h"
#include "rtl8139c.h"
#include "heartbeat.h"
#include "biosfont.h"
#include "exception.h"

exception_table exp_table;

/* The VBR Buffer - we better find out how large VO's actually is */
uint8 vbr_buffer[VO_VBR_SIZE];
uint8 *vbr_buffer_katana;

static void init_vbr_table(void)
{
    /* STAGE: !!! Install handlers for the other exception types OR remove
        the alternative exception type handlers. */

    /* STAGE: Add our magic sprinkles of evil to the VOOT VBR. */
    memcpy(VBR_INT(vbr_buffer) - (interrupt_sub_handler_base - interrupt_sub_handler),
            interrupt_sub_handler,
            interrupt_sub_handler_end - interrupt_sub_handler);

    /* STAGE: Relocate the Katana VBR index - bypass our entry logic. */
    vbr_buffer_katana = vbr_buffer + (sizeof(uint16) * 4);

    /* STAGE: Flush cache after modifying application memory */
    flush_cache();

    /* STAGE: Notify ourselves of the change. */
    exp_table.vbr_switched = TRUE;
}

static bool is_vbr_switch_time(void)
{
    uint32 int_installed;

    /* STAGE: Check to see if our interrupt hooks are still installed. */
    int_installed = memcmp(VBR_INT(vbr_buffer) - (interrupt_sub_handler_base - interrupt_sub_handler),
                            interrupt_sub_handler,
                            interrupt_sub_handler_end - interrupt_sub_handler);

    /* Have we had enough exceptions to make it worthwhile? */
    return exp_table.ubc_exception_count >= 5 && int_installed;
}

uint32 add_exception_handler(const exception_table_entry *new_entry)
{
    uint32 index;

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if(!(exp_table.table[index].type))
        {
            memcpy(&exp_table.table[index], new_entry, sizeof(exception_table_entry));
            return index + 1;
        }
    }

    return 0;
}

void* exception_handler(register_stack *stack)
{
    uint32 exception_code, index;
    void *back_vector;
    bool do_vbr_switch;

    /* STAGE: Increase our counters and set the proper back_vectors */
    switch (stack->exception_type)
    {
        case EXP_TYPE_GEN:
            exp_table.general_exception_count++;
            exception_code = *REG_EXPEVT;
            if (exception_code == EXP_CODE_UBC)    /* Never pass on UBC interrupts to the game */
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

    do_vbr_switch = is_vbr_switch_time();

    /* STAGE: Handle the first initialization */
    if (do_vbr_switch && !exp_table.vbr_switched)
    {
        /* ***** PLACE OTHER INITIALIZATION TIME CODE HERE ***** */
        /* STAGE: Locate and assign syMalloc functionality. */
        malloc_init();

        /* STAGE: Initialize the BBA. */
        if (pci_detect())
        {
            if (pci_bb_init())
            {
                if(rtl_init())
                {
                    /* STAGE: Handle ASIC exceptions */
                    init_asic_handler();
                }
            }
        }

        /* STAGE: Grab a UBC timer. */
        init_heartbeat();

        /* STAGE: Initialize the new VBR */
        init_vbr_table();

        /* STAGE: Pre-cache the biosfont address. */
        bfont_init(); 
    }
    /* STAGE: Handle reinitializations differently. */
    else if(do_vbr_switch && exp_table.vbr_switched)
    {
        /* STAGE: Initialize the new VBR */
        init_vbr_table();

        /* DEBUG: Notify about reinitialization. */
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Reinitialized VBR\n");
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
