#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "system.h"

typedef enum
{
    LOAD,
    RUN
} customize_check_mode;

extern uint32 do_osd;

void customize_init(void);
bool customize_reinit(void);
void* customize_handler(register_stack *stack, void *current_vector);

#endif
