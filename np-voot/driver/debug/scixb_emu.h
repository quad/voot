/*  scixb_emu.h

    $Id: scixb_emu.h,v 1.2 2003/03/07 20:26:00 quad Exp $

*/

#ifndef __COMMON_SCIXB_EMU__
#define __COMMON_SCIXB_EMU__

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

#endif
