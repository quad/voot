#ifndef __TRAP_H__
#define __TRAP_H__

#include "system.h"

typedef enum
{
    NONE,
    IN,
    OUT
} dir_e;

typedef struct
{
    struct
    {
        uint8   data;
        dir_e   direction;
    } data[16];

    uint32      start;
    uint32      end;
} scif_fifo_t;

void init_ubc_b_serial(void);
uint32 trap_inject_data(const uint8 *data, uint32 size);
void* rxi_handler(register_stack *stack, void *current_vector);
void* serial_handler(register_stack *stack, void *current_vector);

#endif
