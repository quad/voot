/*  scif_emu.h

    $Id: scif_emu.h,v 1.1 2002/06/11 20:36:03 quad Exp $

*/

#ifndef __COMMON_SCIF_EMU_H__
#define __COMMON_SCIF_EMU_H__

#include "system.h"

/* NOTE: Module definitions */

void    scif_emu_init   (void);
void *  serial_handler  (register_stack *stack, void *current_vector);

#endif
