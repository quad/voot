/*  exception-lowlevel.h

    $Id: exception-lowlevel.h,v 1.2 2002/06/20 10:20:04 quad Exp $

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

extern void     exception_handler_lowlevel  (void);
extern void     my_exception_finish         (void);
extern void     ubc_wait                    (void);

extern uint8    bios_patch_base[];
extern void *   bios_patch_handler;
extern uint8    bios_patch_end[];

#endif
