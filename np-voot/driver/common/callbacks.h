/*  callbacks.h

    $Id: callbacks.h,v 1.2 2002/06/20 10:20:04 quad Exp $

DESCRIPTION

    The declarations of all module callback functions.

*/

#ifndef __COMMON_CALLBACKS_H__
#define __COMMON_CALLBACKS_H__

/* NOTE: External definitions. */

extern void     module_initialize   (void);
extern void     module_configure    (void);
extern void     module_bios_vector  (void);
extern void *   module_heartbeat    (register_stack *stack, void *current_vector);

#endif
