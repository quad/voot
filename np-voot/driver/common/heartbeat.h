/*  heartbeat.h

    $Id: heartbeat.h,v 1.1 2002/06/11 20:29:52 quad Exp $

*/

#ifndef __COMMON_HEARTBEAT_H__
#define __COMMON_HEARTBEAT_H__

#include "system.h"
#include "callbacks.h"

void    heartbeat_init      (void);
void *  pageflip_handler    (register_stack *stack, void *current_vector);
void *  ta_handler          (void *passer, register_stack *stack, void *current_vector);

#endif
