/*  asic.c

    Handle the SEGA-G2 bus ASIC controller.
*/

#include "vars.h"
#include "system.h"
#include "serial.h"
#include "exception.h"
#include "asic.h"

asic_lookup_table asic_table;

#ifdef DEBUG_ASIC

static void dump_asic_segment(volatile uint32 *mask_base)
{
    if (!mask_base[0] && !mask_base[1])
    {
        ubc_serial_write_str(" !MASK\r\n");
        return;
    }

    /* Segment 0-1 */
    if (mask_base[0] & ASIC_MASK0_TADONE)
        ubc_serial_write_str(" +TADONE");
    if (mask_base[0] & ASIC_MASK0_RASTER_BOTTOM)
        ubc_serial_write_str(" +RASTER_BOTTOM");
    if (mask_base[0] & ASIC_MASK0_RASTER_TOP)
        ubc_serial_write_str(" +RASTER_TOP");
    if (mask_base[0] & ASIC_MASK0_VSYNC)
        ubc_serial_write_str(" +VSYNC");

    /* Segment 0-2 */
    if (mask_base[0] & ASIC_MASK0_OPAQUE_POLY)
        ubc_serial_write_str(" +OPAQUE_POLY");
    if (mask_base[0] & ASIC_MASK0_OPAQUE_MOD)
        ubc_serial_write_str(" +OPAQUE_MOD");
    if (mask_base[0] & ASIC_MASK0_TRANS_POLY)
        ubc_serial_write_str(" +TRANS_POLY");
    if (mask_base[0] & ASIC_MASK0_TRANS_MOD)
        ubc_serial_write_str(" +TRANS_MOD");

    /* Segment 0-3 */
    if (mask_base[0] & ASIC_MASK0_MAPLE_DMA)
        ubc_serial_write_str(" +MAPLE_DMA");
    if (mask_base[0] & ASIC_MASK0_MAPLE_ERROR)
        ubc_serial_write_str(" +MAPLE_ERROR");
    if (mask_base[0] & ASIC_MASK0_GDROM_DMA)
        ubc_serial_write_str(" +GDROM_DMA");
    if (mask_base[0] & ASIC_MASK0_AICA_DMA)
        ubc_serial_write_str(" +AICA_DMA");
    if (mask_base[0] & ASIC_MASK0_EXT1_DMA)
        ubc_serial_write_str(" +EXT1_DMA");
    if (mask_base[0] & ASIC_MASK0_EXT2_DMA)
        ubc_serial_write_str(" +EXT2_DMA");
    if (mask_base[0] & ASIC_MASK0_MYSTERY_DMA)
        ubc_serial_write_str(" +MYSTERY_DMA");

    /* Segment 0-4 */
    if (mask_base[0] & ASIC_MASK0_PUNCHPOLY)
        ubc_serial_write_str(" +PUNCHPOLY");

    /* Segemtn 1-1 */
    if (mask_base[1] & ASIC_MASK1_GDROM)
        ubc_serial_write_str(" +GDROM");
    if (mask_base[1] & ASIC_MASK1_AICA)
        ubc_serial_write_str(" +AICA");
    if (mask_base[1] & ASIC_MASK1_MODEM)
        ubc_serial_write_str(" +MODEM");
    if (mask_base[1] & ASIC_MASK1_PCI)
        ubc_serial_write_str(" +PCI");

    ubc_serial_write_str("\r\n");
}

void dump_asic(void)
{
    ubc_serial_write_str("[UBC] ASIC Interrupt Mask Status:\r\n");

    ubc_serial_write_str("[UBC] IRQ 13 -");
    dump_asic_segment(ASIC_IRQ13_MASK);

    ubc_serial_write_str("[UBC] IRQ 11 -");
    dump_asic_segment(ASIC_IRQ11_MASK);

    ubc_serial_write_str("[UBC] IRQ 9 -");
    dump_asic_segment(ASIC_IRQ9_MASK);

    ubc_serial_write_str("[UBC] Done!\r\n");
}

#endif

uint32 add_asic_handler(asic_lookup_table_entry new_entry)
{
    uint32  index;

    for (index = 0; index < ASIC_TABLE_SIZE; index++)
    {
        if(!(asic_table.table[index].irq))
        {
            volatile uint32 *mask_base;

            switch (new_entry.irq)
            {
                case EXP_CODE_INT9:
                    mask_base = ASIC_IRQ9_MASK;
                    break;

                case EXP_CODE_INT11:
                    mask_base = ASIC_IRQ11_MASK;
                    break;

                case EXP_CODE_INT13:
                    mask_base = ASIC_IRQ13_MASK;
                    break;

                default:    /* Whoa, someone got very screwed up here. Abort! */
                    return 0;
                    break;
            }

            mask_base[0] |= new_entry.mask0;
            mask_base[1] |= new_entry.mask1;

            asic_table.table[index] = new_entry;
            return index + 1;
        }
    }

    return 0;
}

void* handle_asic_exception(register_stack *stack, void *current_vector)
{
    uint32 index;
    uint32 code;
    void *new_vector;

    new_vector = current_vector;
    code = *REG_INTEVT;

    /* STAGE: Handle exception table */
    for (index = 0; index < ASIC_TABLE_SIZE; index++)
    {
        asic_lookup_table_entry passer;

        /* Technically, this can cause matchs on exceptions in cases where
            the SH4 combines them and/or the exceptions have been placed in
            a queue. However, this doesn't both me too much. */

        passer.mask0 = ASIC_IRQ_STATUS[0] & asic_table.table[index].mask0;
        passer.mask1 = ASIC_IRQ_STATUS[1] & asic_table.table[index].mask1;
        passer.irq = code;

        if ((passer.mask0 || passer.mask1) && (asic_table.table[index].irq == passer.irq))
        {
            new_vector = asic_table.table[index].handler(&passer, stack, current_vector);

            /* clear the ASIC interrupt */
            ASIC_IRQ_STATUS[0] = passer.mask0;
            ASIC_IRQ_STATUS[1] = passer.mask1;
        }
    }

    return new_vector;
}

void init_asic_handler(void)
{
    exception_table_entry new_entry;

    ubc_serial_write_str("[UBC] ASIC handler intialization ... ");

    new_entry.type = EXP_TYPE_INT;
    new_entry.handler = handle_asic_exception;

    new_entry.code = EXP_CODE_INT9;
    add_exception_handler(new_entry);
    ubc_serial_write_str("9");

    new_entry.code = EXP_CODE_INT11;
    add_exception_handler(new_entry);
    ubc_serial_write_str(" 11");
    
    new_entry.code = EXP_CODE_INT13;
    add_exception_handler(new_entry);

    ubc_serial_write_str(" 13");

    ubc_serial_write_str(" done!\r\n");
}
