/*  exception-lowlevel.h

    $Id: exception-lowlevel.h,v 1.3 2002/06/23 03:22:52 quad Exp $

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

extern uint8    bios_patch_base[];
extern void *   bios_patch_handler;
extern uint8    bios_patch_end[];

#endif
