#ifndef __SCIF_EMU__
#define __SCIF_EMU__

void init_ubc_b_serial(void);
void* serial_handler(register_stack *stack, void *current_vector);

#endif
