/*  callbacks.h

    $Id: callbacks.h,v 1.4 2002/11/24 14:56:45 quad Exp $

DESCRIPTION

    The declarations of all module callback functions.

*/

#ifndef __COMMON_CALLBACKS_H__
#define __COMMON_CALLBACKS_H__

#include "system.h"

/* NOTE: External definitions. */

extern void     module_initialize   (void);
extern void     module_configure    (void);
extern void     module_reconfigure  (void);
extern void     module_bios_vector  (void);
extern void *   module_heartbeat    (register_stack *stack, void *current_vector);

#endif
