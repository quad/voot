/*  heartbeat.h

    $Id: heartbeat.h,v 1.2 2002/06/20 10:20:04 quad Exp $

*/

#ifndef __COMMON_HEARTBEAT_H__
#define __COMMON_HEARTBEAT_H__

#include "system.h"

/* NOTE: Module definitions. */

void    heartbeat_init      (void);
void *  pageflip_handler    (register_stack *stack, void *current_vector);
void *  ta_handler          (void *passer, register_stack *stack, void *current_vector);

#endif
