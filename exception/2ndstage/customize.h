#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "system.h"

void customize_init(void);
void customize_reinit(void);
void* customize_handler(register_stack *stack, void *current_vector);

#endif
