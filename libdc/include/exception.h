#ifndef __LIBDC_EXCEPTION_H__
#define __LIBDC_EXCEPTION_H__

#include "vars.h"
#include "system.h"

#define ASIC_BASE           (0xa05f6900)
#define ASIC_IRQ_STATUS     (REGISTER(uint32) (ASIC_BASE + 0x00))
#define ASIC_IRQ13_MASK     (REGISTER(uint32) (ASIC_BASE + 0x10))
#define ASIC_IRQ11_MASK     (REGISTER(uint32) (ASIC_BASE + 0x20))
#define ASIC_IRQ9_MASK      (REGISTER(uint32) (ASIC_BASE + 0x30))

/* Masks taken from an e-mail from Marcus Comstedt on dcdev */

#define ASIC_MASK0_TADONE           0x04    /* Rendering complete */
#define ASIC_MASK0_RASTER_BOTTOM    0x08    /* Bottom raster event */
#define ASIC_MASK0_RASTER_TOP       0x10    /* Top raster event */
#define ASIC_MASK0_VSYNC            0x20    /* Vsync event */

#define ASIC_MASK0_OPAQUE_POLY      0x080   /* Opaque polygon binning complete */
#define ASIC_MASK0_OPAQUE_MOD       0x100   /* Opaque modifier binning complete */
#define ASIC_MASK0_TRANS_POLY       0x200   /* Transparent polygon binning complete */
#define ASIC_MASK0_TRANS_MOD        0x400   /* Transparent modifier binning complete */

#define ASIC_MASK0_MAPLE_DMA        0x01000     /* Maple DMA complete */
#define ASIC_MASK0_MAPLE_ERROR      0x02000     /* Maple error */
#define ASIC_MASK0_GDROM_DMA        0x04000     /* GD-ROM DMA complete */
#define ASIC_MASK0_AICA_DMA         0x08000     /* AICA DMA complete */
#define ASIC_MASK0_EXT1_DMA         0x10000     /* External DMA 1 complete */
#define ASIC_MASK0_EXT2_DMA         0x20000     /* External DMA 2 complete */
#define ASIC_MASK0_MYSTERY_DMA      0x40000     /* Mystery DMA complete */

#define ASIC_MASK0_PUNCHPOLY        0x200000    /* Punchthrough polygon binning complete */

#define ASIC_MASK1_GDROM            0x01    /* GD-ROM command status */
#define ASIC_MASK1_AICA             0x02    /* AICA */
#define ASIC_MASK1_MODEM            0x04    /* Modem ? */
#define ASIC_MASK1_PCI              0x08    /* Expansion port (PCI Bridge) */

#define EXP_TYPE_GEN        1
#define EXP_TYPE_CACHE      2
#define EXP_TYPE_INT        3

#define EXP_CODE_INT9       0x320
#define EXP_CODE_INT11      0x360
#define EXP_CODE_INT13      0x3A0
#define EXP_CODE_UBC        0x1E0
#define EXP_CODE_BAD        0xFFF

#define EXP_TABLE_SIZE  10

typedef struct
{
    /* SH4 Exception Ring */
    uint32  code;

    /* ASIC Exception Ring */
    uint32  mask0, mask1;

    void    *data;

    bool    (*handler)(const void *, register_stack *);
} exception_table_entry;

typedef struct
{
    /* Exception counters */
    uint32      general_exception_count;
    uint32      cache_exception_count;
    uint32      interrupt_exception_count;
    uint32      ubc_exception_count;
    uint32      odd_exception_count;

    /* Function hooks for various interrupts */
    exception_table_entry   table[EXP_TABLE_SIZE];
} exception_table;

uint32 add_exception_handler(exception_table *exp_table, const exception_table_entry *new_entry);
bool exception_handler(exception_table *exp_table, register_stack *stack);

#endif
