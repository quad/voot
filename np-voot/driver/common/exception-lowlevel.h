/*  exception-lowlevel.h

    $Id: exception-lowlevel.h,v 1.4 2002/08/04 05:48:04 quad Exp $

*/

#ifndef __COMMON_EXCEPTION_LOWLEVEL_H__
#define __COMMON_EXCEPTION_LOWLEVEL_H__

#include "vars.h"

/* NOTE: External definitions and buffers. */

extern uint8    general_sub_handler[];
extern uint8    general_sub_handler_base[];
extern uint8    general_sub_handler_end[];

extern uint8    cache_sub_handler[];
extern uint8    cache_sub_handler_base[];
extern uint8    cache_sub_handler_end[];

extern uint8    interrupt_sub_handler[];
extern uint8    interrupt_sub_handler_base[];
extern uint8    interrupt_sub_handler_end[];

extern void     ubc_handler_lowlevel    (void);
extern void     my_exception_finish     (void);

#endif
