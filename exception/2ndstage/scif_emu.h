#ifndef __SCIF_EMU_H__
#define __SCIF_EMU_H__

#include "system.h"

void scif_emu_init(void);
void* serial_handler(register_stack *stack, void *current_vector);

#endif
