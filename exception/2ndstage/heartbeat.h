#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include "system.h"

typedef struct
{
    uint32 count;
    uint32 spc;
} my_pageflip;

extern my_pageflip pageflip_info;

void init_heartbeat(void);
void* heartbeat(register_stack *stack, void *current_vector);

#endif
