#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include "system.h"

void init_heartbeat(void);
void* heartbeat(register_stack *stack, void *current_vector);

#endif
