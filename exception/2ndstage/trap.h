#ifndef __TRAP_H__
#define __TRAP_H__

#include "system.h"

void init_ubc_b_serial(void);
void trap_inject_data(uint8 *data, uint32 size);
void* rxi_handler(register_stack *stack, void *current_vector);
void* serial_handler(register_stack *stack, void *current_vector);

#endif
