#ifndef __CUSTOMIZE_H__
#define __CUSTOMIZE_H__

#include "system.h"

typedef enum
{
    LOAD,
    RUN
} customize_check_mode;

typedef struct
{
    uint8 vr_type;

    uint8 data[0x40F];
} customize_data;

void customize_init(void);
bool customize_reinit(void);
void* customize_handler(register_stack *stack, void *current_vector);

#endif
