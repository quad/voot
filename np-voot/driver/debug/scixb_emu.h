/*  scixb_emu.h

    $Id: scixb_emu.h,v 1.1 2002/08/04 05:48:05 quad Exp $

*/

#ifndef __COMMON_SCIXB_EMU__
#define __COMMON_SCIXB_EMU__

#define FIFO_INDEX_FROM_ROOT    0xb8

typedef struct
{
    uint8  *pointer;
    uint32  size;
    uint32  end;
    uint32  head;
    uint32  status;
} scixb_fifo_t;

/* STAGE: Module definitions. */

void    scixb_init      (void);
void    scixb_inject    (char data);

#endif
